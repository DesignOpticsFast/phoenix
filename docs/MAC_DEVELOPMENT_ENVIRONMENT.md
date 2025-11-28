# Mac Development Environment Documentation

**Purpose**: Document Mac development environment setup for Phoenix  
**Last Updated**: 2025-01-26 (Sprint 4.5)  
**Status**: ✅ Working

> **Note:** For current toolchain versions (Qt, C++ standard, CMake, Protobuf, etc.), see [VERSIONS.md](VERSIONS.md).

## 🖥️ **System Information**

- **macOS Version**: Tested on macOS 13+ (Apple Silicon and Intel)
- **Architecture**: ARM64 (Apple Silicon) or x86_64 (Intel)
- **Primary Development**: Crucible (macOS)

## 🔧 **Development Tools**

### **Qt Installation**

- **Location**: `$HOME/Qt/<version>/macos` (see [VERSIONS.md](VERSIONS.md) for current version)
- **Installation Method**: Qt Online Installer (recommended) or Homebrew
- **CMake Path**: `$HOME/Qt/<version>/macos/lib/cmake`

> **Note:** For the current Qt version and exact path, see [VERSIONS.md](VERSIONS.md).

**Key Files:**

```bash
# Qt6 Configuration (adjust version as needed)
$HOME/Qt/<version>/macos/lib/cmake/Qt6/Qt6Config.cmake

# Qt6 Binaries
$HOME/Qt/<version>/macos/bin/qmake
$HOME/Qt/<version>/macos/bin/moc
$HOME/Qt/<version>/macos/bin/uic
$HOME/Qt/<version>/macos/bin/rcc

# Qt6 Libraries
$HOME/Qt/<version>/macos/lib/
```

### **CMake**

- **Location**: System-installed (via Xcode Command Line Tools) or Homebrew
- **Version**: See [VERSIONS.md](VERSIONS.md) for minimum version
- **Usage**: `cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/Qt/<version>/macos`

### **Git**

- **Version**: 2.50.1 (Apple Git-155)
- **Location**: System-installed

### **Other Tools**

- **Ninja Build**: Not installed (uses make instead)
- **Xcode Command Line Tools**: Installed
- **Homebrew**: Not installed

## 🚀 **Build Commands**

### **Standard Build**

```bash
# Configure with Qt (see VERSIONS.md for current version and path)
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=$HOME/Qt/<version>/macos

# Build
cmake --build build -j
```

### **Debug Build**

```bash
# Configure for debug (see VERSIONS.md for Qt path)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$HOME/Qt/<version>/macos

# Build
cmake --build build -j
```

### **Clean Build**

```bash
# Remove build directory
rm -rf build

# Reconfigure and build (see VERSIONS.md for Qt path)
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=$HOME/Qt/<version>/macos
cmake --build build -j
```

## 📁 **Directory Structure**

```
$HOME/workspace/phoenix/  # or your preferred location
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
# Set Qt path (see VERSIONS.md for current version)
export CMAKE_PREFIX_PATH=$HOME/Qt/<version>/macos

# Alternative: Set Qt6_DIR
export Qt6_DIR=$HOME/Qt/<version>/macos/lib/cmake/Qt6
```

### **Optional Environment Setup**

```bash
# Add Qt tools to PATH (if needed, see VERSIONS.md for version)
export PATH="$HOME/Qt/<version>/macos/bin:$PATH"

# Verify Qt installation
qmake -v
moc -v
uic -v
```

## 🛠️ **Development Workflow**

### **Daily Development**

```bash
# 1. Pull latest changes
git pull

# 2. Generate Dock icon (if needed)
./scripts/generate_macos_icon.sh

# 3. Build Phoenix (see VERSIONS.md for Qt path)
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=$HOME/Qt/<version>/macos
cmake --build build -j

# 4. Test application
open build/Phoenix.app
```

### **Icon Updates**

```bash
# 1. Update source image
cp your_new_icon.svg resources/macos/Phoenix.svg

# 2. Regenerate Dock icon
./scripts/generate_macos_icon.sh

# 3. Rebuild
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=$HOME/Qt/<version>/macos
cmake --build build -j

# 4. Test new icon
killall -KILL Dock
open build/Phoenix.app
```

## 🔧 **Troubleshooting**

### **Qt Not Found**

```bash
# Check Qt installation (see VERSIONS.md for current version)
ls -la $HOME/Qt/<version>/macos/lib/cmake/Qt6/

# Verify CMake can find Qt6
cmake --find-package -DNAME=Qt6 -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST -DCMAKE_PREFIX_PATH=$HOME/Qt/<version>/macos
```

### **Build Failures**

```bash
# Check CMake configuration (see VERSIONS.md for Qt path)
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=$HOME/Qt/<version>/macos --debug-output

# Check build logs
cmake --build build -j --verbose
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

# Check Qt (see VERSIONS.md for version)
$HOME/Qt/<version>/macos/bin/qmake -v
```

### **Phoenix Build Check**

```bash
# Check if Phoenix builds (see VERSIONS.md for Qt path)
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=$HOME/Qt/<version>/macos
cmake --build build -j

# Check if app runs
open build/Phoenix.app
```

## 🚀 **Quick Reference**

### **Essential Commands**

```bash
# Build Phoenix (see VERSIONS.md for Qt path)
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=$HOME/Qt/<version>/macos && cmake --build build -j

# Generate Dock icon
./scripts/generate_macos_icon.sh

# Test Dock icon
killall -KILL Dock && open build/Phoenix.app

# Clean build
rm -rf build
```

### **Key Paths**

- **Qt**: `$HOME/Qt/<version>/macos` (see [VERSIONS.md](VERSIONS.md) for current version)
- **CMake**: System-installed (see [VERSIONS.md](VERSIONS.md) for minimum version)
- **Git**: System-installed
- **Phoenix**: `$HOME/workspace/phoenix` (or your preferred location)

## 📚 **Related Documentation**

- [HOW_TO_UPDATE_MACOS_DOCK_ICON.md](HOW_TO_UPDATE_MACOS_DOCK_ICON.md) - Icon update guide
- [mac_dock_icon.md](mac_dock_icon.md) - Initial Dock icon setup
- [CONTROL_REV_5.3.md](sprint4/CONTROL_REV_5.3.md) - Sprint 4 requirements

---

**Status**: ✅ **COMPLETE** - Mac development environment documented  
**Next**: Use this guide for Mac development and troubleshooting
