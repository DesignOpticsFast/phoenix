# Development Workflow

## Dev-01 Preflight (Required)

All PRs must pass a local dev-01 build before push.

### Preflight Process

- Run `scripts/dev01-preflight.sh` on dev-01 before committing.
- Pushing from dev-01 automatically runs this via a Git pre-push hook.
- If preflight fails, fix locally before pushing.
- CI builds run on dev-01 and expect a green preflight.

### For Remote Developers

If you work remotely, you can SSH into dev-01 and push after running:

```bash
ssh dev-01
cd /path/to/phoenix
scripts/dev01-preflight.sh
git push
```

### Makefile Targets

- `make preflight` - Run the preflight build
- `make build-dev01` - Same as preflight
- `make clean-dev01` - Clean the dev-01 build directory

### Troubleshooting

#### Preflight Fails

1. Check you're on dev-01: `hostname`
2. Verify Qt environment: `echo $CMAKE_PREFIX_PATH`
3. Run preflight manually: `./scripts/dev01-preflight.sh`
4. Check build logs for specific errors

#### CI Fails After Preflight Passes

1. Ensure you're using the correct runner labels
2. Verify environment variables match between local and CI
3. Check that all changes are committed and pushed

### Environment Requirements

- **Host**: Must be dev-01 (hostname contains "dev-01")
- **Qt**: CMAKE_PREFIX_PATH must point to Qt 6.10.0 installation
- **Build Tools**: Ninja, CMake, GCC
- **License**: QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 (automatically set)

### Build Environment Policy

**dev-01 is the canonical build environment** - macOS builds are intentionally not required because:

- dev-01 provides consistent, fast builds (25 seconds)
- All developers can access dev-01 for validation
- CI runs exclusively on dev-01 for reliability
- No macOS-specific dependencies or workflows needed

### Build Directory

All dev-01 builds use `build/dev-01-relwithdebinfo` to:

- Isolate from other build configurations
- Document the build source (dev-01)
- Allow parallel builds with different configurations

## QML Debugging

We do not use QML debugging in Phoenix. The build system explicitly undefines `QT_QML_DEBUG` to ensure QML debugging hooks are not compiled in.

**Important**: In Qt, defining `QT_QML_DEBUG` (even as `0`) enables QML debugging hooks. We must ensure it is **not defined** at all.

### CMake Configuration

- CMake explicitly undefines `QT_QML_DEBUG` using `-UQT_QML_DEBUG` for all targets
- This ensures QML debugging is disabled even if the IDE injects the macro

### Qt Creator Run Settings

When using Qt Creator, ensure QML debugging is disabled:

1. **Projects → Run** (for your run configuration)
2. **Uncheck** "QML debugging and profiling"
3. **Remove** any additional arguments containing `--qmljsdebugger=...`

This prevents the "QML debugging is enabled. Only use this in a safe environment." banner from appearing at runtime.

## Diagnostics

Phoenix provides several developer diagnostics for troubleshooting icon and font rendering issues:

### Environment Variables

- **`PHX_ICON_NOCACHE=1`** - Bypass the icon cache (useful when debugging icon rendering issues)
- **`PHX_ICON_DEBUG_OUTLINE=1`** - Draw red outline rectangles around icon rendering areas (useful for verifying layout/logical coordinates)

### Logging

- **`QT_LOGGING_RULES="phx.fonts.debug=true;phx.icons.debug=true"`** - Enable debug-level logs for icon and font subsystems
  
  Example:
  ```bash
  QT_LOGGING_RULES="phx.fonts.debug=true;phx.icons.debug=true" ./phoenix_app
  ```

### Compile-Time Diagnostics

- **`PHX_DEV_DIAG`** - Enable startup diagnostics (family/style enumeration, glyph availability probes)
  
  To enable:
  1. Edit `src/app/BuildFlags.h`
  2. Uncomment `#define PHX_DEV_DIAG 1`
  3. Rebuild
  
  This enables:
  - Font Awesome family/style enumeration dumps at startup
  - Glyph availability probes for common icons
  - Detailed startup diagnostics in `IconBootstrap`

**Note**: By default, all diagnostics are disabled for production builds. The icon system runs quietly with minimal logging overhead.

## Palantir IPC Client

Phoenix communicates with compute services via the **Palantir** IPC layer. The PalantirClient provides a non-blocking, event-driven interface with a connection state machine.

### Connection Configuration

- **Socket Name**: Configured via `PALANTIR_SOCKET` environment variable (default: `"palantir_bedrock"`)
  
  Example:
  ```bash
  PALANTIR_SOCKET=/tmp/palantir_custom ./phoenix_app
  ```

### Connection Lifecycle

PalantirClient uses a finite state machine (FSM) with the following states:
- **Idle**: Initial state, no connection
- **Connecting**: Async connection initiated
- **Connected**: Successfully connected and ready
- **ErrorBackoff**: Connection error, waiting before retry
- **PermanentFail**: Maximum retry attempts reached

### Error Handling & Backoff

- **Exponential Backoff**: Reconnection attempts use exponential delays (1s, 2s, 4s, 8s, 16s)
- **Max Attempts**: After 5 failed attempts, connection enters `PermanentFail` state
- **Protocol Errors**: Malformed frames (bad magic, version mismatch, oversize payload) trigger immediate disconnect and backoff

### Protocol

Palantir uses a framed binary protocol:
- **Magic**: `'PLTR'` (0x504C5452)
- **Byte Order**: BigEndian (network byte order)
- **Header**: 12 bytes (magic: u32, version: u16, type: u16, length: u32)
- **Max Message Size**: 8 MiB
- **Version**: Currently v1

### Message Handling

Messages are dispatched via:
1. **Signal**: `messageReceived(quint16 type, QByteArray payload)` - for signal/slot consumers
2. **Handler**: `registerHandler(quint16 type, Handler)` - for direct callback registration

Handlers are called after the signal is emitted, allowing both patterns to coexist.

### Non-Blocking Design

- **No Blocking Calls**: All connection operations are async; the GUI thread never blocks
- **Event-Driven**: Socket signals (`connected`, `errorOccurred`, `readyRead`) drive state transitions
- **No Process Spawning**: Phoenix never spawns Bedrock; the Palantir service is managed separately
