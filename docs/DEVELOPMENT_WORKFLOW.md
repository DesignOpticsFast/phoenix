# Development Workflow - Phoenix/Bedrock

**Version:** 3.0.0  
**Last Updated:** 2025-11-25  
**Environment:** Local Development + GitHub Actions CI

> ⚠️ **UPDATED (2025-11-25): dev-01 removed. All CI now runs on GitHub Actions.**

---

## 🎯 **Core Development Policy**

### **Local Development + GitHub Actions CI**

**Workflow:**
1. **Develop locally** - Make code changes on your machine
2. **Test locally** - Build and test with Qt (see [VERSIONS.md](VERSIONS.md) for current version)
3. **Commit and push** - Push changes to repository
4. **CI validates** - GitHub Actions runs tests automatically

> **Note:** For current toolchain versions, see [VERSIONS.md](VERSIONS.md).

**Benefits:**
- ✅ **Consistent CI environment** - GitHub Actions provides reproducible builds
- ✅ **Automated testing** - Unit and integration tests run on every PR
- ✅ **Multi-platform** - CI runs on Linux, macOS, and Windows
- ✅ **No infrastructure maintenance** - No need to manage self-hosted runners
- ✅ **Quality assurance** - Automated validation and checks

---

## 🚀 **Development Workflow**

### **Phase 1: Local Development**

#### **1.1 Environment Setup**

**macOS:**
```bash
# Install Qt (via Homebrew, see VERSIONS.md for current version)
brew install qt@6
export CMAKE_PREFIX_PATH=$(brew --prefix qt@6)

# Or via Qt Installer (see VERSIONS.md for path)
export CMAKE_PREFIX_PATH=~/Qt/<version>/macos
```

**Linux:**
```bash
# Install Qt (via package manager or Qt installer, see VERSIONS.md for current version)
export CMAKE_PREFIX_PATH=/path/to/Qt/<version>/gcc_64
```

#### **1.2 Code Changes**

- ✅ **Make code changes locally**
- ✅ **Test builds locally**
- ✅ **Run unit tests locally** (if configured)
- ✅ **Debug issues locally**

#### **1.3 Testing & Validation**

```bash
# Build locally
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH"
cmake --build build

# Run tests (if available)
ctest --test-dir build --output-on-failure
```

### **Phase 2: Git Workflow**

#### **2.1 Commit Changes**

```bash
# Stage changes
git add .

# Commit with descriptive message
git commit -m "feat: add new feature"

# Push to repository
git push origin feature/your-branch
```

#### **2.2 Create Pull Request**

1. Push your branch to GitHub
2. Create a Pull Request
3. GitHub Actions will automatically:
   - Build the project
   - Run unit tests
   - Run integration tests (Bedrock)
   - Validate code quality

#### **2.3 Review and Merge**

- Review CI results
- Address any failures
- Get code review approval
- Merge when CI passes

---

## 🔄 **CI/CD Pipeline**

### **GitHub Actions Workflows**

**Phoenix CI (`.github/workflows/ci.yml`):**
- ✅ Builds on `ubuntu-latest` with Qt 6.10.1
- ✅ Runs unit tests
- ✅ Validates translations
- ✅ Runs soak tests (scheduled)

**Bedrock CI (`.github/workflows/ci.yml`):**
- ✅ Builds on `ubuntu-latest`
- ✅ Runs unit tests
- ✅ Runs integration tests (Capabilities + XYSine RPC)

### **CI Triggers**

- **Pull Requests**: Automatic build and test on every PR
- **Push to main**: Full CI pipeline
- **Scheduled**: Soak tests run nightly/weekly

---

## 📋 **Best Practices**

### **Before Committing**

1. ✅ Build succeeds locally
2. ✅ Tests pass locally (if available)
3. ✅ Code follows style guidelines
4. ✅ No debug code or temporary files

### **Commit Messages**

Use conventional commit format:
- `feat: add new feature`
- `fix: fix bug in component`
- `docs: update documentation`
- `refactor: restructure code`
- `test: add test cases`

### **Branch Naming**

Use `type/scope/short-desc`:
- `feature/phoenix/new-button`
- `fix/bedrock/memory-leak`
- `docs/update-readme`

---

## 🛠️ **Troubleshooting**

### **CI Build Fails**

1. Check CI logs for specific error
2. Reproduce locally if possible
3. Fix issue and push new commit
4. CI will re-run automatically

### **Local Build Issues**

1. Verify Qt 6.10.1 is installed
2. Check `CMAKE_PREFIX_PATH` is set correctly
3. Ensure submodules are initialized
4. Check CMake output for errors

### **Test Failures**

1. Run tests locally to reproduce
2. Check test logs for details
3. Fix failing tests
4. Verify tests pass before pushing

---

## 📚 **Additional Resources**

- [Phoenix README](../README.md) - Main project documentation
- [CI Workflow System](CI_WORKFLOW_SYSTEM.md) - Detailed CI documentation
- [Setup Mac Dev Env](SETUP_MAC_DEV_ENV.md) - macOS setup guide
- [Coding Standards](CODING_STANDARDS_CPP_QT.md) - Code style guidelines

---

## ⚠️ **Migration Notes**

**dev-01 has been permanently removed.** All development now happens locally, with CI running on GitHub Actions. This provides:

- Better scalability
- No infrastructure maintenance
- Consistent build environments
- Multi-platform testing

If you have questions about the migration, see [DEV01_REMOVAL_SUMMARY.md](DEV01_REMOVAL_SUMMARY.md).
