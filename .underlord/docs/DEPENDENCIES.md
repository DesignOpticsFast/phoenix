# Project Dependencies

**Last Updated:** 2025-10-16

---

## Build System

### CMake
- **Version Required:** 3.22+
- **Purpose:** Build configuration
- **Installation:**
  ```bash
  # Amazon Linux 2023
  sudo dnf install cmake
  
  # macOS
  brew install cmake
  ```

### Ninja (Optional)
- **Version:** 1.10+
- **Purpose:** Fast parallel builds
- **Installation:**
  ```bash
  sudo dnf install ninja-build  # Amazon Linux
  brew install ninja            # macOS
  ```

---

## Compilers

### GCC
- **Version Required:** 11.0+
- **Purpose:** C++17 compilation on Linux
- **Installation:**
  ```bash
  sudo dnf install gcc gcc-c++
  ```

### Clang (macOS)
- **Version Required:** 14.0+
- **Purpose:** C++17 compilation on macOS
- **Included with:** Xcode Command Line Tools

---

## Core Libraries

### OpenCASCADE (OCCT)
- **Version:** 7.6+
- **Purpose:** Geometric modeling kernel
- **License:** LGPL 2.1
- **Installation:**
  ```bash
  sudo dnf install occt occt-devel  # Amazon Linux
  brew install opencascade          # macOS
  ```

---

## Testing Frameworks

### GoogleTest
- **Version:** 1.10+
- **Purpose:** Unit testing
- **Installation:**
  ```bash
  sudo dnf install gtest gtest-devel  # Amazon Linux
  brew install googletest             # macOS
  ```

---

## Phoenix-Specific

### Qt6
- **Version Required:** 6.9+
- **Modules:** Core, Widgets, Gui
- **Installation:**
  ```bash
  brew install qt@6  # macOS
  ```
- **Environment:**
  ```bash
  export Qt6_DIR=/opt/homebrew/opt/qt@6/lib/cmake/Qt6
  ```

### Bedrock (Sibling Dependency)
- **Location:** Must be in `../bedrock` relative to Phoenix
- **Build Order:** Bedrock must be built before Phoenix

---

## Development Tools

### ccache (Compiler Cache)
- **Version:** 4.9.1+
- **Purpose:** Dramatically speeds up rebuilds by caching compilation results
- **Installation:**
  ```bash
  # Amazon Linux 2023
  sudo dnf install -y ccache
  
  # macOS
  brew install ccache
  ```
- **Configuration:**
  ```bash
  # Environment variables (add to ~/.bashrc)
  export CCACHE_DIR=/var/cache/ccache
  export CCACHE_BASEDIR=~/workspace
  export CCACHE_COMPRESS=1
  export CCACHE_SLOPPINESS=time_macros
  
  # Verify setup
  ccache -s
  ```
- **Usage:** Automatically enabled via CMake launcher variables
- **Performance:** 100% cache hit rate on rebuilds (~20GB cache, verified on AL2023 dev-01)

### GitHub CLI (gh)
- **Version:** 2.0+
- **Purpose:** PR management, CI monitoring
- **Installation:**
  ```bash
  sudo dnf install gh  # Amazon Linux
  brew install gh      # macOS
  ```
- **Authentication:**
  ```bash
  gh auth login
  gh auth refresh -h github.com -s repo
  ```

### Git
- **Version:** 2.0+
- **Purpose:** Version control
- **Configuration:**
  ```bash
  git config --global user.name "Your Name"
  git config --global user.email "your.email@example.com"
  git config --global credential.useHttpPath true
  ```

---

## Version Compatibility Matrix

| Tool | Bedrock Min | Phoenix Min | Notes |
|------|-------------|-------------|-------|
| CMake | 3.22 | 3.22 | Same version for consistency |
| GCC | 11.0 | 11.0 | C++17 support |
| Clang | 14.0 | 14.0 | macOS default |
| OCCT | 7.6 | 7.6 | Via Bedrock |
| Qt6 | N/A | 6.9 | Phoenix only |
| GoogleTest | 1.10 | 1.10 | Testing framework |
| ccache | 4.9.1 | 4.9.1 | Compiler cache (20G), verified on AL2023 dev-01 |

---

## Quick Setup Scripts

### Amazon Linux 2023 (Bedrock)
```bash
sudo dnf update -y
sudo dnf install -y cmake ninja-build gcc gcc-c++ ccache \
  occt occt-devel gtest gtest-devel git gh
```

### macOS (Phoenix)
```bash
brew install cmake ninja opencascade tbb ccache qt@6 googletest gh
echo 'export Qt6_DIR=/opt/homebrew/opt/qt@6/lib/cmake/Qt6' >> ~/.zshrc
```

---

## Troubleshooting

### CMake can't find OCCT
```bash
cmake -DOCCT_DIR=/path/to/occt ...
```

### CMake can't find Qt6
```bash
export Qt6_DIR=/opt/homebrew/opt/qt@6/lib/cmake/Qt6
```

### GTest not found
```bash
sudo dnf install gtest-devel  # Amazon Linux
brew install googletest       # macOS
```

---

**Note:** Always run `.underlord/preflight.sh` after installing or updating dependencies.
