# CI Integration Notes - Sprint 4.5

**Date:** 2025-01-25  
**Sprint:** 4.5  
**Status:** Recommendations for CI Integration

---

## Integration Tests in CI

### Required Test Targets

The following integration tests should be added to CI:

```bash
# Bedrock integration tests
ctest -R envelope_helpers
ctest -R CapabilitiesIntegrationTest
ctest -R XYSineIntegrationTest
```

### Platform Requirements

#### Linux
- **Status:** Required
- **Sanitizers:** TSAN and ASAN supported
- **Note:** TSAN validation should be done on Linux (macOS/ARM64 support may be limited)

#### macOS
- **Status:** Required
- **Sanitizers:** ASAN supported, TSAN unreliable
- **Note:** Use ASAN with `detect_container_overflow=0` to avoid protobuf false positives

#### Windows
- **Status:** Best-effort
- **Sanitizers:** ASAN supported via MSVC
- **Note:** Do not block build on Windows failures

---

## Sanitizer Configuration

### TSAN (ThreadSanitizer)

**Platform:** Linux only (macOS TSAN unreliable)

**Build Flags:**
```cmake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
```

**Runtime:**
```bash
export TSAN_OPTIONS="halt_on_error=1"
./tests/integration/integration_tests
```

### ASAN (AddressSanitizer)

**Platform:** All platforms

**Build Flags:**
```cmake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -O1")
```

**Runtime (to avoid protobuf false positives):**
```bash
export ASAN_OPTIONS="detect_container_overflow=0"
./tests/integration/integration_tests
```

**Note:** Protobuf container-overflow false positives are known and do not affect production code. Tests pass without ASAN or with `detect_container_overflow=0`.

---

## CI Workflow Recommendations

### Example GitHub Actions Workflow

```yaml
name: Integration Tests

on:
  push:
    branches: [ main, sprint/* ]
  pull_request:
    branches: [ main ]

jobs:
  integration-tests:
    name: Integration Tests - ${{ matrix.os }}
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest]
    
    steps:
      - uses: actions/checkout@v4
      
      - name: Setup Qt
        # ... Qt setup steps ...
      
      - name: Configure CMake
        run: |
          cmake -B build -S . \
            -DCMAKE_BUILD_TYPE=Release \
            -DBEDROCK_WITH_TRANSPORT_DEPS=ON
      
      - name: Build
        run: cmake --build build --target integration_tests
      
      - name: Run Integration Tests
        run: |
          cd build
          ctest -R envelope_helpers --output-on-failure
          ctest -R CapabilitiesIntegrationTest --output-on-failure
          ctest -R XYSineIntegrationTest --output-on-failure
      
      - name: Upload Test Logs on Failure
        if: failure()
        uses: actions/upload-artifact@v3
        with:
          name: integration-test-logs-${{ matrix.os }}
          path: build/Testing/**/*.xml
```

### Example for Linux with TSAN

```yaml
  integration-tests-tsan:
    name: Integration Tests - Linux TSAN
    runs-on: ubuntu-latest
    
    steps:
      - uses: actions/checkout@v4
      
      - name: Setup Qt
        # ... Qt setup steps ...
      
      - name: Configure CMake with TSAN
        run: |
          cmake -B build -S . \
            -DCMAKE_BUILD_TYPE=Debug \
            -DBEDROCK_WITH_TRANSPORT_DEPS=ON \
            -DCMAKE_CXX_FLAGS="-fsanitize=thread"
      
      - name: Build
        run: cmake --build build --target integration_tests
      
      - name: Run Integration Tests with TSAN
        env:
          TSAN_OPTIONS: "halt_on_error=1"
        run: |
          cd build
          ctest -R CapabilitiesIntegrationTest --output-on-failure
          ctest -R XYSineIntegrationTest --output-on-failure
```

---

## Test Artifacts

### On Failure

Upload the following artifacts when tests fail:

1. **Test Logs:**
   - `build/Testing/**/*.xml` (CTest XML output)
   - `build/Testing/Temporary/LastTest.log`

2. **Build Logs:**
   - `build/CMakeCache.txt`
   - `build/CMakeFiles/CMakeError.log`

3. **Core Dumps (if available):**
   - `core.*` files

---

## Known Issues

### ASAN Container Overflow

**Symptom:** ASAN reports container-overflow in protobuf destructor

**Impact:** False positive, tests pass without ASAN

**Workaround:** Set `ASAN_OPTIONS=detect_container_overflow=0`

**Status:** Documented, not blocking

### macOS TSAN

**Symptom:** TSAN may not work reliably on macOS/ARM64

**Impact:** TSAN validation should be done on Linux

**Workaround:** Skip TSAN on macOS, run on Linux only

**Status:** Platform limitation, not a bug

---

## Test Timeouts

### Recommended Timeouts

- **Individual Test:** 30 seconds
- **Test Suite:** 5 minutes
- **Full CI Run:** 15 minutes

### Timeout Configuration

```yaml
- name: Run Integration Tests
  timeout-minutes: 5
  run: |
    cd build
    ctest -R "*IntegrationTest*" --output-on-failure
```

---

## Status

✅ **CI Integration Recommendations Complete**

These recommendations should be integrated into the CI pipeline. Integration tests are ready for CI and have been validated locally on macOS and Linux.

