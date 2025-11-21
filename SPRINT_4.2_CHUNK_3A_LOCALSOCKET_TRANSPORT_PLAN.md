# Sprint 4.2 Chunk 3A: Phoenix LocalSocket Transport Layer Plan

**Date:** 2025-11-21  
**Status:** PLANNING ONLY — No implementation yet  
**Intent:** Design and plan LocalSocket transport layer integration for Phoenix before making changes

---

## Executive Summary

**Key Finding:** LocalSocket transport infrastructure **already exists** in Phoenix!  
- `LocalSocketChannel` class implements `TransportClient` interface
- `AnalysisWorker` already integrates with `LocalSocketChannel`
- Results flow correctly into `XYPlotViewGraphs`
- Socket path configuration exists

**This plan focuses on:**
- Documenting existing implementation
- Identifying gaps and improvements needed
- Proposing enhancements for Sprint 4.2
- Listing open questions for Mark

---

## 1. Phoenix Transport Layer Code Map

### Directory Tree

```
phoenix/
├── src/
│   ├── transport/
│   │   ├── TransportClient.hpp              # Base interface (✅ exists)
│   │   ├── TransportFactory.hpp/cpp          # Factory for creating clients (✅ exists)
│   │   ├── LocalSocketChannel.hpp/cpp       # LocalSocket implementation (✅ exists)
│   │   └── GrpcUdsChannel.hpp/cpp           # gRPC UDS implementation (✅ exists)
│   │
│   ├── analysis/
│   │   ├── AnalysisWorker.hpp/cpp            # Worker thread for analysis (✅ uses transport)
│   │   ├── AnalysisResults.hpp               # XYSineResult struct (✅ exists)
│   │   └── demo/
│   │       └── XYSineDemo.hpp/cpp            # Local demo computation (✅ exists)
│   │
│   ├── ui/analysis/
│   │   ├── XYAnalysisWindow.hpp/cpp          # Analysis window UI (✅ uses AnalysisWorker)
│   │   └── AnalysisWindowManager.hpp/cpp     # Window management (✅ exists)
│   │
│   └── plot/
│       └── XYPlotViewGraphs.hpp/cpp          # Qt Graphs plot view (✅ receives results)
│
├── proto/
│   └── palantir.proto                        # Protobuf schema (✅ exists)
│
├── contracts/
│   └── docs/                                 # Contract documentation (✅ exists)
│
└── tests/
    └── test_xysine_transport.cpp             # Transport tests (✅ exists)
```

### Key Files Analysis

**Transport Layer:**
- ✅ `TransportClient.hpp` — Base interface with `connect()`, `disconnect()`, `isConnected()`, `backendName()`
- ✅ `LocalSocketChannel.hpp/cpp` — Full LocalSocket implementation
  - Uses `QLocalSocket` for Unix domain sockets
  - Length-prefixed protobuf messages (4-byte little-endian)
  - Implements `computeXYSine()`, `requestCapabilities()`, `cancelJob()`
  - Socket name configurable via `PHOENIX_LOCALSOCKET_NAME` env var (default: `"palantir_bedrock"`)

**Analysis Integration:**
- ✅ `AnalysisWorker.cpp` — Already uses `LocalSocketChannel` when `PHX_WITH_TRANSPORT_DEPS` enabled
  - Creates client in `executeCompute()`
  - Calls `client->computeXYSine()` with progress callback
  - Handles cancel via `client->cancelJob()`
  - Emits results via `finished()` signal

**UI Integration:**
- ✅ `XYAnalysisWindow.cpp` — Connects to `AnalysisWorker::finished` signal
  - Converts `XYSineResult` to `std::vector<QPointF>`
  - Calls `m_plotView->setData()` to update plot

**Protobuf Schema:**
- ✅ `proto/palantir.proto` — Complete schema for Palantir protocol
  - `CapabilitiesRequest/Capabilities`
  - `StartJob/StartReply`
  - `ComputeSpec`
  - `Progress`
  - `ResultMeta`
  - `DataChunk`
  - `Cancel`

---

## 2. Current TransportClient Interface

### Existing Interface (`TransportClient.hpp`)

```cpp
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

### LocalSocketChannel Extended Interface

`LocalSocketChannel` extends `TransportClient` with:

```cpp
class LocalSocketChannel : public TransportClient {
public:
    // Connection (inherited from TransportClient)
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    QString backendName() const override;
    
    // Palantir protocol methods
    bool requestCapabilities(QStringList& features);
    bool computeXYSine(const QMap<QString, QVariant>& params,
                       XYSineResult& outResult,
                       std::function<void(double, const QString&)> progressCallback = nullptr);
    bool cancelJob(const QString& jobId = QString());
    QString currentJobId() const;
};
```

### Assessment

**✅ Strengths:**
- Clean separation: base interface + concrete implementation
- Non-blocking: uses Qt signals/slots for async operations
- Error handling: returns `bool` + logs via `qWarning()`
- Progress support: callback mechanism for progress updates
- Cancel support: job ID tracking + cancel message

**⚠️ Gaps/Questions:**
1. **Socket Path:** Currently uses `PHOENIX_LOCALSOCKET_NAME` env var (default: `"palantir_bedrock"`)
   - **Question:** Should we use `$XDG_RUNTIME_DIR/bedrock.sock` on Linux/macOS?
   - **Question:** What about Windows named pipe path?

2. **Error Reporting:** Currently uses `qWarning()` + return `bool`
   - **Question:** Should we integrate with Phoenix error matrix/UI error reporting?

3. **Connection Strategy:** Currently connects on-demand in `AnalysisWorker`
   - **Question:** Should Phoenix auto-start Bedrock, or only connect to existing process?

4. **Threading:** Currently uses `QEventLoop` in worker thread (non-blocking UI)
   - ✅ **Confirmed:** Already non-blocking on UI thread (uses `AnalysisWorker` in separate thread)

---

## 3. LocalSocketTransport Implementation Analysis

### Current Implementation (`LocalSocketChannel.cpp`)

**Socket Path:**
- **Current:** Uses `PHOENIX_LOCALSOCKET_NAME` env var (default: `"palantir_bedrock"`)
- **Location:** Set in constructor, fallback to env var in `connect()`
- **Platform:** Uses Qt's `QLocalSocket` (abstracts Unix sockets on Linux/macOS, named pipes on Windows)

**Connection Strategy:**
- **Current:** Synchronous connect with 3-second timeout (`waitForConnected(3000)`)
- **Reconnect:** No automatic reconnect logic (fails on first attempt)
- **Error Handling:** Returns `false` + logs warning

**Timeout Behavior:**
- **Connection:** 3 seconds (`waitForConnected(3000)`)
- **Message Send:** 3 seconds (`waitForBytesWritten(3000)`)
- **Message Receive:** 5 seconds (via `QTimer` + `QEventLoop`)
- **Computation:** 10 seconds for `ResultMeta`, 5 seconds per `DataChunk`

**Error Handling:**
- **Current:** Returns `bool` + `qWarning()` logging
- **No structured error codes** (just success/failure)
- **No error propagation to UI** (except via `AnalysisWorker::finished(false, ...)`)

**Logging:**
- **Current:** Uses `qDebug()` and `qWarning()`
- **No structured logging** (no log levels, no categories)
- **No integration with Phoenix logging system** (if one exists)

**Cancel Mechanism:**
- **Current:** Sends `Cancel` protobuf message with job ID
- **Job ID tracking:** Stores `m_currentJobId` from last `computeXYSine()` call
- **Cancel during compute:** Handled via progress callback checking for `"CANCELLED"` status

**Threading:**
- ✅ **Non-blocking UI:** Uses `QEventLoop` in worker thread (not UI thread)
- ✅ **Signal-based:** Uses Qt signals for async operations
- ✅ **Thread-safe:** `QLocalSocket` is thread-safe when used in single thread

### Proposed Enhancements

**1. Socket Path Standardization**
- **Linux/macOS:** Use `$XDG_RUNTIME_DIR/bedrock.sock` (or fallback to `/tmp/bedrock.sock`)
- **Windows:** Use named pipe `\\.\pipe\bedrock` (Qt handles this automatically)
- **Override:** Keep `PHOENIX_LOCALSOCKET_NAME` env var for testing/override

**2. Error Reporting Integration**
- **Structured errors:** Define error codes/categories
- **UI integration:** Map errors to user-friendly messages
- **Error matrix:** Document error conditions and responses

**3. Connection Resilience**
- **Auto-retry:** Retry connection with exponential backoff (optional)
- **Connection state:** Track connection state and emit signals
- **Health check:** Periodic ping/pong to detect disconnections

**4. Logging Enhancement**
- **Structured logging:** Use Qt logging categories (`qt.category.transport`)
- **Log levels:** Differentiate debug/info/warning/error
- **Integration:** Connect to Phoenix logging system (if exists)

**5. Timeout Configuration**
- **Configurable timeouts:** Make timeouts configurable (env vars or settings)
- **Adaptive timeouts:** Increase timeout for large computations

---

## 4. Integration Points

### Current Flow

**Analysis "Run" Flow:**
1. User clicks "Run" in `XYAnalysisWindow`
2. `onRunClicked()` creates `AnalysisWorker` in separate thread
3. `AnalysisWorker::run()` calls `executeCompute()`
4. **Current:** If `PHOENIX_DEMO_MODE=1`, uses `XYSineDemo::compute()` (local)
5. **Otherwise:** Creates `LocalSocketChannel`, connects, calls `computeXYSine()`
6. `AnalysisWorker` emits `finished(success, result, error)` signal
7. `XYAnalysisWindow::onWorkerFinished()` receives result
8. Converts `XYSineResult` to `std::vector<QPointF>`
9. Calls `m_plotView->setData(points)` to update `XYPlotViewGraphs`

### Integration Points Identified

**✅ Already Integrated:**
- **Transport → Analysis:** `AnalysisWorker` uses `LocalSocketChannel`
- **Analysis → UI:** `XYAnalysisWindow` receives results via signal
- **UI → Plot:** `XYPlotViewGraphs` receives data via `setData()`

**📍 Where Synthetic Data is Generated:**
- **Demo Mode:** `XYSineDemo::compute()` in `src/analysis/demo/XYSineDemo.cpp`
- **Location:** Called from `AnalysisWorker::executeCompute()` when `PHOENIX_DEMO_MODE=1`
- **Replacement:** This is already bypassed when demo mode is OFF (uses transport)

**📍 Where Bedrock RPC is Invoked:**
- **Location:** `AnalysisWorker::executeCompute()` line ~219
- **Method:** `client->computeXYSine(m_params, result, progressCallback)`
- **Status:** ✅ Already implemented

**📍 How Results Flow to QtGraphs:**
- **Path:** `AnalysisWorker::finished()` → `XYAnalysisWindow::onWorkerFinished()` → `XYPlotViewGraphs::setData()`
- **Conversion:** `XYSineResult` → `std::vector<QPointF>` → QML LineSeries
- **Status:** ✅ Already implemented

**📍 Where Transport Errors Surface:**
- **Current:** `AnalysisWorker::executeCompute()` emits `finished(false, QVariant(), errorMessage)`
- **UI:** `XYAnalysisWindow::onWorkerFinished()` shows error message (likely via QMessageBox or status bar)
- **Status:** ✅ Basic error handling exists, but could be enhanced

---

## 5. Test Strategy

### Existing Tests

**✅ `test_xysine_transport.cpp`:**
- `testXYSineComputeSuccess()` — Tests successful XY Sine computation
- `testXYSineParameterConversion()` — Tests parameter conversion
- `testXYSineConnectionFailure()` — Tests connection failure handling

### Proposed Test Plan

**Unit Tests:**
1. **Connection Failure:**
   - Test with invalid socket name
   - Test with non-existent socket
   - Test timeout behavior
   - **Status:** ✅ Partially covered by `testXYSineConnectionFailure()`

2. **Message Framing:**
   - Test length-prefixed message encoding/decoding
   - Test message fragmentation handling
   - Test invalid message handling
   - **Status:** ❌ Not tested (internal implementation detail)

3. **Error Handling:**
   - Test protobuf serialization failures
   - Test socket write failures
   - Test socket read failures
   - **Status:** ❌ Not tested

**Integration Tests:**
1. **Bedrock Not Running:**
   - Test connection failure when Bedrock is not running
   - Test error message propagation to UI
   - **Status:** ✅ Covered by `testXYSineConnectionFailure()`

2. **Echo RPC Round-Trip:**
   - Test echo RPC functionality (if Bedrock supports it)
   - Test latency/performance
   - **Status:** ❌ Not tested (Bedrock echo RPC not yet integrated)

3. **XY Sine End-to-End:**
   - Test full flow: UI → AnalysisWorker → LocalSocketChannel → Bedrock → Results → Plot
   - Test with various parameters
   - Test progress updates
   - **Status:** ✅ Covered by `testXYSineComputeSuccess()`

**Functional Tests:**
1. **Cancel During Compute:**
   - Test cancel button during computation
   - Test cancel message sent to Bedrock
   - Test UI updates correctly
   - **Status:** ❌ Not tested (cancel functionality exists but not tested)

2. **Progress Updates:**
   - Test progress callback invoked correctly
   - Test progress updates in UI
   - **Status:** ❌ Not tested

3. **Error Recovery:**
   - Test reconnection after failure
   - Test retry logic (if implemented)
   - **Status:** ❌ Not tested

---

## 6. Open Questions for Mark

### Question 1: Socket Path Standardization

**Question:** What socket path should Phoenix use for LocalSocket connection?

**Current:** Uses `PHOENIX_LOCALSOCKET_NAME` env var (default: `"palantir_bedrock"`)

**Options:**
- **Option A:** Use `$XDG_RUNTIME_DIR/bedrock.sock` on Linux/macOS (fallback to `/tmp/bedrock.sock`)
- **Option B:** Keep current env var approach (flexible for testing)
- **Option C:** Use platform-specific defaults:
  - Linux: `$XDG_RUNTIME_DIR/bedrock.sock` or `/tmp/bedrock.sock`
  - macOS: `$TMPDIR/bedrock.sock` or `/tmp/bedrock.sock`
  - Windows: `\\.\pipe\bedrock` (Qt handles named pipes)

**Recommendation:** **Option C** — Platform-specific defaults with env var override

**Your Decision:** ❓

---

### Question 2: Auto-Start Bedrock vs Connect-Only

**Question:** Should Phoenix auto-start Bedrock, or only connect to an existing Bedrock process?

**Current:** Only connects to existing process (fails if Bedrock not running)

**Options:**
- **Option A:** Auto-start Bedrock if not running
  - Pros: Better UX, no manual step
  - Cons: Process management complexity, error handling

- **Option B:** Only connect to existing Bedrock (current)
  - Pros: Simple, explicit control
  - Cons: Requires manual Bedrock startup

- **Option C:** Configurable (env var or setting)
  - Pros: Flexible, can choose per deployment
  - Cons: More complexity

**Recommendation:** **Option B** (current) — Keep simple for Sprint 4.2, add auto-start later if needed

**Your Decision:** ❓

---

### Question 3: Error Reporting Integration

**Question:** How should transport errors be reported to users?

**Current:** Returns `bool` + `qWarning()` logging, UI shows generic error message

**Options:**
- **Option A:** Structured error codes + user-friendly messages
  - Define error enum (ConnectionFailed, Timeout, ProtocolError, etc.)
  - Map to user-friendly messages
  - Integrate with Phoenix error matrix

- **Option B:** Keep current approach (simple bool + message)
  - Pros: Simple, works
  - Cons: Less structured, harder to handle specific errors

**Recommendation:** **Option A** — Structured errors for better UX

**Your Decision:** ❓

---

### Question 4: Connection Resilience

**Question:** Should LocalSocket implement automatic reconnection?

**Current:** Fails on first connection attempt, no retry

**Options:**
- **Option A:** Implement retry with exponential backoff
  - Pros: More resilient, better UX
  - Cons: More complexity, may mask real issues

- **Option B:** Keep current (fail fast)
  - Pros: Simple, explicit failures
  - Cons: Less resilient

- **Option C:** Configurable retry (env var)
  - Pros: Flexible
  - Cons: More complexity

**Recommendation:** **Option B** (current) — Keep simple for Sprint 4.2, add retry later if needed

**Your Decision:** ❓

---

### Question 5: Logging Strategy

**Question:** How should transport layer log messages?

**Current:** Uses `qDebug()` and `qWarning()` (unstructured)

**Options:**
- **Option A:** Use Qt logging categories (`qt.category.transport`)
  - Pros: Structured, filterable
  - Cons: Requires Qt 5.2+ (already using Qt 6, so OK)

- **Option B:** Keep current approach
  - Pros: Simple, works
  - Cons: Less structured

**Recommendation:** **Option A** — Use Qt logging categories for better debugging

**Your Decision:** ❓

---

### Question 6: Timeout Configuration

**Question:** Should timeouts be configurable?

**Current:** Hardcoded timeouts (3s connection, 5s message, 10s computation)

**Options:**
- **Option A:** Make configurable via env vars or settings
  - Pros: Flexible, can adjust for slow systems
  - Cons: More complexity

- **Option B:** Keep hardcoded (current)
  - Pros: Simple, predictable
  - Cons: May be too short/long for some systems

**Recommendation:** **Option B** (current) — Keep simple, adjust later if needed

**Your Decision:** ❓

---

### Question 7: Windows Named Pipe Path

**Question:** What named pipe path should be used on Windows?

**Current:** Uses Qt's `QLocalSocket` which abstracts this (defaults to `\\.\pipe\<socketName>`)

**Options:**
- **Option A:** Use Qt's default (`\\.\pipe\palantir_bedrock`)
  - Pros: Simple, Qt handles it
  - Cons: May not match Bedrock's actual path

- **Option B:** Explicitly configure Windows path
  - Pros: Explicit control
  - Cons: More platform-specific code

**Recommendation:** **Option A** — Use Qt's abstraction, verify Bedrock uses same path

**Your Decision:** ❓

---

### Question 8: Echo RPC Integration

**Question:** Should Phoenix use Bedrock's Echo RPC for connection testing?

**Current:** No echo RPC usage (only XY Sine computation)

**Options:**
- **Option A:** Add echo RPC call for connection health check
  - Pros: Better connection validation, can measure latency
  - Cons: Requires Bedrock echo RPC to be available

- **Option B:** Keep current (no echo RPC)
  - Pros: Simple, works
  - Cons: Less connection validation

**Recommendation:** **Option A** — Add echo RPC for better connection testing (after Bedrock echo RPC is integrated)

**Your Decision:** ❓

---

## 7. Proposed Implementation Plan

### Phase 1: Documentation & Validation (Sprint 4.2 Day 2)

**Goal:** Document existing implementation and validate it works with Bedrock

**Tasks:**
1. ✅ Document current LocalSocket implementation (this plan)
2. ⏭️ Verify socket path matches Bedrock's expected path
3. ⏭️ Test end-to-end: Phoenix → Bedrock → Results
4. ⏭️ Document error handling flow

**Deliverables:**
- This planning document
- Socket path verification
- End-to-end test results

---

### Phase 2: Enhancements (If Approved)

**Goal:** Improve LocalSocket transport based on Mark's decisions

**Potential Tasks:**
1. **Socket Path Standardization** (if Option C approved)
   - Implement platform-specific defaults
   - Keep env var override
   - Test on Linux/macOS/Windows

2. **Error Reporting** (if Option A approved)
   - Define error enum
   - Map to user-friendly messages
   - Integrate with UI error display

3. **Logging** (if Option A approved)
   - Add Qt logging categories
   - Structure log messages
   - Add log level filtering

4. **Echo RPC Integration** (if Option A approved)
   - Add echo RPC call to `LocalSocketChannel`
   - Use for connection health check
   - Measure latency

**Deliverables:**
- Enhanced LocalSocket implementation
- Updated tests
- Documentation

---

## 8. Summary

### Current State

**✅ What Exists:**
- Complete LocalSocket transport implementation (`LocalSocketChannel`)
- Integration with analysis pipeline (`AnalysisWorker`)
- UI integration (`XYAnalysisWindow` → `XYPlotViewGraphs`)
- Basic error handling and cancel support
- Tests for basic functionality

**⚠️ What Needs Decision:**
- Socket path standardization
- Error reporting strategy
- Logging approach
- Connection resilience
- Echo RPC integration

**❌ What's Missing:**
- Comprehensive error handling tests
- Progress update tests
- Cancel functionality tests
- Echo RPC integration (if desired)

### Recommendations

**For Sprint 4.2 Day 2:**
1. **Validate existing implementation** works with Bedrock
2. **Document socket path** requirements
3. **Test end-to-end** flow
4. **Get Mark's decisions** on open questions

**For Future Sprints:**
1. Implement approved enhancements
2. Add comprehensive tests
3. Integrate echo RPC (if approved)
4. Improve error reporting (if approved)

---

**Next Steps:** Await Mark's decisions on open questions, then proceed with validation and enhancements (Chunk 3B).

