# Mac Development Environment Documentation

**Purpose**: Document Mac development environment setup for Phoenix  
**Last Updated**: 2025-11-19  
**Status**: ✅ Working (Tested on macOS, Apple Silicon)

## 🖥️ **System Requirements**

- **macOS**: 12.0 (Monterey) or later
- **Architecture**: ARM64 (Apple Silicon) or Intel x86_64
- **Xcode Command Line Tools**: Required (install via `xcode-select --install`)
- **CMake**: 3.20 or later

## 🔧 **Development Tools**

### **Qt6 Installation**

- **Location**: `~/Qt/6.10.0/macos` (or custom path)
- **Version**: Qt 6.10.0 (required)
- **Installation Method**: Qt Installer or Qt Online Installer
- **CMake Path**: `~/Qt/6.10.0/macos/lib/cmake/Qt6`

**Key Files:**

```bash
# Qt6 Configuration
~/Qt/6.10.0/macos/lib/cmake/Qt6/Qt6Config.cmake

# Qt6 Binaries
~/Qt/6.10.0/macos/bin/qmake
~/Qt/6.10.0/macos/bin/moc
~/Qt/6.10.0/macos/bin/uic
~/Qt/6.10.0/macos/bin/rcc

# Qt6 Libraries
~/Qt/6.10.0/macos/lib/
```

**Verifying Qt Installation:**

```bash
# Check Qt version
~/Qt/6.10.0/macos/bin/qmake -v

# Verify CMake can find Qt6
cmake --find-package -DNAME=Qt6 -DCOMPILER_ID=AppleClang -DLANGUAGE=CXX -DMODE=EXIST -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos
```

### **CMake**

- **Location**: System-installed (via Xcode Command Line Tools) or Homebrew
- **Version**: Check with `cmake --version` (must be 3.20+)
- **Usage**: `cmake -S . -B build -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos`

### **Git**

- **Version**: 2.50.1 (Apple Git-155)
- **Location**: System-installed

### **Other Tools**

- **Ninja Build**: Not installed (uses make instead)
- **Xcode Command Line Tools**: Installed
- **Homebrew**: Not installed

## 🚀 **Build Commands**

### **First-Time Setup**

```bash
# Clone repository
git clone <repo-url>
cd phoenix

# Initialize submodules (required for libsodium)
git submodule update --init --recursive

# Generate macOS app icon (required before building)
./scripts/generate_macos_icon.sh
```

### **App-Only Build (Recommended for First Build)**

```bash
# Configure and build phoenix_app only
mkdir -p build && cd build
cmake -S .. -B . \
  -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos \
  -DBUILD_TESTING=OFF \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --target phoenix_app --parallel

# Run the app
open Phoenix.app
```

### **Full Build (With Tests)**

```bash
# Configure with all components
mkdir -p build && cd build
cmake -S .. -B . \
  -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos \
  -DBUILD_TESTING=ON \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --parallel

# Run tests
ctest --test-dir . --output-on-failure
```

### **Debug Build**

```bash
# Configure for debug
mkdir -p build && cd build
cmake -S .. -B . \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos \
  -DBUILD_TESTING=OFF
cmake --build . --target phoenix_app --parallel
```

### **Clean Build**

```bash
# Remove build directory
rm -rf build

# Reconfigure and build
mkdir -p build && cd build
cmake -S .. -B . \
  -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos \
  -DBUILD_TESTING=OFF \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --target phoenix_app --parallel
```

## 📁 **Directory Structure**

```
/Users/mark/Projects/phoenix/
├── build/                          # Build directory
│   └── Phoenix.app/                # macOS app bundle
│       ├── Contents/
│       │   ├── Info.plist         # Bundle configuration
│       │   └── Resources/
│       │       └── Phoenix.icns    # Dock icon
│       └── MacOS/
│           └── Phoenix             # Executable
├── resources/macos/
│   ├── Phoenix.svg                 # Source icon
│   └── Phoenix.icns               # Generated Dock icon
├── scripts/
│   └── generate_macos_icon.sh      # Icon generation script
└── docs/
    ├── HOW_TO_UPDATE_MACOS_DOCK_ICON.md
    └── MAC_DEVELOPMENT_ENVIRONMENT.md
```

## 🔍 **Environment Variables**

### **Required for Phoenix Build**

```bash
# Set Qt6 path (if not using -DCMAKE_PREFIX_PATH)
export CMAKE_PREFIX_PATH=~/Qt/6.10.0/macos

# Alternative: Set Qt6_DIR
export Qt6_DIR=~/Qt/6.10.0/macos/lib/cmake/Qt6
```

### **Optional Environment Setup**

```bash
# Add Qt6 tools to PATH (if needed)
export PATH="$HOME/Qt/6.10.0/macos/bin:$PATH"

# Verify Qt6 installation
qmake -v
moc -v
uic -v
```

**Note:** It's recommended to use `-DCMAKE_PREFIX_PATH` in CMake commands rather than environment variables, as it's more explicit and reproducible.

## 🛠️ **Development Workflow**

### **Daily Development**

```bash
# 1. Pull latest changes
git pull
git submodule update --init --recursive  # Update submodules if needed

# 2. Generate Dock icon (if missing or updated)
./scripts/generate_macos_icon.sh

# 3. Build Phoenix (app-only, fast)
cd build  # or mkdir -p build && cd build
cmake -S .. -B . \
  -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos \
  -DBUILD_TESTING=OFF \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --target phoenix_app --parallel

# 4. Test application
open Phoenix.app
```

### **libsodium Dependency**

Phoenix uses vendored libsodium for licensing features. The libsodium sources are in `third_party/libsodium/` and build automatically with Phoenix.

- **Default**: `PHX_WITH_LIBSODIUM=ON` (required for licensing features)
- **Dev-only**: Can disable with `-DPHX_WITH_LIBSODIUM=OFF` (disables licensing features)

**Note:** For Capo testing and production use, keep libsodium enabled. The vendored libsodium builds cleanly on macOS with the same C sources as Linux.

### **Icon Updates**

```bash
# 1. Update source image
cp your_new_icon.svg resources/macos/Phoenix.svg

# 2. Regenerate Dock icon
./scripts/generate_macos_icon.sh

# 3. Rebuild
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos
cmake --build build -j

# 4. Test new icon
killall -KILL Dock
open build/Phoenix.app
```

## 🔧 **Troubleshooting**

### **Qt6 Not Found**

**Error:**
```
CMake Error: Could not find a package configuration file provided by "Qt6"
```

**Solution:**
```bash
# Check Qt6 installation
ls -la ~/Qt/6.10.0/macos/lib/cmake/Qt6/

# Verify CMake can find Qt6
cmake --find-package -DNAME=Qt6 -DCOMPILER_ID=AppleClang -DLANGUAGE=CXX -DMODE=EXIST -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos

# If Qt6 is not found, ensure CMAKE_PREFIX_PATH is set correctly
cmake -S . -B build -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos
```

### **Missing Phoenix.icns**

**Error:**
```
FATAL_ERROR: Missing Phoenix.icns — required for macOS app bundle.
```

**Solution:**
```bash
# Generate the icon
./scripts/generate_macos_icon.sh

# Verify icon exists
ls -l resources/macos/Phoenix.icns
```

### **libsodium Build Failures**

**Error:**
```
No libsodium source files found!
```

**Solution:**
```bash
# Ensure submodules are initialized
git submodule update --init --recursive

# Verify libsodium sources exist
ls -la third_party/libsodium/src/libsodium/

# If libsodium still fails, can disable for dev-only builds (disables licensing)
cmake -S . -B build -DPHX_WITH_LIBSODIUM=OFF -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos
```

**Note:** For production builds, libsodium must be enabled. The vendored libsodium builds cleanly on macOS.

### **Build Failures**

```bash
# Check CMake configuration with debug output
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos \
  --debug-output

# Check build logs with verbose output
cmake --build build --parallel --verbose
```

### **Icon Issues**

```bash
# Check icon file
ls -l resources/macos/Phoenix.icns

# Verify bundle contents
ls -l build/Phoenix.app/Contents/Resources/

# Check Info.plist
plutil -p build/Phoenix.app/Contents/Info.plist | grep -i CFBundleIconFile
```

## 📋 **Verification Commands**

### **System Check**

```bash
# Check macOS version
sw_vers

# Check architecture
uname -m

# Check memory
system_profiler SPHardwareDataType | grep "Memory:"
```

### **Development Tools Check**

```bash
# Check CMake
cmake --version

# Check Git
git --version

# Check Qt6
/Users/mark/Qt/6.9.3/macos/bin/qmake -v
```

### **Phoenix Build Check**

```bash
# Check if Phoenix builds
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos
cmake --build build -j

# Check if app runs
open build/Phoenix.app
```

## 🚀 **Quick Reference**

### **Essential Commands**

```bash
# First-time setup
git submodule update --init --recursive
./scripts/generate_macos_icon.sh

# Build Phoenix (app-only)
mkdir -p build && cd build
cmake -S .. -B . \
  -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/macos \
  -DBUILD_TESTING=OFF \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --target phoenix_app --parallel

# Run app
open Phoenix.app

# Clean build
rm -rf build
```

### **Key Paths**

- **Qt6**: `~/Qt/6.10.0/macos` (or custom path)
- **CMake**: System-installed (via Xcode) or Homebrew
- **Git**: System-installed
- **Phoenix Build**: `build/Phoenix.app`
- **Icon Source**: `resources/macos/Phoenix.svg`
- **Icon Output**: `resources/macos/Phoenix.icns`

## 📚 **Related Documentation**

- [HOW_TO_UPDATE_MACOS_DOCK_ICON.md](HOW_TO_UPDATE_MACOS_DOCK_ICON.md) - Icon update guide
- [mac_dock_icon.md](mac_dock_icon.md) - Initial Dock icon setup
- [CONTROL_REV_5.3.md](sprint4/CONTROL_REV_5.3.md) - Sprint 4 requirements

---

**Status**: ✅ **COMPLETE** - Mac development environment documented  
**Next**: Use this guide for Mac development and troubleshooting
