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
