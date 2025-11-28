# Developer Diagnostics & Troubleshooting

> **Note:** For the main development workflow, see [DEVELOPMENT_WORKFLOW.md](DEVELOPMENT_WORKFLOW.md).  
> This document focuses on diagnostics, troubleshooting, and protocol details.

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
- **Protocol Errors**: Malformed envelopes (invalid version, unknown type, oversize payload) trigger immediate disconnect and backoff

### Protocol

Phoenix uses the **envelope-based Palantir protocol** over `QLocalSocket`. The protocol uses `MessageEnvelope` protobuf messages wrapped in a length-prefixed wire format.

**Wire Format:**
```
[4-byte length][serialized MessageEnvelope]
```

**MessageEnvelope Structure:**
- **version**: Protocol version (currently 1)
- **type**: Message type enum (e.g., `CAPABILITIES_REQUEST`, `XY_SINE_REQUEST`)
- **payload**: Serialized inner protobuf message (e.g., `CapabilitiesRequest`, `XYSineRequest`)
- **metadata**: Optional string→string map for tracing/flags

**Max Message Size**: 10 MB (enforced on both client and server)

> **Note:** For detailed protocol documentation, see [ADR-0002-Envelope-Based-Palantir-Framing.md](adr/ADR-0002-Envelope-Based-Palantir-Framing.md) and [IPC_ENVELOPE_PROTOCOL.md](ipc/IPC_ENVELOPE_PROTOCOL.md).

### Message Handling

Messages are dispatched via the envelope protocol:
1. **Envelope Parsing**: `MessageEnvelope` is parsed from the wire format
2. **Type Dispatch**: Message type determines which handler processes the payload
3. **Payload Extraction**: Inner protobuf message is extracted and deserialized

> **Note:** For detailed protocol documentation, see [ADR-0002-Envelope-Based-Palantir-Framing.md](adr/ADR-0002-Envelope-Based-Palantir-Framing.md) and [IPC_ENVELOPE_PROTOCOL.md](ipc/IPC_ENVELOPE_PROTOCOL.md).

### Non-Blocking Design

- **No Blocking Calls**: All connection operations are async; the GUI thread never blocks
- **Event-Driven**: Socket signals (`connected`, `errorOccurred`, `readyRead`) drive state transitions
- **No Process Spawning**: Phoenix never spawns Bedrock; the Palantir service is managed separately
