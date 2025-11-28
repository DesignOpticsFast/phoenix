# Developer Environment Setup — macOS

> ⚠️ **UPDATED (2025-11-25): dev-01 removed. All CI now runs on GitHub Actions.**
>
> This guide is for **local macOS development** only. CI runs automatically on GitHub Actions.

This guide provides steps for setting up a local macOS development environment for Phoenix. It covers installing Qt, configuring environment variables, and running a smoke test build.

> **Note:** For current toolchain versions (Qt, C++ standard, CMake, Protobuf, etc.), see [VERSIONS.md](VERSIONS.md).

---

## 🎯 **Development Policy**

### **Local Development + GitHub Actions CI**

**Workflow:**
1. **Develop locally** - Make code changes on your macOS machine
2. **Test locally** - Build and test with Qt 6.10.1
3. **Commit and push** - Push changes to repository
4. **CI validates** - GitHub Actions runs tests automatically

---

## 📦 **Prerequisites**

- macOS (tested on macOS 13+)
- Homebrew (or Qt installer from qt.io)
- CMake (see [VERSIONS.md](VERSIONS.md) for minimum version)
- Git
- C++17 compatible compiler (Xcode Command Line Tools)

> **Note:** For exact version requirements, see [VERSIONS.md](VERSIONS.md).

---

## 🔧 **Qt Installation**

### **Option 1: Homebrew (Recommended)**

```bash
# Install Qt via Homebrew
brew install qt@6

# Verify installation
brew list qt@6
```

### **Option 2: Qt Installer**

1. Download Qt from [qt.io](https://www.qt.io/download) (see [VERSIONS.md](VERSIONS.md) for current version)
2. Install to the standard location (see [VERSIONS.md](VERSIONS.md) for macOS path)
3. Verify: `ls ~/Qt/<version>/macos/lib/cmake/Qt6`

---

## 🛠️ **Environment Setup**

### **Set CMAKE_PREFIX_PATH**

**Homebrew:**
```bash
export CMAKE_PREFIX_PATH=$(brew --prefix qt@6)
```

**Qt Installer:**
```bash
export CMAKE_PREFIX_PATH=~/Qt/<version>/macos
```

**Add to `~/.zshrc` or `~/.bash_profile`:**
```bash
# Qt for Phoenix development (see VERSIONS.md for current version)
export CMAKE_PREFIX_PATH=$(brew --prefix qt@6)  # or ~/Qt/<version>/macos
```

> **Note:** For the exact Qt path and version, see [VERSIONS.md](VERSIONS.md).

---

## 🚀 **Build and Test**

```bash
# Clone repository
git clone https://github.com/DesignOpticsFast/phoenix.git
cd phoenix

# Initialize submodules
git submodule update --init --recursive

# Configure build
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH"

# Build
cmake --build build -j

# Run
./build/phoenix_app
```

---

## ✅ **Verification**

```bash
# Check Qt version
qmake --version

# Verify CMake can find Qt
cmake --find-package -DNAME=Qt6 -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST

# Test build
cmake -S . -B build -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH" && cmake --build build
```

---

## 🔍 **Troubleshooting**

### CMake cannot find Qt
- Ensure `CMAKE_PREFIX_PATH` points to Qt installation root
- For Homebrew: `CMAKE_PREFIX_PATH=$(brew --prefix qt@6)`
- For Qt Installer: `CMAKE_PREFIX_PATH=~/Qt/6.10.1/macos`

### Build fails
- Check Qt version: `qmake --version` (see [VERSIONS.md](VERSIONS.md) for expected version)
- Verify Qt modules: `ls $CMAKE_PREFIX_PATH/lib/cmake/Qt6/`
- Check CMake output for Qt6_DIR path

---

## 📚 **Additional Resources**

- [Phoenix README](../README.md) - Main project documentation
- [Development Workflow](DEVELOPMENT_WORKFLOW.md) - Development process
- [CI Workflow System](CI_WORKFLOW_SYSTEM.md) - CI documentation

---

## ⚠️ **Note**

**dev-01 has been permanently removed.** All CI now runs on GitHub Actions. This guide is for local development only.
