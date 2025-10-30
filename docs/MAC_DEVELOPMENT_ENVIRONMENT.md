# Mac Development Environment Documentation

**Purpose**: Document Mac development environment setup for Phoenix  
**Last Updated**: 2025-01-22  
**Status**: ✅ Working (Tested on macOS 26.0.1, Apple Silicon)

## 🖥️ **System Information**

- **macOS Version**: 26.0.1 (Build 25A362)
- **Architecture**: ARM64 (Apple Silicon)
- **Memory**: 64 GB
- **User**: mark
- **Connection**: Tailscale IP 100.97.54.75

## 🔧 **Development Tools**

### **Qt6 Installation**

- **Location**: `/Users/mark/Qt/6.9.3/macos`
- **Version**: Qt 6.9.3
- **Installation Method**: Qt Installer
- **CMake Path**: `/Users/mark/Qt/6.9.3/macos/lib/cmake/Qt6`

**Key Files:**

```bash
# Qt6 Configuration
/Users/mark/Qt/6.9.3/macos/lib/cmake/Qt6/Qt6Config.cmake

# Qt6 Binaries
/Users/mark/Qt/6.9.3/macos/bin/qmake
/Users/mark/Qt/6.9.3/macos/bin/moc
/Users/mark/Qt/6.9.3/macos/bin/uic
/Users/mark/Qt/6.9.3/macos/bin/rcc

# Qt6 Libraries
/Users/mark/Qt/6.9.3/macos/lib/
```

### **CMake**

- **Location**: System-installed (via Xcode Command Line Tools)
- **Version**: Available via `cmake --version`
- **Usage**: `cmake -S . -B build -DCMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos`

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
# Configure with Qt6
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos

# Build
cmake --build build -j
```

### **Debug Build**

```bash
# Configure for debug
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos

# Build
cmake --build build -j
```

### **Clean Build**

```bash
# Remove build directory
rm -rf build

# Reconfigure and build
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos
cmake --build build -j
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
export CMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos

# Alternative: Set Qt6_DIR
export Qt6_DIR=/Users/mark/Qt/6.9.3/macos/lib/cmake/Qt6
```

### **Optional Environment Setup**

```bash
# Add Qt6 tools to PATH (if needed)
export PATH="/Users/mark/Qt/6.9.3/macos/bin:$PATH"

# Verify Qt6 installation
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

# 3. Build Phoenix
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos
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
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos
cmake --build build -j

# 4. Test new icon
killall -KILL Dock
open build/Phoenix.app
```

## 🔧 **Troubleshooting**

### **Qt6 Not Found**

```bash
# Check Qt6 installation
ls -la /Users/mark/Qt/6.9.3/macos/lib/cmake/Qt6/

# Verify CMake can find Qt6
cmake --find-package -DNAME=Qt6 -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST -DCMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos
```

### **Build Failures**

```bash
# Check CMake configuration
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos --debug-output

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
# Build Phoenix
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos && cmake --build build -j

# Generate Dock icon
./scripts/generate_macos_icon.sh

# Test Dock icon
killall -KILL Dock && open build/Phoenix.app

# Clean build
rm -rf build
```

### **Key Paths**

- **Qt6**: `/Users/mark/Qt/6.9.3/macos`
- **CMake**: System-installed
- **Git**: System-installed
- **Phoenix**: `/Users/mark/Projects/phoenix`

## 📚 **Related Documentation**

- [HOW_TO_UPDATE_MACOS_DOCK_ICON.md](HOW_TO_UPDATE_MACOS_DOCK_ICON.md) - Icon update guide
- [mac_dock_icon.md](mac_dock_icon.md) - Initial Dock icon setup
- [CONTROL_REV_5.3.md](sprint4/CONTROL_REV_5.3.md) - Sprint 4 requirements

---

**Status**: ✅ **COMPLETE** - Mac development environment documented  
**Next**: Use this guide for Mac development and troubleshooting
