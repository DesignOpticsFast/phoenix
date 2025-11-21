# Sprint 4.2 Chunk 3C: Echo-based Transport Integration Test Plan

**Date:** 2025-11-21  
**Status:** PLANNING ONLY — No implementation yet  
**Intent:** Design end-to-end Echo transport test for Phoenix LocalSocket ↔ Bedrock PalantirServer

---

## Executive Summary

**Key Finding:** Phoenix's LocalSocket transport uses **Palantir Ping/Pong protocol** (not gRPC Echo).  
- Bedrock's `PalantirServer` handles Ping/Pong via LocalSocket
- Phoenix's `LocalSocketChannel` already has `pingHealthCheck()` method
- Need to create integration test that exercises Ping → Pong round-trip

**Test Focus:** Validate Phoenix LocalSocket ↔ Bedrock PalantirServer communication via Ping/Pong health check.

---

## 1. Bedrock Echo RPC / Palantir Server Mapping

### 1.1 Bedrock Transport Mechanisms

**Bedrock has TWO separate transport mechanisms:**

#### **A. gRPC Echo RPC** (Separate from Palantir)
- **Location:** `tests/bedrock_echo_sanity.cpp`
- **Protocol:** gRPC (not Palantir)
- **Endpoint:** `localhost:50051` (TCP)
- **Status:** ✅ Implemented, tested in Bedrock
- **Usage:** Used by Phoenix's `GrpcUdsChannel` (not LocalSocket)

#### **B. PalantirServer** (LocalSocket - This is what we're testing)
- **Location:** `src/palantir/PalantirServer.cpp`, `src/palantir/bedrock_server.cpp`
- **Protocol:** Palantir (LocalSocket + Protobuf)
- **Socket:** Unix Domain Socket (Linux/macOS) or Named Pipe (Windows)
- **Default Socket Name:** `"palantir_bedrock"` (configurable via `--socket` flag)
- **Status:** ✅ Implemented, handles Ping/Pong
- **Usage:** Used by Phoenix's `LocalSocketChannel`

### 1.2 How to Start Bedrock PalantirServer

**Executable:** `bedrock_server` (built from `src/palantir/bedrock_server.cpp`)

**Command:**
```bash
./bedrock_server [--socket <socket_name>]
```

**Default Behavior:**
- Socket name: `"palantir_bedrock"` (if `--socket` not provided)
- Uses Qt's `QLocalServer` (abstracts Unix sockets/named pipes)
- Listens for LocalSocket connections
- Handles Palantir protocol messages:
  - `CapabilitiesRequest` → `Capabilities`
  - `Ping` → `Pong`
  - `StartJob` → `StartReply` + `ResultMeta` + `DataChunk`
  - `Cancel` → (acknowledgment)

**Environment Variables:**
- None required (socket name via CLI flag)

**Build Target:**
- Target name: `bedrock_server` (if built in Bedrock repo)
- Or: Run `bedrock_server` executable directly

### 1.3 Socket Path / Port Details

**LocalSocket (PalantirServer):**
- **Linux/macOS:** Unix Domain Socket
  - Qt abstracts the actual path (typically `/tmp/<socket_name>` or system temp)
  - Socket name: `"palantir_bedrock"` (default)
- **Windows:** Named Pipe
  - Qt abstracts as `\\.\pipe\<socket_name>`
  - Socket name: `"palantir_bedrock"` (default)

**gRPC Echo (Separate):**
- **Endpoint:** `localhost:50051` (TCP)
- **Not used by LocalSocket transport**

### 1.4 Protocol Mismatch Analysis

**✅ NO MISMATCH:**
- Phoenix `LocalSocketChannel` uses Palantir protocol ✅
- Bedrock `PalantirServer` uses Palantir protocol ✅
- Both use `QLocalSocket` / `QLocalServer` ✅
- Both use length-prefixed Protobuf messages ✅

**Protocol Alignment:**
- ✅ Ping/Pong messages match (`palantir.proto`)
- ✅ Socket naming matches (default: `"palantir_bedrock"`)
- ✅ Message framing matches (4-byte little-endian length prefix)

---

## 2. Phoenix-Side Echo Path Mapping

### 2.1 LocalSocketChannel Ping/Pong Implementation

**Files:**
- `src/transport/LocalSocketChannel.hpp` — `pingHealthCheck()` declaration
- `src/transport/LocalSocketChannel.cpp` — `pingHealthCheck()` implementation

**Current Implementation:**
- ✅ `pingHealthCheck()` method exists (non-blocking, logging-only)
- ✅ Sends `Ping` message via `sendMessage()`
- ✅ Receives `Pong` asynchronously via `parseIncomingData()`
- ✅ Logs Pong receipt with timestamp

**Message Flow:**
1. `pingHealthCheck()` creates `Ping` protobuf message
2. Serializes and sends via `sendMessage()` (length-prefixed)
3. `parseIncomingData()` receives `Pong` response
4. Logs: `"Health check: Received Pong (timestamp: <ms>)"`

### 2.2 Existing Phoenix Test Harnesses

**Relevant Test Files:**

1. **`tests/transport_sanity_tests.cpp`** ✅
   - Has `testLocalSocketConnection()` — tests connection only
   - Has `testLocalSocketCapabilities()` — tests CapabilitiesRequest
   - **Missing:** No Ping/Pong test

2. **`tests/test_xysine_transport.cpp`** ✅
   - Tests `computeXYSine()` end-to-end
   - Uses `LocalSocketChannel`
   - **Missing:** No Ping/Pong test

3. **`tests/echo_dialog_tests.cpp`** ✅
   - Tests Echo dialog UI (gRPC Echo, not LocalSocket Ping)

**Test Infrastructure:**
- ✅ QtTest framework (`QTEST_MAIN`)
- ✅ `QSKIP()` for optional tests (when Bedrock not running)
- ✅ Environment variable support (`PHOENIX_LOCALSOCKET_NAME`)

### 2.3 Integration Points

**Connection:**
- `LocalSocketChannel::connect()` → `QLocalSocket::connectToServer(socketName)`
- Socket name: `PHOENIX_LOCALSOCKET_NAME` env var or `"palantir_bedrock"` default

**Ping/Pong:**
- `LocalSocketChannel::pingHealthCheck()` → sends `Ping` message
- `LocalSocketChannel::parseIncomingData()` → receives `Pong` message
- Logging: `qCDebug(phoenixTransport)` category

**Error Handling:**
- `TransportError` enum (structured error codes)
- `lastError()` / `lastErrorString()` methods
- Error codes: `ConnectionFailed`, `ConnectionTimeout`, `ProtocolError`, `ServerError`, `NetworkError`

---

## 3. Proposed Integration Test Harness

### 3.1 Test Design Options

**Option A: CTest-Only Binary** ⭐ **RECOMMENDED**
- **Name:** `phoenix_transport_ping_test` (or add to existing `transport_sanity_tests`)
- **Type:** QtTest executable
- **Location:** `tests/transport_ping_test.cpp` (or extend `transport_sanity_tests.cpp`)
- **Pros:**
  - Integrates with existing test infrastructure
  - Can use `QSKIP()` for optional execution
  - Matches existing test patterns
  - Easy to run via `ctest` or `make test`
- **Cons:**
  - Requires QtTest framework
  - Less standalone than CLI tool

**Option B: Standalone CLI Tool**
- **Name:** `phoenix_transport_ping` (executable)
- **Type:** Simple `main()` function
- **Location:** `tools/phoenix_transport_ping.cpp` (new directory)
- **Pros:**
  - Standalone, easy to run manually
  - No QtTest dependency
  - Can be used in scripts
- **Cons:**
  - Doesn't integrate with CTest
  - Requires separate CMake target
  - Less consistent with existing tests

**Option C: Test-Only Code Path in Phoenix Binary**
- **Type:** Hidden flag/command-line option in `phoenix_app`
- **Pros:**
  - Uses existing binary
  - No new targets
- **Cons:**
  - Clutters main application
  - Harder to run in CI
  - Not recommended

### 3.2 Recommended Approach: **Option A** (CTest Binary)

**Rationale:**
- Matches existing test patterns (`transport_sanity_tests.cpp`)
- Integrates with CTest infrastructure
- Can use `QSKIP()` for graceful handling when Bedrock not running
- Easy to add to CI

**Implementation:**
- Add new test method to `tests/transport_sanity_tests.cpp`: `testLocalSocketPing()`
- Or create separate `tests/transport_ping_test.cpp` if we want isolation

**Test Flow:**
1. Create `LocalSocketChannel` with socket name from env var
2. Call `connect()` — verify success or skip test
3. Call `pingHealthCheck()` — send Ping
4. Wait briefly (100-200ms) for async Pong
5. Verify:
   - `lastError() == TransportError::NoError`
   - Connection still active
   - Logs show Pong received (if logging enabled)
6. Call `disconnect()`

### 3.3 Test Scenarios

**Scenario 1: Successful Ping/Pong** ✅
- **Prerequisites:** Bedrock PalantirServer running
- **Steps:**
  1. Connect to Bedrock
  2. Send Ping
  3. Wait for Pong
  4. Verify success
- **Expected:** `lastError() == NoError`, connection active

**Scenario 2: Bedrock Not Running** ❌
- **Prerequisites:** Bedrock NOT running
- **Steps:**
  1. Attempt connect (should fail)
  2. Verify error code
- **Expected:** `lastError() == ConnectionFailed` or `ConnectionTimeout`

**Scenario 3: Protocol Error** ⚠️
- **Prerequisites:** Bedrock running but malformed response
- **Steps:**
  1. Connect successfully
  2. Send Ping
  3. Receive malformed response (simulated or real)
  4. Verify error handling
- **Expected:** `lastError() == ProtocolError` (if detectable)

**Scenario 4: Connection Lost During Ping** ⚠️
- **Prerequisites:** Bedrock running, then stopped mid-test
- **Steps:**
  1. Connect successfully
  2. Send Ping
  3. Stop Bedrock server
  4. Verify error handling
- **Expected:** `lastError() == NetworkError` or `ConnectionFailed`

---

## 4. Logging and Error Expectations

### 4.1 Successful Ping/Pong

**Log Messages (with `phoenix.transport` category):**
```
[phoenix.transport] Connected to palantir_bedrock
[phoenix.transport] Health check: Ping sent
[phoenix.transport] Health check: Received Pong (timestamp: 1234567890ms)
```

**Error Codes:**
- `lastError() == TransportError::NoError`
- `isConnected() == true`

**Test Assertions:**
- `QVERIFY(client.isConnected())`
- `QVERIFY(client.lastError() == TransportError::NoError)`
- `QVERIFY(!client.lastErrorString().isEmpty() || client.lastError() == TransportError::NoError)`

### 4.2 Connection Failure (Bedrock Not Running)

**Log Messages:**
```
[phoenix.transport] Connection timeout to palantir_bedrock - Connection timeout
[phoenix.transport] sendMessage: Not connected - Failed to connect to Bedrock server. Please ensure Bedrock is running.
```

**Error Codes:**
- `lastError() == TransportError::ConnectionTimeout` (if timeout)
- OR `lastError() == TransportError::ConnectionFailed` (if refused)

**Test Assertions:**
- `QVERIFY(!client.isConnected())`
- `QVERIFY(client.lastError() == TransportError::ConnectionTimeout || client.lastError() == TransportError::ConnectionFailed)`
- `QVERIFY(!client.lastErrorString().isEmpty())`

**User-Friendly Message:**
- `lastErrorString()` should return: `"Connection to Bedrock server timed out."` or `"Failed to connect to Bedrock server. Please ensure Bedrock is running."`

### 4.3 Protocol Error (Malformed Reply)

**Log Messages:**
```
[phoenix.transport] Connected to palantir_bedrock
[phoenix.transport] Health check: Ping sent
[phoenix.transport] parseIncomingData: Failed to parse Pong - Protocol error communicating with Bedrock server.
```

**Error Codes:**
- `lastError() == TransportError::ProtocolError`

**Test Assertions:**
- `QVERIFY(client.lastError() == TransportError::ProtocolError)`
- `QVERIFY(client.lastErrorString().contains("Protocol error"))`

**Note:** This scenario may be hard to trigger without modifying Bedrock or injecting test data. May be optional for initial implementation.

### 4.4 Network Error (Connection Lost)

**Log Messages:**
```
[phoenix.transport] Connected to palantir_bedrock
[phoenix.transport] Health check: Ping sent
[phoenix.transport] Socket error: <error_code> <error_string> - Network I/O error occurred.
```

**Error Codes:**
- `lastError() == TransportError::NetworkError`

**Test Assertions:**
- `QVERIFY(client.lastError() == TransportError::NetworkError)`
- `QVERIFY(client.lastErrorString().contains("Network"))`

### 4.5 Log Message Format

**Current Format:**
- Uses `qCDebug(phoenixTransport)` and `qCWarning(phoenixTransport)`
- Category: `"phoenix.transport"`
- Format: `"[category] message"`

**Test-Specific Prefixes:**
- **Recommendation:** No special prefix needed
- Use existing `phoenix.transport` category
- Test can filter logs by category if needed

---

## 5. Test Matrix

### 5.1 dev-01 / Linux (Primary) ✅

**Platform:** Linux (Amazon Linux 2023)

**Test Execution:**
- **Method:** CTest (`ctest -R transport_ping` or similar)
- **Manual:** `./tests/transport_ping_test` (if standalone)
- **CI:** Add to `.github/workflows/ci.yml` (if Bedrock available)

**Prerequisites:**
- Bedrock `bedrock_server` executable available
- Socket: Unix Domain Socket (Qt abstracts path)
- Socket name: `"palantir_bedrock"` (default) or `PHOENIX_LOCALSOCKET_NAME` env var

**Known Constraints:**
- ✅ LocalSocket fully supported
- ✅ Qt's `QLocalSocket` handles Unix sockets automatically
- ✅ No special path configuration needed

**CI Integration:**
- **Option A:** Assume Bedrock running (if CI has Bedrock)
- **Option B:** Skip test if Bedrock not available (`QSKIP()`)
- **Option C:** Start Bedrock in CI before test (future enhancement)

### 5.2 macOS (Optional - Planning Only)

**Platform:** macOS

**Test Execution:**
- **Method:** Same as Linux (CTest)
- **Manual:** Same as Linux

**Prerequisites:**
- Same as Linux

**Known Constraints:**
- ✅ LocalSocket fully supported (Unix Domain Sockets)
- ✅ Qt's `QLocalSocket` handles macOS sockets automatically
- ⚠️ May need to verify socket path (Qt may use different temp directory)

**CI Integration:**
- **Status:** Not configured yet (as per Sprint 4.2 decisions)
- **Future:** Add macOS CI job when available

### 5.3 Windows (Optional - Planning Only)

**Platform:** Windows

**Test Execution:**
- **Method:** Same as Linux (CTest)
- **Manual:** Same as Linux

**Prerequisites:**
- Same as Linux

**Known Constraints:**
- ✅ Named Pipes supported (Qt abstracts via `QLocalSocket`)
- ✅ Qt handles Windows named pipes automatically
- ⚠️ Socket name format: `"palantir_bedrock"` (Qt converts to `\\.\pipe\palantir_bedrock`)

**CI Integration:**
- **Status:** Not configured yet (as per Sprint 4.2 decisions)
- **Future:** Add Windows CI job when available

---

## 6. Concrete Implementation Steps (Chunk 3D)

### Step 1: Add Test Method to Existing Test File

**File:** `tests/transport_sanity_tests.cpp`

**Action:**
- Add `testLocalSocketPing()` method to `TransportSanity` class
- Implement Ping/Pong test flow
- Use `QSKIP()` if Bedrock not available

**Code Structure:**
```cpp
void testLocalSocketPing() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString socketName = env.value("PHOENIX_LOCALSOCKET_NAME", "palantir_bedrock");
    
    LocalSocketChannel channel(socketName);
    
    // Try to connect
    if (!channel.connect()) {
        QSKIP("Bedrock PalantirServer not available - skipping Ping test");
    }
    
    QVERIFY(channel.isConnected());
    QVERIFY(channel.lastError() == TransportError::NoError);
    
    // Send Ping
    channel.pingHealthCheck();
    
    // Wait briefly for async Pong (100-200ms)
    QThread::msleep(200);
    
    // Process any incoming data
    // (parseIncomingData() is called automatically via Qt signals)
    
    // Verify still connected and no errors
    QVERIFY(channel.isConnected());
    QVERIFY(channel.lastError() == TransportError::NoError);
    
    channel.disconnect();
    QVERIFY(!channel.isConnected());
}
```

### Step 2: Add Connection Failure Test

**File:** `tests/transport_sanity_tests.cpp`

**Action:**
- Add `testLocalSocketPingConnectionFailure()` method
- Test with invalid socket name
- Verify error codes

### Step 3: Update CMakeLists.txt (If Needed)

**File:** `tests/CMakeLists.txt`

**Action:**
- Verify `transport_sanity_tests` target already exists
- Ensure it links to `phoenix_transport` library
- Ensure `PHX_WITH_TRANSPORT_DEPS` guard is present

**Status:** Likely already configured (check existing `transport_sanity_tests` target)

### Step 4: Add CTest Entry (If New Test File)

**File:** `tests/CMakeLists.txt`

**Action:**
- If creating new test file, add `add_test()` entry
- Use `QSKIP()` for optional execution

**Status:** If adding to existing `transport_sanity_tests`, no changes needed

### Step 5: Update CI Workflow (Optional)

**File:** `.github/workflows/ci.yml`

**Action:**
- Add step to start Bedrock server (if desired)
- Or rely on `QSKIP()` if Bedrock not available
- Run test: `ctest -R transport_sanity` (or specific test name)

**Status:** Optional — can rely on `QSKIP()` for now

### Step 6: Documentation

**File:** `docs/` or `README.md`

**Action:**
- Document how to run Ping test
- Document prerequisites (Bedrock server)
- Document expected behavior

---

## 7. Open Questions for Mark

### Question 1: Test Harness Type

**Question:** Should the Ping/Pong test be:
- **Option A:** Added to existing `transport_sanity_tests.cpp` (recommended)
- **Option B:** New standalone test file `transport_ping_test.cpp`
- **Option C:** Standalone CLI tool (not CTest)

**Recommendation:** **Option A** — Add to existing test file for consistency

**Your Decision:** ❓

---

### Question 2: Bedrock Server Assumption

**Question:** Should the test assume Bedrock is already running, or attempt to launch it?

**Options:**
- **Option A:** Assume Bedrock is running (use `QSKIP()` if not available)
  - Pros: Simple, matches existing test patterns
  - Cons: Requires manual Bedrock startup

- **Option B:** Test attempts to launch Bedrock (future enhancement)
  - Pros: Fully automated
  - Cons: More complex, requires Bedrock binary path, process management

**Recommendation:** **Option A** — Assume running, use `QSKIP()` (matches existing tests)

**Your Decision:** ❓

---

### Question 3: Test Execution Scope

**Question:** Where should this test run?

**Options:**
- **Option A:** CI only (when Bedrock available)
- **Option B:** `make preflight` / `make daily` scripts
- **Option C:** Both CI and preflight/daily
- **Option D:** Manual only (developer runs when needed)

**Recommendation:** **Option C** — Both CI and preflight/daily (with `QSKIP()` if Bedrock unavailable)

**Your Decision:** ❓

---

### Question 4: Async Pong Handling

**Question:** How should the test wait for async Pong response?

**Options:**
- **Option A:** Fixed delay (`QThread::msleep(200)`)
  - Pros: Simple
  - Cons: May be flaky (too short/long)

- **Option B:** Poll `parseIncomingData()` or check for Pong in buffer
  - Pros: More reliable
  - Cons: Requires exposing internal state or adding test helper

- **Option C:** Use Qt event loop with timeout
  - Pros: Proper async handling
  - Cons: More complex

**Recommendation:** **Option A** — Fixed delay (200ms) for initial implementation, can enhance later

**Your Decision:** ❓

---

### Question 5: Error Code Verification Depth

**Question:** How detailed should error code verification be?

**Options:**
- **Option A:** Basic verification (`lastError() == NoError` on success)
  - Pros: Simple, matches current implementation
  - Cons: Less comprehensive

- **Option B:** Detailed verification (check specific error codes for each failure scenario)
  - Pros: More thorough
  - Cons: May require additional error code mapping

**Recommendation:** **Option B** — Verify specific error codes for each scenario (already implemented)

**Your Decision:** ❓

---

### Question 6: Log Verification

**Question:** Should the test verify log messages?

**Options:**
- **Option A:** Don't verify logs (just verify error codes and connection state)
  - Pros: Simple, less brittle
  - Cons: Doesn't verify logging works

- **Option B:** Capture and verify log messages
  - Pros: Verifies logging category works
  - Cons: More complex, requires log capture mechanism

**Recommendation:** **Option A** — Don't verify logs (focus on functional behavior)

**Your Decision:** ❓

---

### Question 7: Test Naming

**Question:** What should the test method be named?

**Options:**
- **Option A:** `testLocalSocketPing()` (matches existing naming)
- **Option B:** `testLocalSocketHealthCheck()` (more descriptive)
- **Option C:** `testLocalSocketPingPong()` (explicit)

**Recommendation:** **Option A** — `testLocalSocketPing()` (concise, matches pattern)

**Your Decision:** ❓

---

## 8. Summary

### Current State

**✅ What Exists:**
- Bedrock `PalantirServer` handles Ping/Pong ✅
- Phoenix `LocalSocketChannel::pingHealthCheck()` implemented ✅
- Existing test infrastructure (`transport_sanity_tests.cpp`) ✅
- Structured error codes and logging category ✅

**📍 What's Missing:**
- Integration test for Ping/Pong round-trip ❌
- Test for connection failure error codes ❌
- CI integration (optional) ❌

### Proposed Test Design

**Test Type:** CTest binary (QtTest)  
**Location:** Add to `tests/transport_sanity_tests.cpp`  
**Method Name:** `testLocalSocketPing()`  
**Assumption:** Bedrock PalantirServer already running (use `QSKIP()` if not)  
**Verification:** Error codes, connection state, basic functionality  

### Implementation Plan (Chunk 3D)

1. Add `testLocalSocketPing()` to `transport_sanity_tests.cpp`
2. Add `testLocalSocketPingConnectionFailure()` for error testing
3. Verify CMake configuration (should already be correct)
4. Test manually on dev-01
5. Optionally add to CI (with `QSKIP()` fallback)

### Expected Outcomes

**Successful Test:**
- Connects to Bedrock
- Sends Ping
- Receives Pong (async)
- `lastError() == NoError`
- Connection remains active

**Failure Test:**
- Fails to connect (Bedrock not running)
- `lastError() == ConnectionFailed` or `ConnectionTimeout`
- User-friendly error message available

---

**Next Steps:** Await Mark's decisions on open questions, then proceed with implementation (Chunk 3D).

