# Phoenix Toolchain Versions

**Last Updated:** 2025-01-26 (Sprint 4.5)  
**Purpose:** Single source of truth for Phoenix toolchain versions

---

## Toolchain Overview

### C++ Standard
- **C++17** (required)
- Set via `CMAKE_CXX_STANDARD 17` in `CMakeLists.txt`

### Qt
- **Version:** 6.10.1 (baseline)
- **Required:** Yes (Phoenix is a Qt application)
- **Components:** Widgets, Concurrent, Core, Graphs, GraphsWidgets, QuickWidgets, LinguistTools, PrintSupport

### CMake
- **Minimum version:** 3.20
- Required for Qt6 support and modern CMake features

### Build System
- **Ninja** (recommended, used in CI)
- **Make** (also supported)

---

## Library Versions

### Protobuf
- **Version:** 6.33+ (required for Palantir contracts)
- Used for Palantir IPC protocol serialization (when `PHX_WITH_TRANSPORT_DEPS=ON`)
- Protobuf 6.33+ requires Abseil libraries (automatically linked)

### libsodium
- **Status:** Optional (default ON)
- **Usage:** Cryptographic operations
- Controlled via `PHX_WITH_LIBSODIUM` CMake option

---

## Platform Notes

### macOS (Crucible - Primary Development)

**Qt Installation:**
- **Path:** `$HOME/Qt/6.10.1/macos`
- **Qt6Config.cmake:** `$HOME/Qt/6.10.1/macos/lib/cmake/Qt6/Qt6Config.cmake`
- **Installation method:** Qt Online Installer (not Homebrew)

**CMake Configuration:**
```bash
cmake -S . -B build -G "Ninja" \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DPHX_WITH_TRANSPORT_DEPS=ON \
  -DCMAKE_PREFIX_PATH="$HOME/Qt/6.10.1/macos/lib/cmake"
```

**Compiler:**
- **Apple Clang 17.0.0** (or compatible)
- Target: arm64-apple-darwin

**Libraries:**
- Protobuf: Homebrew (`protobuf` package, version 6.33+)
- libsodium: Homebrew (`libsodium` package, optional)

**Notes:**
- Qt tools (`qmake`, `qtpaths`) are not in system PATH by default
- Access Qt tools via full path: `~/Qt/6.10.1/macos/bin/qmake`
- CMake finds Qt via `CMAKE_PREFIX_PATH` (no PATH modification needed)

### Linux (Fulcrum / CI)

**Qt:**
- **Version:** 6.10.1 (installed via `jurplel/install-qt-action@v3` in CI)
- **CMAKE_PREFIX_PATH:** Set automatically by CI action

**Compiler:**
- **GCC** (version varies by CI image, typically GCC 11+)
- **Clang** (alternative, if available)

**Libraries:**
- Protobuf: System package or Homebrew (version 6.33+)
- libsodium: System package (optional)

**CI Environment:**
- **Ubuntu:** `ubuntu-latest` (GitHub Actions)
- **macOS:** `macos-latest` (GitHub Actions)

### Windows

**Status:** Supported (CMakeLists.txt includes Windows-specific configuration)  
**Qt:** 6.10.1 (via Qt Online Installer or vcpkg)  
**Compiler:** MSVC (version TBD - requires verification during Windows setup)  
**Note:** Windows toolchain details to be verified and documented in future sprint

---

## CI Toolchain

### GitHub Actions (Linux - ubuntu-latest)

**Build Tools:**
- CMake (system package)
- Ninja (system package)
- GCC (system default)

**Qt:**
- Qt 6.10.1 (installed via `jurplel/install-qt-action@v3`)
- Modules: `qtbase qttools qttranslations qtgraphs qtgraphswidgets`
- CMAKE_PREFIX_PATH set automatically by action

**Libraries:**
- Protobuf (system package or Homebrew, version 6.33+)
- libsodium (optional, if enabled)

**Coverage:**
- `lcov` (coverage reporting, Linux only)

### GitHub Actions (macOS - macos-latest)

**Build Tools:**
- CMake (Homebrew)
- Ninja (Homebrew)
- Apple Clang (system default)

**Qt:**
- Qt 6.10.1 (installed via `jurplel/install-qt-action@v3`)
- Modules: `qtbase qttools qttranslations qtgraphs qtgraphswidgets`
- CMAKE_PREFIX_PATH set automatically by action

**Libraries:**
- Protobuf (Homebrew, version 6.33+)
- libsodium (Homebrew, optional)

**Coverage:**
- Coverage reporting not enabled on macOS (Linux only)

---

## Version Constraints

### Minimum Versions
- **CMake:** 3.20 (required for Qt6 support)
- **Qt:** 6.10.1 (required, baseline)
- **Protobuf:** 6.33+ (required for Abseil compatibility, when transport deps enabled)
- **C++ Standard:** C++17 (required)

### Recommended Versions
- **CMake:** 3.20 or later
- **Qt:** 6.10.1 (baseline, verified on Crucible)
- **Protobuf:** 6.33+ (latest stable)
- **Compiler:** Apple Clang 17.0.0+ (macOS), GCC 11+ (Linux)

---

## CMake Options

### Transport Dependencies
- **Option:** `PHX_WITH_TRANSPORT_DEPS`
- **Default:** OFF
- **When enabled:** Requires Protobuf 6.33+ and Palantir contracts submodule
- **Usage:** Enables Palantir IPC client functionality

### Palantir Contracts
- **Option:** `PHX_WITH_PALANTIR_CONTRACTS`
- **Default:** ON
- **Usage:** Enables Palantir contract stub generation

### libsodium
- **Option:** `PHX_WITH_LIBSODIUM`
- **Default:** ON
- **Usage:** Cryptographic operations

### Developer Tools
- **Option:** `PHX_DEV_TOOLS`
- **Default:** OFF
- **Usage:** Build developer tools (e.g., Icon Gallery)

---

## Future Planning

### C++23 Upgrade (Bedrock)
- **Status:** Planned for future sprint (Bedrock only)
- **Documentation:** See `bedrock/docs/C++23_UPGRADE_PLAN.md` for details
- **Phoenix Impact:** Phoenix will remain C++17 for compatibility; Bedrock C++23 upgrade does not affect Phoenix

### Qt Version Updates
- **Current baseline:** Qt 6.10.1 (verified on Crucible, Sprint 4.5)
- **Future updates:** Will be documented in this file when Qt version changes
- **Compatibility:** Phoenix requires Qt 6.10+ (per `find_package(Qt6 6.10 REQUIRED)`)

---

## Notes

- **Qt Path:** On macOS, Qt is typically installed at `~/Qt/6.10.1/macos` via Qt Online Installer
- **CMAKE_PREFIX_PATH:** Must point to Qt's CMake directory: `$HOME/Qt/6.10.1/macos/lib/cmake`
- **CI Consistency:** CI uses Qt 6.10.1 on both Linux and macOS for consistency
- **Version Updates:** This document should be updated when toolchain versions change
- **Palantir Contracts:** Contracts are managed via git submodule (`contracts/`)

---

## Verification

**Qt 6.10.1 Verification (Crucible):**
- ✅ Qt 6.10.1 installed at `~/Qt/6.10.1/macos`
- ✅ CMake successfully finds Qt 6.10.1 via `CMAKE_PREFIX_PATH`
- ✅ All required Qt6 components available
- ✅ No Qt-related errors in build output
- **Verification date:** 2025-01-26 (Chunk 4.1)

---

**See Also:**
- `bedrock/docs/VERSIONS.md` - Bedrock toolchain versions
- `bedrock/docs/C++23_UPGRADE_PLAN.md` - Future C++23 upgrade planning (Bedrock)

