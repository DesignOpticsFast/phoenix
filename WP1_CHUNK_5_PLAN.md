# WP1 Chunk 5: Phoenix TransportClient Skeleton + Legacy Cleanup - Implementation Plan

**Date:** 2025-11-17  
**Status:** Planning Complete - Ready for Review  
**WP1 Chunk:** 5 of 10

---

## Overview

Create the foundational transport client layer for Phoenix (matching Sprint 4.2 architecture) and remove all pre-4.2 Palantir/transport artifacts. This establishes a clean foundation for WP2 transport implementation.

**Critical Constraint:** All builds must use Qt 6.10.0 via `-DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64`

---

## Legacy Artifacts Identified

### Files/Directories to Remove:

1. **`proto/palantir.proto`**
   - Old proto file (not part of new contracts/ architecture)
   - Generated code: `src/palantir/palantir.pb.h` and `src/palantir/palantir.pb.cc`

2. **`src/palantir/` directory (entire directory)**
   - `PalantirClient.hpp` - Old QLocalSocket-based client
   - `PalantirClient.cpp` - Old client implementation
   - `PalantirLogging.h` - Old logging categories
   - `PalantirLogging.cpp` - Old logging implementation
   - `palantir.pb.h` - Generated proto header (from old proto)
   - `palantir.pb.cc` - Generated proto source (from old proto)

3. **CMakeLists.txt references**
   - `PHX_WITH_PALANTIR` option (line 19)
   - Conditional inclusion block (lines 138-144)
   - Compile definitions for `PHX_WITH_PALANTIR` (lines 32-33)

4. **`src/main.cpp` references**
   - Palantir logging category filter rules (lines 51-53)

### What to Keep:

- `PHX_WITH_PALANTIR_CONTRACTS` option (for new contracts submodule)
- `contracts/` submodule (new architecture)
- `.contract-version` file

---

## New Transport Layer Structure

### Directory Structure:

```
src/transport/
├── TransportClient.hpp      # Pure virtual interface
├── GrpcUdsChannel.hpp       # gRPC UDS stub header
├── GrpcUdsChannel.cpp       # gRPC UDS stub implementation
├── LocalSocketChannel.hpp    # LocalSocket stub header
└── LocalSocketChannel.cpp    # LocalSocket stub implementation
```

### TransportClient Interface:

**File:** `src/transport/TransportClient.hpp`

```cpp
#pragma once

#include <QString>

class TransportClient {
public:
    virtual ~TransportClient() = default;

    // Connection management
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    // Backend identification
    virtual QString backendName() const = 0;

    // Placeholders for future request/response API
    // virtual Response sendRequest(const Request& req) = 0;
};
```

**Notes:**
- Pure virtual interface (no QObject, no signals/slots yet)
- Uses Qt types (`QString`) for compatibility
- Minimal API surface (will expand in WP2)

### GrpcUdsChannel Stub:

**File:** `src/transport/GrpcUdsChannel.hpp`

```cpp
#pragma once

#include "TransportClient.hpp"
#include <QString>

class GrpcUdsChannel : public TransportClient {
public:
    explicit GrpcUdsChannel(const QString& socketPath = QString());
    ~GrpcUdsChannel() override = default;

    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    QString backendName() const override;

private:
    QString socketPath_;
    bool connected_;
};
```

**File:** `src/transport/GrpcUdsChannel.cpp`

```cpp
#include "GrpcUdsChannel.hpp"

GrpcUdsChannel::GrpcUdsChannel(const QString& socketPath)
    : socketPath_(socketPath.isEmpty() ? QStringLiteral("palantir_bedrock") : socketPath)
    , connected_(false)
{
}

bool GrpcUdsChannel::connect() {
    // Stub: always returns false
    return false;
}

void GrpcUdsChannel::disconnect() {
    // Stub: no-op
    connected_ = false;
}

bool GrpcUdsChannel::isConnected() const {
    return connected_;
}

QString GrpcUdsChannel::backendName() const {
    return QStringLiteral("gRPC (UDS) stub");
}
```

### LocalSocketChannel Stub:

**File:** `src/transport/LocalSocketChannel.hpp`

```cpp
#pragma once

#include "TransportClient.hpp"
#include <QString>

class LocalSocketChannel : public TransportClient {
public:
    explicit LocalSocketChannel(const QString& socketPath = QString());
    ~LocalSocketChannel() override = default;

    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    QString backendName() const override;

private:
    QString socketPath_;
    bool connected_;
};
```

**File:** `src/transport/LocalSocketChannel.cpp`

```cpp
#include "LocalSocketChannel.hpp"

LocalSocketChannel::LocalSocketChannel(const QString& socketPath)
    : socketPath_(socketPath.isEmpty() ? QStringLiteral("palantir_bedrock") : socketPath)
    , connected_(false)
{
}

bool LocalSocketChannel::connect() {
    // Stub: always returns false
    return false;
}

void LocalSocketChannel::disconnect() {
    // Stub: no-op
    connected_ = false;
}

bool LocalSocketChannel::isConnected() const {
    return connected_;
}

QString LocalSocketChannel::backendName() const {
    return QStringLiteral("LocalSocket stub");
}
```

---

## CMake Integration

### New Library Target:

**Add to `CMakeLists.txt` (after `phoenix_canonical_json`):**

```cmake
# ---- transport library ----------------------------------------------------
add_library(phoenix_transport STATIC
  src/transport/TransportClient.hpp
  src/transport/GrpcUdsChannel.hpp
  src/transport/GrpcUdsChannel.cpp
  src/transport/LocalSocketChannel.hpp
  src/transport/LocalSocketChannel.cpp
)

target_include_directories(phoenix_transport PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(phoenix_transport PUBLIC
  Qt6::Core
)
```

**Notes:**
- No gRPC or protobuf dependencies yet (WP2)
- Links only Qt6::Core (for QString)
- Static library (matches other Phoenix libraries)

### Remove Old Palantir Code:

**Remove from `CMakeLists.txt`:**
- Line 19: `option(PHX_WITH_PALANTIR ...)`
- Lines 32-33: Compile definitions for `PHX_WITH_PALANTIR`
- Lines 138-144: Conditional inclusion of old PalantirClient files

---

## Test Implementation

### New Test File:

**File:** `tests/transport_sanity_tests.cpp`

```cpp
#include <QtTest/QtTest>
#include "transport/TransportClient.hpp"
#include "transport/GrpcUdsChannel.hpp"
#include "transport/LocalSocketChannel.hpp"

class TransportSanity : public QObject {
    Q_OBJECT

private slots:
    void testStubNames() {
        GrpcUdsChannel grpc;
        LocalSocketChannel lsc;

        QVERIFY(grpc.backendName().contains("gRPC"));
        QVERIFY(lsc.backendName().contains("LocalSocket"));
    }

    void testStubConnection() {
        GrpcUdsChannel grpc;
        LocalSocketChannel lsc;

        QVERIFY(!grpc.isConnected());
        QVERIFY(!lsc.isConnected());

        QVERIFY(!grpc.connect());  // Stub always returns false
        QVERIFY(!lsc.connect());   // Stub always returns false

        QVERIFY(!grpc.isConnected());
        QVERIFY(!lsc.isConnected());
    }
};

QTEST_MAIN(TransportSanity)
#include "transport_sanity_tests.moc"
```

### CMake Test Integration:

**Add to `tests/CMakeLists.txt`:**

```cmake
# transport sanity tests
if(BUILD_TESTING)
  add_executable(transport_sanity_tests
    transport_sanity_tests.cpp
  )

  target_link_libraries(transport_sanity_tests
    PRIVATE
      phoenix_transport
      Qt6::Test
      Qt6::Core
  )

  target_include_directories(transport_sanity_tests
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME transport_sanity_tests COMMAND transport_sanity_tests)
endif()
```

---

## Implementation Steps

### Phase 1: Legacy Cleanup

1. **Remove old proto file:**
   - Delete `proto/palantir.proto`
   - Delete `src/palantir/palantir.pb.h`
   - Delete `src/palantir/palantir.pb.cc`

2. **Remove old Palantir client code:**
   - Delete entire `src/palantir/` directory:
     - `PalantirClient.hpp`
     - `PalantirClient.cpp`
     - `PalantirLogging.h`
     - `PalantirLogging.cpp`
     - `palantir.pb.h`
     - `palantir.pb.cc`

3. **Clean up CMakeLists.txt:**
   - Remove `PHX_WITH_PALANTIR` option
   - Remove conditional inclusion block
   - Remove compile definitions for `PHX_WITH_PALANTIR`

4. **Clean up src/main.cpp:**
   - Remove palantir logging category filter rules (lines 51-53)

5. **Verify proto directory:**
   - If `proto/` directory is now empty, consider removing it (or leave for future use)

### Phase 2: New Transport Layer

6. **Create transport directory:**
   - Create `src/transport/` directory

7. **Create TransportClient interface:**
   - Write `src/transport/TransportClient.hpp`
   - Pure virtual interface with minimal API

8. **Create GrpcUdsChannel stub:**
   - Write `src/transport/GrpcUdsChannel.hpp`
   - Write `src/transport/GrpcUdsChannel.cpp`
   - Implement stub methods (all return false/no-op)

9. **Create LocalSocketChannel stub:**
   - Write `src/transport/LocalSocketChannel.hpp`
   - Write `src/transport/LocalSocketChannel.cpp`
   - Implement stub methods (all return false/no-op)

### Phase 3: CMake Integration

10. **Add phoenix_transport library:**
    - Add library target to `CMakeLists.txt`
    - Link Qt6::Core
    - Set include directories

### Phase 4: Testing

11. **Create transport sanity test:**
    - Write `tests/transport_sanity_tests.cpp`
    - Test stub names and connection behavior

12. **Wire test into CMake:**
    - Add test executable to `tests/CMakeLists.txt`
    - Link phoenix_transport and Qt6::Test
    - Add test to CTest

### Phase 5: Verification

13. **Local build verification:**
    ```bash
    cmake -S . -B build/transport \
      -DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64 \
      -DBUILD_TESTING=ON
    cmake --build build/transport
    ctest --test-dir build/transport -R transport_sanity
    ```

14. **Verify legacy cleanup:**
    - Confirm `proto/palantir.proto` is gone
    - Confirm `src/palantir/` directory is gone
    - Confirm no references to old PalantirClient in codebase
    - Confirm `PHX_WITH_PALANTIR` option is removed

15. **CI verification:**
    - Push changes and verify CI passes
    - Confirm contract parity check still runs
    - Verify all platforms (macOS, Linux, Windows) build successfully

---

## Questions & Decisions

### ✅ Resolved:

1. **Legacy cleanup scope:** Remove all pre-4.2 Palantir artifacts (proto, old client, logging)
2. **Transport interface:** Pure virtual (no QObject) for now, will add signals/slots in WP2
3. **Stub implementation:** All methods return false/no-op (no networking yet)
4. **Test scope:** Minimal sanity test (stub names and connection behavior)
5. **Qt version:** Always use Qt 6.10.0 via CMAKE_PREFIX_PATH

### ❓ Open Questions:

1. **Proto directory:** Should we remove the empty `proto/` directory, or keep it for future use?
   - **Recommendation:** Keep it (may be useful for future local proto files)

2. **TransportClient namespace:** Should transport classes be in a namespace?
   - **Recommendation:** No namespace for now (matches Phoenix style), can add later if needed

3. **Socket path defaults:** Should default socket paths match old PalantirClient (`palantir_bedrock`)?
   - **Recommendation:** Yes, use `palantir_bedrock` as default (maintains compatibility)

4. **Include guards vs pragma once:** Use `#pragma once` (matches Phoenix style) ✅

5. **Test naming:** Use `transport_sanity_tests` (matches existing test naming pattern) ✅

---

## Verification Checklist

- [ ] `proto/palantir.proto` removed
- [ ] `src/palantir/` directory removed completely
- [ ] `PHX_WITH_PALANTIR` option removed from CMakeLists.txt
- [ ] Old Palantir logging references removed from src/main.cpp
- [ ] `src/transport/` directory created
- [ ] `TransportClient.hpp` created (pure virtual interface)
- [ ] `GrpcUdsChannel.hpp/cpp` created (stub implementation)
- [ ] `LocalSocketChannel.hpp/cpp` created (stub implementation)
- [ ] `phoenix_transport` library target added to CMakeLists.txt
- [ ] `transport_sanity_tests.cpp` created
- [ ] Test wired into tests/CMakeLists.txt
- [ ] Local build succeeds with Qt 6.10.0
- [ ] Transport sanity tests pass
- [ ] CI passes on all platforms
- [ ] Contract parity check still works
- [ ] No references to old PalantirClient in codebase

---

## Expected Outcomes

### Files Removed:
- `proto/palantir.proto`
- `src/palantir/` (entire directory)

### Files Created:
- `src/transport/TransportClient.hpp`
- `src/transport/GrpcUdsChannel.hpp`
- `src/transport/GrpcUdsChannel.cpp`
- `src/transport/LocalSocketChannel.hpp`
- `src/transport/LocalSocketChannel.cpp`
- `tests/transport_sanity_tests.cpp`

### Files Modified:
- `CMakeLists.txt` (remove old Palantir, add phoenix_transport)
- `tests/CMakeLists.txt` (add transport_sanity_tests)
- `src/main.cpp` (remove palantir logging rules)

### Build Output:
- `phoenix_transport` static library builds successfully
- `transport_sanity_tests` executable builds and passes
- All platforms (macOS, Linux, Windows) build successfully

---

## Summary

This chunk establishes a clean transport layer foundation by:
1. **Removing all legacy Palantir artifacts** (old proto, old client, old logging)
2. **Creating new transport skeleton** (TransportClient interface + two stub implementations)
3. **Adding minimal tests** (sanity checks for stub behavior)
4. **Ensuring Qt 6.10.0 compliance** (all builds use correct CMAKE_PREFIX_PATH)

The result is a clean, minimal transport layer ready for WP2 implementation.

---

**Ready for review and approval before implementation.**

