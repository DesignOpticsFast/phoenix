# Crucible Environment Checklist

**Date:** 2025-11-25  
**Platform:** macOS (Darwin)  
**Hostname:** Crucible

---

## Build Directories Status

| Directory | Status | Test Binary | Notes |
|-----------|--------|-------------|-------|
| `build/` | ✅ EXISTS | ❌ MISSING | Directory exists but `palantir_integration_tests` not found |
| `build_tsan/` | ❌ MISSING | N/A | Directory does not exist |
| `build_asan/` | ❌ MISSING | N/A | Directory does not exist |
| `build_coverage/` | ❌ MISSING | N/A | Directory does not exist |

### Details

- **`build/`**: Exists but contains no `palantir_integration_tests` binary
  - Found subdirectories: `integration/`, `integration_transport/`, `tsan/`, `debug/`, `release/`, etc.
  - No test executables found in any subdirectory
  - No `envelope_helpers_test` found (fallback binary)

- **`build_tsan/`**: Does not exist at root level
  - Note: `build/tsan/` exists but is a subdirectory, not the canonical location

- **`build_asan/`**: Does not exist

- **`build_coverage/`**: Does not exist

---

## Tool Availability Status

| Tool | Status | Version | Notes |
|------|--------|---------|-------|
| `lcov` | ✅ FOUND | 2.3.2-1 | Installed and ready |
| `genhtml` | ✅ FOUND | 2.3.2-1 | Installed and ready |
| `cppcheck` | ✅ FOUND | 2.18.0 | Installed and ready |
| `clang-tidy` | ❌ MISSING | N/A | Optional (requires `compile_commands.json`) |
| `valgrind` | ❌ MISSING | N/A | Expected on macOS (will be SKIPPED) |

**Last Updated:** 2025-11-25 (Phase 5B)

---

## Suggested Setup Commands

### 1. Install Required Tools (Homebrew)

```bash
# Install lcov and genhtml (part of lcov package)
brew install lcov

# Install cppcheck
brew install cppcheck

# Install clang-tidy (optional, but recommended)
brew install llvm
# Note: After installation, you may need to add llvm to PATH:
# export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
```

**Verify installations:**
```bash
lcov --version
genhtml --version
cppcheck --version
clang-tidy --version
```

### 2. Create TSAN Build

```bash
cd /Users/underlord/workspace/phoenix

# Create TSAN build directory
mkdir -p build_tsan

# Configure with TSAN flags
cmake -S . -B build_tsan \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_CXX_FLAGS="-fsanitize=thread -g -O1" \
  -DCMAKE_C_FLAGS="-fsanitize=thread -g -O1" \
  -DPHX_WITH_TRANSPORT_DEPS=ON

# Build
cmake --build build_tsan --target palantir_integration_tests
# Or if that target doesn't exist, build all tests:
cmake --build build_tsan
```

**Verify:**
```bash
ls -lh build_tsan/palantir_integration_tests
# Or check in tests subdirectory:
ls -lh build_tsan/tests/palantir_integration_tests
```

### 3. Create ASAN Build

```bash
cd /Users/underlord/workspace/phoenix

# Create ASAN build directory
mkdir -p build_asan

# Configure with ASAN flags
cmake -S . -B build_asan \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -g -O1" \
  -DCMAKE_C_FLAGS="-fsanitize=address -g -O1" \
  -DPHX_WITH_TRANSPORT_DEPS=ON

# Build
cmake --build build_asan --target palantir_integration_tests
# Or build all tests:
cmake --build build_asan
```

**Verify:**
```bash
ls -lh build_asan/palantir_integration_tests
# Or check in tests subdirectory:
ls -lh build_asan/tests/palantir_integration_tests
```

### 4. Create Coverage Build

```bash
cd /Users/underlord/workspace/phoenix

# Create coverage build directory
mkdir -p build_coverage

# Configure with coverage flags
cmake -S . -B build_coverage \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_CXX_FLAGS="--coverage -g -O0" \
  -DCMAKE_C_FLAGS="--coverage -g -O0" \
  -DCMAKE_EXE_LINKER_FLAGS="--coverage" \
  -DPHX_WITH_TRANSPORT_DEPS=ON

# Build
cmake --build build_coverage --target palantir_integration_tests
# Or build all tests:
cmake --build build_coverage
```

**Verify:**
```bash
ls -lh build_coverage/palantir_integration_tests
# Or check in tests subdirectory:
ls -lh build_coverage/tests/palantir_integration_tests
```

### 5. Ensure Test Binary Exists in Normal Build

If `palantir_integration_tests` doesn't exist in `build/`, you may need to:

```bash
cd /Users/underlord/workspace/phoenix

# Rebuild normal build with transport deps enabled
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DPHX_WITH_TRANSPORT_DEPS=ON

# Build test targets
cmake --build build --target palantir_integration_tests
# Or build all:
cmake --build build
```

**Note:** The exact target name may vary. Check `tests/CMakeLists.txt` for the actual test executable names.

---

## Test Binary Discovery

The framework searches for test binaries in this order:
1. `{build_dir}/palantir_integration_tests`
2. `{build_dir}/tests/palantir_integration_tests`
3. `{build_dir}/tests/envelope_helpers_test` (fallback)

If none of these are found, the task will be marked as ⚠️ SKIPPED.

---

## Current Status Summary

**Ready for QA Run:** ⚠️ **PARTIAL**

**Installed:**
- ✅ Tools: lcov (2.3.2-1), genhtml (2.3.2-1), cppcheck (2.18.0)

**Still Missing:**
- ❌ All build directories (build_tsan/, build_asan/, build_coverage/)
- ❌ Test binary in build/ directory (palantir_integration_tests)
- ❌ clang-tidy (optional)

**Expected Behavior:**
- Static Analysis task can now run (cppcheck available)
- Coverage task can run if build_coverage/ is created
- TSAN, ASAN, Soak tasks will SKIP (no builds/test binaries)
- Valgrind will SKIP (expected on macOS)

**Last Updated:** 2025-11-25 (Phase 5B)

---

## Next Steps

1. **Install tools** using Homebrew commands above
2. **Create builds** using CMake commands above
3. **Verify test binaries** exist in each build directory
4. **Re-run environment check** to confirm readiness
5. **Run medium-length QA test** to validate setup
6. **Run first overnight QA** once environment is ready

---

**Last Updated:** 2025-11-25  
**Checked By:** UnderLord

