# Building Phoenix

## Requirements

- Qt 6.10.0 or later
- CMake 3.20 or later
- C++17-capable compiler (GCC 11+, Clang 14+, MSVC 2019+)
- Linux (dev-01) or macOS

## Quick Start

### Linux (dev-01)

```bash
git clone <repo-url>
cd phoenix
git submodule update --init --recursive
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64 \
  -DBUILD_TESTING=ON \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### macOS

**Prerequisites:**
- Qt 6.10.0 installed (typically at `~/Qt/6.10.0/macos`)
- Xcode Command Line Tools installed
- CMake 3.20+ (via Xcode or Homebrew)

**App-only build (recommended for first-time Mac builds):**

```bash
git clone <repo-url>
cd phoenix
git submodule update --init --recursive

# Generate macOS app icon (required for app bundle)
./scripts/generate_macos_icon.sh

# Configure and build
mkdir -p build && cd build
cmake -S .. -B . \
  -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos \
  -DBUILD_TESTING=OFF \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --target phoenix_app --parallel

# Run the app
open Phoenix.app
```

**Full build (with tests):**

```bash
cd phoenix
mkdir -p build && cd build
cmake -S .. -B . \
  -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos \
  -DBUILD_TESTING=ON \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --parallel
```

**Note:** `PHX_WITH_LIBSODIUM=ON` (default) is required for licensing features. To disable libsodium (dev-only, disables licensing):

```bash
cmake -S .. -B . \
  -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos \
  -DPHX_WITH_LIBSODIUM=OFF \
  -DBUILD_TESTING=OFF
```

## Build Targets

- `phoenix_app` - Main application executable (macOS: `Phoenix.app` bundle)
- `phoenix_analysis` - Analysis window library
- `analysis_sanity_tests` - Analysis window tests
- `graphs_perf_sanity` - Qt Graphs performance test

## Platform-Specific Notes

### Linux (dev-01)
- Tests can be built and run with `BUILD_TESTING=ON`
- Uses X11/XWayland by default for reliable docking

### macOS
- **Icon required:** `Phoenix.icns` must exist before building (generate via `scripts/generate_macos_icon.sh`)
- **App bundle:** Builds as `Phoenix.app` bundle in `build/` directory
- **libsodium:** Vendored libsodium builds automatically (required for licensing features)
- **Tests:** Can be disabled with `BUILD_TESTING=OFF` for faster app-only builds

## Common Issues

### Qt6 Not Found (macOS)
```
CMake Error: Could not find a package configuration file provided by "Qt6"
```
**Solution:** Set `CMAKE_PREFIX_PATH` to your Qt installation:
```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos
```

### Missing Phoenix.icns (macOS)
```
FATAL_ERROR: Missing Phoenix.icns — required for macOS app bundle.
```
**Solution:** Generate the icon:
```bash
./scripts/generate_macos_icon.sh
```

### libsodium Build Failures
If libsodium fails to build on macOS, check:
- Xcode Command Line Tools installed: `xcode-select --install`
- Submodules initialized: `git submodule update --init --recursive`
- For dev-only builds, can disable: `-DPHX_WITH_LIBSODIUM=OFF` (disables licensing features)
