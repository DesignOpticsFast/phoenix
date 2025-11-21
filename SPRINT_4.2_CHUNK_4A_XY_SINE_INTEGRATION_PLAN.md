# Sprint 4.2 Chunk 4A: XY Sine Integration Plan

**Date:** 2025-11-21  
**Status:** PLANNING ONLY — No implementation yet  
**Intent:** Design full Phoenix → Palantir → Bedrock → Phoenix flow for XY Sine feature

---

## Executive Summary

**Key Finding:** XY Sine integration is **already largely implemented** ✅  
- Transport layer (`LocalSocketChannel::computeXYSine()`) ✅  
- Bedrock computation (`PalantirServer::computeXYSine()`) ✅  
- Phoenix UI integration (`AnalysisWorker`, `XYAnalysisWindow`) ✅  
- Result display (`XYPlotViewGraphs::setData()`) ✅  

**Remaining Work:** Validation, error handling refinement, progress throttling, and comprehensive testing.

---

## 1. Existing XY Sine Pieces Mapping

### 1.1 Phoenix-Side Components

**Files:**

#### **Parameter Definition:**
- `src/features/FeatureRegistry.cpp` — Defines XY Sine parameters:
  - `frequency`: Double, default 1.0, range [0.1, 100.0]
  - `amplitude`: Double, default 1.0, range [0.0, 10.0]
  - `phase`: Double, default 0.0, range [-2π, 2π]
  - `samples`: Int, default 1000, range [10, 100000]

#### **Result Structure:**
- `src/analysis/AnalysisResults.hpp` — `XYSineResult` struct:
  ```cpp
  struct XYSineResult {
      std::vector<double> x;
      std::vector<double> y;
  };
  ```

#### **Demo Implementation (Temporary):**
- `src/analysis/demo/XYSineDemo.hpp/cpp` — Local computation (used when `PHOENIX_DEMO_MODE=1`)
  - Matches Bedrock algorithm exactly
  - Uses same parameter parsing logic

#### **Transport Layer:**
- `src/transport/LocalSocketChannel.hpp/cpp` — `computeXYSine()` method ✅
  - Sends `StartJob` with `ComputeSpec`
  - Receives `StartReply`, `Progress`, `ResultMeta`, `DataChunk` messages
  - Handles chunked data assembly
  - Converts to `XYSineResult`

#### **Analysis Worker:**
- `src/analysis/AnalysisWorker.hpp/cpp` — Executes computation:
  - Calls `client->computeXYSine(m_params, result, progressCallback)`
  - Emits `progressChanged()` and `finished()` signals
  - Handles cancel via `m_cancelRequested` flag

#### **UI Components:**
- `src/ui/analysis/XYAnalysisWindow.cpp` — Receives results:
  - Converts `XYSineResult` → `std::vector<QPointF>`
  - Calls `m_plotView->setData(points)`

#### **Plot Display:**
- `src/plot/XYPlotViewGraphs.hpp/cpp` — Displays results:
  - `setData(const std::vector<QPointF>& points)` method
  - Updates Qt Graphs QML LineSeries
  - Handles axis ranges and zoom limits

### 1.2 Bedrock-Side Components

**Files:**

#### **Palantir Protocol:**
- `proto/palantir.proto` — Protocol definitions:
  - `ComputeSpec` — Contains `feature_id` ("xy_sine") and `params` map
  - `StartJob` — Wraps `ComputeSpec` with `job_id`
  - `StartReply` — Status ("OK", "UNIMPLEMENTED", "RESOURCE_EXHAUSTED")
  - `Progress` — Progress percentage and status string
  - `ResultMeta` — Result metadata (status, dtype, shape, bytes_total)
  - `DataChunk` — Chunked data with `chunk_index` and `total_chunks`
  - `Cancel` — Cancellation request

#### **Server Implementation:**
- `src/palantir/PalantirServer.hpp/cpp` — Server logic:
  - `handleStartJob()` — Processes `StartJob`, starts worker thread
  - `processJob()` — Executes computation, sends progress, sends result
  - `computeXYSine()` — Core computation algorithm:
    - Parses parameters: `frequency`, `amplitude`, `phase`, `samples`
    - Validates samples (minimum 2)
    - Computes: `x = t * 2π`, `y = amplitude * sin(2π * frequency * t + phase)`
    - Where `t = i / (samples - 1)` from 0 to 1

#### **Data Transmission:**
- `sendProgress()` — Sends `Progress` messages (0-100%)
- `sendResult()` — Sends `ResultMeta` with shape `[samples]`
- `sendDataChunk()` — Sends `DataChunk` messages (chunked binary data)
  - Format: `x[]` (doubles) followed by `y[]` (doubles)

### 1.3 Protocol Schema (Already Defined)

**Request Schema (`ComputeSpec`):**
```protobuf
message ComputeSpec {
    string feature_id = 1;  // "xy_sine"
    map<string, string> params = 2;  // Key-value pairs:
                                      // "frequency" → "1.0"
                                      // "amplitude" → "1.0"
                                      // "phase" → "0.0"
                                      // "samples" → "1000"
}
```

**Response Schema (Chunked):**
```protobuf
message ResultMeta {
    JobId job_id = 1;
    string status = 2;  // "SUCCEEDED", "FAILED", "CANCELLED"
    string dtype = 3;   // "f64"
    repeated int32 shape = 4;  // [samples]
    int64 compute_elapsed_ms = 5;
    int64 bytes_total = 6;  // samples * sizeof(double) * 2
}

message DataChunk {
    JobId job_id = 1;
    int32 chunk_index = 2;
    int32 total_chunks = 3;
    bytes data = 4;  // Binary: x[] (doubles) + y[] (doubles)
}
```

**Progress Schema:**
```protobuf
message Progress {
    JobId job_id = 1;
    double progress_pct = 2;  // 0.0 to 100.0
    string status = 3;        // "Computing...", "CANCELLED", etc.
}
```

### 1.4 Summary: What Exists vs. What's Needed

**✅ Already Implemented:**
- Parameter definition (`FeatureRegistry`)
- Transport layer (`LocalSocketChannel::computeXYSine()`)
- Bedrock computation (`PalantirServer::computeXYSine()`)
- Result structure (`XYSineResult`)
- UI integration (`AnalysisWorker`, `XYAnalysisWindow`)
- Plot display (`XYPlotViewGraphs::setData()`)
- Progress handling (via `Progress` messages)
- Cancel handling (via `Cancel` messages)

**📍 Needs Validation/Enhancement:**
- Error code mapping (INVALID_ARGUMENT, UNAVAILABLE, etc.)
- Progress throttling (currently every message, should be ~2 Hz)
- Comprehensive error handling (user-friendly messages)
- Integration testing (end-to-end validation)
- Parameter validation (Bedrock-side bounds checking)

---

## 2. XY Sine RPC Schema (Already Defined)

### 2.1 Request Schema

**Current Schema (`ComputeSpec`):**
- ✅ `feature_id`: `"xy_sine"` (string)
- ✅ `params`: Map<string, string> with:
  - `"frequency"` → `"1.0"` (double as string)
  - `"amplitude"` → `"1.0"` (double as string)
  - `"phase"` → `"0.0"` (double as string)
  - `"samples"` → `"1000"` (int as string)
  - `"n_samples"` → `"1000"` (backwards-compatible alias, only if `"samples"` not set)

**Schema Critique:**
- ✅ **Strengths:**
  - Flexible (map-based, easy to extend)
  - Matches Phoenix's `QMap<QString, QVariant>` model
  - Backwards-compatible (`n_samples` alias)
  - String-based (no type coercion issues)

- ⚠️ **Potential Issues:**
  - No explicit validation in proto (handled in code)
  - String conversion overhead (minimal)
  - No default values in proto (handled in code)

**Recommendation:** ✅ **Keep current schema** — It's flexible and matches Phoenix's data model.

### 2.2 Response Schema

**Current Schema (Chunked):**
- ✅ `ResultMeta`: Status, dtype ("f64"), shape `[samples]`, bytes_total
- ✅ `DataChunk`: Binary data (x[] doubles + y[] doubles)

**Data Format:**
- Binary: `x[0..samples-1]` (doubles) followed by `y[0..samples-1]` (doubles)
- Total bytes: `samples * sizeof(double) * 2`
- Chunked: Multiple `DataChunk` messages if data is large

**Schema Critique:**
- ✅ **Strengths:**
  - Efficient (binary, no string conversion)
  - Chunked (handles large datasets)
  - Matches Phoenix's `std::vector<double>` model
  - Shape metadata allows validation

- ⚠️ **Potential Issues:**
  - Chunk ordering must be preserved (handled via `chunk_index`)
  - No explicit point count in `ResultMeta` (derived from `shape[0]`)

**Recommendation:** ✅ **Keep current schema** — Efficient and matches Phoenix's data model.

### 2.3 Point Count Expectations

**Target:** ~10,000 points (configurable via `samples` parameter)

**Current Limits:**
- Phoenix `FeatureRegistry`: `samples` range [10, 100000]
- Bedrock: No explicit limit (validates minimum 2)

**Memory Considerations:**
- 10,000 points: `10,000 * 8 bytes * 2 = 160 KB` (x + y)
- 100,000 points: `100,000 * 8 bytes * 2 = 1.6 MB` (x + y)
- Chunked transmission handles large datasets efficiently

**Recommendation:** ✅ **Current limits are appropriate** — Supports target ~10k points with room for growth.

---

## 3. Bedrock XY Sine Behavior (High Level)

### 3.1 Request Interpretation

**Parameter Parsing:**
1. Extract `feature_id` from `ComputeSpec` → verify `"xy_sine"`
2. Parse `params` map:
   - `"frequency"` → double (default: 1.0)
   - `"amplitude"` → double (default: 1.0)
   - `"phase"` → double (default: 0.0)
   - `"samples"` → int (default: 1000, canonical)
   - `"n_samples"` → int (default: 1000, backwards-compatible, only if `"samples"` not set)
3. Validate `samples` (minimum 2, clamp if < 2)

**Error Handling:**
- Invalid `feature_id` → `StartReply.status = "UNIMPLEMENTED"`
- Invalid parameter values (e.g., non-numeric) → Use defaults (silent fallback)
- `samples < 2` → Clamp to 2 (silent fallback)
- **Future:** Explicit `INVALID_ARGUMENT` for out-of-range values

### 3.2 Sample Generation Algorithm

**Current Algorithm (Bedrock `PalantirServer::computeXYSine()`):**
```cpp
for (int i = 0; i < samples; ++i) {
    double t = static_cast<double>(i) / (samples - 1.0);  // 0 to 1
    double x = t * 2.0 * M_PI;  // Scale to 0..2π domain
    double y = amplitude * std::sin(2.0 * M_PI * frequency * t + phase);
    
    xValues.push_back(x);
    yValues.push_back(y);
}
```

**Mathematical Formula:**
- `t = i / (samples - 1)` where `i ∈ [0, samples-1]` → `t ∈ [0, 1]`
- `x = t * 2π` → `x ∈ [0, 2π]`
- `y = amplitude * sin(2π * frequency * t + phase)`

**Behavior:**
- ✅ Generates exactly `samples` points
- ✅ X domain: `[0, 2π]` (one full cycle)
- ✅ Y range: `[-amplitude, amplitude]` (for frequency=1, phase=0)
- ✅ Matches Phoenix `XYSineDemo` algorithm exactly

**Recommendation:** ✅ **Keep current algorithm** — Matches Phoenix expectations.

### 3.3 Progress Emission

**Current Behavior:**
- Bedrock sends `Progress` messages during computation
- Progress percentage: `0.0` to `100.0`
- Status string: `"Computing..."`, `"CANCELLED"`, etc.

**Current Implementation:**
- `PalantirServer::sendProgress(jobId, progress, status)`
- Called during `processJob()` loop
- **Issue:** May send progress too frequently (every iteration)

**Proposed Behavior:**
- **Throttle to ~2 Hz** (every 500ms or every N samples)
- Send progress:
  - `0%` — Job started
  - `50%` — Mid-computation (if samples > threshold)
  - `100%` — Computation complete

**Recommendation:** ⚠️ **Add progress throttling** — Limit to ~2 Hz for UI responsiveness.

### 3.4 Cancel Handling

**Current Behavior:**
- Phoenix sends `Cancel` message with `job_id`
- Bedrock sets `jobCancelled_[jobId] = true`
- Worker thread checks `jobCancelled_` flag periodically
- If cancelled: Send `Progress` with `status = "CANCELLED"`, send `ResultMeta` with `status = "CANCELLED"`

**Polling Interval:**
- **Current:** Checked every loop iteration (may be too frequent)
- **Proposed:** Check every N samples (e.g., every 100 samples) or every 100ms

**Stop and Reply:**
1. Set `jobCancelled_[jobId] = true`
2. Worker thread detects cancellation
3. Stop computation loop
4. Send `Progress` with `status = "CANCELLED"`
5. Send `ResultMeta` with `status = "CANCELLED"` (no data chunks)
6. Clean up job state

**Recommendation:** ✅ **Current cancel handling is adequate** — May benefit from polling interval optimization.

### 3.5 INVALID_ARGUMENT Handling

**Current Behavior:**
- Invalid parameter values → Silent fallback to defaults
- `samples < 2` → Clamp to 2

**Proposed Behavior:**
- **Explicit validation:**
  - `samples` out of range [10, 100000] → `StartReply.status = "INVALID_ARGUMENT"`
  - `frequency` out of range [0.1, 100.0] → `StartReply.status = "INVALID_ARGUMENT"`
  - `amplitude` out of range [0.0, 10.0] → `StartReply.status = "INVALID_ARGUMENT"`
  - `phase` out of range [-2π, 2π] → `StartReply.status = "INVALID_ARGUMENT"`
- **Error message:** `StartReply.error_message = "Parameter 'samples' out of range: 5 (valid: 10-100000)"`

**Recommendation:** ⚠️ **Add explicit validation** — Return `INVALID_ARGUMENT` for out-of-range parameters.

---

## 4. Phoenix-Side Integration Flow

### 4.1 Current Flow (Step-by-Step)

**1. User Clicks Run in XY AnalysisWindow**
- `XYAnalysisWindow::onRunClicked()` → `AnalysisWorker::run()`

**2. AnalysisWorker Builds XY Sine Request**
- `AnalysisWorker::executeCompute()`:
  - Reads `m_params` (`QMap<QString, QVariant>`)
  - Parameters already set via `setParameters("xy_sine", params)`

**3. AnalysisWorker Calls TransportClient**
- `AnalysisWorker::executeCompute()`:
  - Creates `LocalSocketChannel` (or uses existing)
  - Calls `client->connect()`
  - Calls `client->computeXYSine(m_params, result, progressCallback)`

**4. LocalSocketChannel Sends Request**
- `LocalSocketChannel::computeXYSine()`:
  - Converts `QMap<QString, QVariant>` → `ComputeSpec` (string map)
  - Creates `StartJob` with `job_id` (UUID)
  - Sends `StartJob` via `sendMessage()`
  - Waits for `StartReply`

**5. Bedrock Computes, Emits Progress, Returns Response**
- `PalantirServer::handleStartJob()`:
  - Validates `feature_id` ("xy_sine")
  - Starts worker thread → `processJob()`
  - Sends `StartReply` with `status = "OK"`
- `PalantirServer::processJob()`:
  - Calls `computeXYSine()` → generates x[], y[]
  - Sends `Progress` messages (throttled)
  - Sends `ResultMeta` with shape `[samples]`
  - Sends `DataChunk` messages (chunked binary data)

**6. LocalSocketChannel Receives Response**
- `LocalSocketChannel::computeXYSine()`:
  - Receives `StartReply` → verifies `status = "OK"`
  - Receives `Progress` → calls `progressCallback(percent, status)`
  - Receives `ResultMeta` → verifies `status = "SUCCEEDED"`
  - Receives `DataChunk` messages → assembles binary data
  - Parses binary: `x[]` (doubles) + `y[]` (doubles)
  - Converts to `XYSineResult` (`std::vector<double> x, y`)

**7. AnalysisWorker Converts Response**
- `AnalysisWorker::executeCompute()`:
  - Receives `XYSineResult` from `computeXYSine()`
  - Emits `finished(true, QVariant::fromValue(result), "")`

**8. XYAnalysisWindow Receives ResultData**
- `XYAnalysisWindow::onWorkerFinished()`:
  - Receives `QVariant` result
  - Converts to `XYSineResult`: `result.value<XYSineResult>()`

**9. XYPlotViewGraphs::setData() Gets Called**
- `XYAnalysisWindow::onWorkerFinished()`:
  - Converts `XYSineResult` → `std::vector<QPointF>`:
    ```cpp
    std::vector<QPointF> points;
    for (size_t i = 0; i < xyResult.x.size(); ++i) {
        points.emplace_back(xyResult.x[i], xyResult.y[i]);
    }
    ```
  - Calls `m_plotView->setData(points)`

**10. Plot Updates**
- `XYPlotViewGraphs::setData()`:
  - Converts `std::vector<QPointF>` → `QList<QPointF>`
  - Calls QML `mainSeries.replace(pointList)`
  - Updates axis ranges

### 4.2 Parameter Mapping

**Phoenix → Bedrock:**
- `QMap<QString, QVariant>` → `ComputeSpec.params` (map<string, string>)
- Conversion: `QVariant::toString()` for each parameter
- Keys: `"frequency"`, `"amplitude"`, `"phase"`, `"samples"`

**Bedrock → Phoenix:**
- `ComputeSpec.params` (map<string, string>) → `QMap<QString, QVariant>`
- Conversion: String parsing in `PalantirServer::computeXYSine()`
- **Note:** Phoenix doesn't receive params back (only result data)

### 4.3 Result Mapping

**Bedrock → Phoenix:**
- Binary data (`x[]` doubles + `y[]` doubles) → `XYSineResult`
- `XYSineResult` → `std::vector<QPointF>` (in `XYAnalysisWindow`)
- `std::vector<QPointF>` → `QList<QPointF>` (in `XYPlotViewGraphs`)
- `QList<QPointF>` → QML `LineSeries.replace()` (Qt Graphs)

**Data Flow:**
```
Bedrock: x[], y[] (doubles)
  ↓ (binary serialization)
Palantir: DataChunk (bytes)
  ↓ (deserialization)
Phoenix: XYSineResult { std::vector<double> x, y }
  ↓ (conversion)
Phoenix: std::vector<QPointF>
  ↓ (conversion)
Phoenix: QList<QPointF>
  ↓ (QML call)
Qt Graphs: LineSeries.replace(QList<QPointF>)
```

---

## 5. Progress + Cancel Integration

### 5.1 Progress Representation

**Bedrock Progress Messages:**
- `Progress.progress_pct`: `0.0` to `100.0` (double)
- `Progress.status`: `"Computing..."`, `"CANCELLED"`, etc. (string)

**Phoenix Progress Handling:**
- `LocalSocketChannel::computeXYSine()` receives `Progress` messages
- Calls `progressCallback(percent, status)` (lambda in `AnalysisWorker`)
- `AnalysisWorker` emits `progressChanged(AnalysisProgress(percent, status))`
- `XYAnalysisWindow` receives `progressChanged()` signal → updates UI

**Current Issue:** ⚠️ **No throttling** — Progress messages may arrive too frequently

### 5.2 Progress Throttling

**Current Behavior:**
- Bedrock sends `Progress` every loop iteration (potentially 1000+ messages for 1000 samples)
- Phoenix processes every message immediately

**Proposed Behavior:**
- **Throttle to ~2 Hz** (every 500ms or every N samples)
- **Bedrock-side throttling:**
  - Track last progress send time
  - Only send if `(current_time - last_send_time) >= 500ms`
  - Or: Send every `samples / 2` iterations (for 1000 samples, send every 500 iterations)

**Phoenix-side throttling (alternative):**
- Track last progress update time in `AnalysisWorker`
- Only emit `progressChanged()` if `(current_time - last_emit_time) >= 500ms`
- **Recommendation:** Prefer Bedrock-side throttling (reduces network traffic)

### 5.3 Cancel Integration

**Cancel Trigger:**
- User clicks Cancel button → `XYAnalysisWindow::onCancelClicked()`
- Emits signal → `AnalysisWorker::requestCancel()`
- Sets `m_cancelRequested.store(true)`

**Cancel Propagation:**
1. `AnalysisWorker::requestCancel()` → sets `m_cancelRequested = true`
2. `AnalysisWorker::executeCompute()` checks `m_cancelRequested`:
   - If true → calls `client->cancelJob(m_currentJobId)`
   - Disconnects client
   - Emits `cancelled()` signal
3. `LocalSocketChannel::cancelJob()`:
   - Creates `Cancel` message with `job_id`
   - Sends via `sendMessage()`
4. Bedrock receives `Cancel`:
   - `PalantirServer::handleCancel()` → sets `jobCancelled_[jobId] = true`
   - Worker thread checks flag → stops computation
   - Sends `Progress` with `status = "CANCELLED"`
   - Sends `ResultMeta` with `status = "CANCELLED"`

**UI State Unwind:**
- `XYAnalysisWindow` receives `cancelled()` signal
- Cleans up worker (`cleanupWorker()`)
- Resets UI state (hides progress, enables Run button)

**Recommendation:** ✅ **Current cancel handling is adequate** — May benefit from polling interval optimization in Bedrock.

---

## 6. Error Handling for XY Sine

### 6.1 Error Code Mapping

**Current Error Codes (`TransportError` enum):**
- `NoError` — Success
- `ConnectionFailed` — Cannot connect to server
- `ConnectionTimeout` — Connection timed out
- `ProtocolError` — Protocol/message parsing error
- `ServerError` — Server returned error status
- `NetworkError` — Socket/network I/O error

**Palantir Protocol Error Statuses:**
- `StartReply.status = "OK"` → Success
- `StartReply.status = "UNIMPLEMENTED"` → Feature not supported
- `StartReply.status = "RESOURCE_EXHAUSTED"` → Server at capacity
- `StartReply.status = "INVALID_ARGUMENT"` → Invalid parameters (proposed)
- `ResultMeta.status = "SUCCEEDED"` → Success
- `ResultMeta.status = "FAILED"` → Computation failed
- `ResultMeta.status = "CANCELLED"` → Cancelled

**Proposed Error Mapping:**

| Bedrock Error | TransportError | User Message |
|---------------|----------------|--------------|
| `StartReply.status = "OK"` | `NoError` | Success |
| `StartReply.status = "UNIMPLEMENTED"` | `ServerError` | "Feature 'xy_sine' is not supported by Bedrock server." |
| `StartReply.status = "RESOURCE_EXHAUSTED"` | `ServerError` | "Bedrock server is at capacity. Please try again later." |
| `StartReply.status = "INVALID_ARGUMENT"` | `ServerError` | "Invalid parameters: {error_message}" |
| `ResultMeta.status = "FAILED"` | `ServerError` | "Computation failed: {error_message}" |
| `ResultMeta.status = "CANCELLED"` | `NoError` (special case) | "Computation cancelled." |
| Connection timeout | `ConnectionTimeout` | "Connection to Bedrock server timed out." |
| Connection refused | `ConnectionFailed` | "Failed to connect to Bedrock server. Please ensure Bedrock is running." |
| Protocol parse error | `ProtocolError` | "Protocol error communicating with Bedrock server." |
| Network I/O error | `NetworkError` | "Network I/O error occurred." |

### 6.2 User-Facing Error Messages

**Current Behavior:**
- `AnalysisWorker` emits `finished(false, QVariant(), error)` with generic message
- `XYAnalysisWindow` shows `QMessageBox::warning()` with error text

**Proposed Behavior:**
- **Toast/Dialog:** Use `QMessageBox::warning()` for critical errors
- **Inline Error:** Show error in status bar or progress area
- **Error Categories:**
  - **UNAVAILABLE** (Bedrock down): "Bedrock server is not available. Please ensure Bedrock is running."
  - **INVALID_ARGUMENT** (bad params): "Invalid parameters: {parameter} out of range."
  - **PERMISSION_DENIED** (license): "License required: Feature 'xy_sine' requires a valid license."
  - **CANCELLED**: "Computation cancelled." (informational, not error)
  - **INTERNAL** (server error): "Computation failed. Please check server logs."

**Recommendation:** ✅ **Current error handling is adequate** — Enhance with specific error messages per category.

---

## 7. Test Strategy

### 7.1 Unit Tests (Phoenix)

**Parameter → Request Mapping:**
- **File:** `tests/test_xysine_transport.cpp` (already exists)
- **Test:** `testXYSineParameterConversion()`
  - Verify `QMap<QString, QVariant>` → `ComputeSpec.params` conversion
  - Test all parameter types (double, int)
  - Test default values

**Result → UI Mapping:**
- **File:** `tests/test_xyplot_autoscale.cpp` (already exists)
- **Test:** Verify `XYSineResult` → `std::vector<QPointF>` conversion
- **Test:** Verify `setData()` updates plot correctly

### 7.2 Unit Tests (Bedrock)

**Request → Sample Generation:**
- **File:** `tests/test_xysine_compute.cpp` (already exists)
- **Tests:**
  - `XYSineCompute.BasicComputation` — Basic sine wave
  - `XYSineCompute.WithPhase` — Phase offset
  - `XYSineCompute.WithFrequency` — Frequency scaling
  - `XYSineCompute.ParameterDefaults` — Default values
  - `XYSineCompute.SampleClamping` — Minimum samples validation

**Protocol Integration:**
- **File:** `tests/test_palantir_xysine.cpp` (already exists)
- **Tests:**
  - `PalantirXYSine.DirectComputation` — End-to-end via `PalantirServer`
  - `PalantirXYSine.ParameterParsing` — Parameter extraction
  - `PalantirXYSine.SupportedFeature` — Feature registration

### 7.3 Integration Tests

**Happy Path:**
- **File:** `tests/test_xysine_transport.cpp` (already exists)
- **Test:** `testXYSineComputeSuccess()`
  - Connect to Bedrock
  - Send request with typical parameters (`frequency=1.0`, `amplitude=1.0`, `phase=0.0`, `samples=100`)
  - Verify response: `XYSineResult` with correct size
  - Verify data: X domain `[0, 2π]`, Y matches sine wave

**INVALID_ARGUMENT Case:**
- **Test:** `testXYSineInvalidParameters()` (new)
  - Send request with `samples=5` (below minimum 10)
  - Verify `StartReply.status = "INVALID_ARGUMENT"`
  - Verify error message contains parameter name

**UNAVAILABLE Case:**
- **Test:** `testXYSineConnectionFailure()` (already exists)
  - Connect to invalid socket name
  - Verify `TransportError::ConnectionFailed` or `ConnectionTimeout`
  - Verify user-friendly error message

**CANCELLED Case:**
- **Test:** `testXYSineCancel()` (new)
  - Start computation with large `samples` (e.g., 10000)
  - Send `Cancel` message mid-computation
  - Verify `ResultMeta.status = "CANCELLED"`
  - Verify no data chunks received

### 7.4 Test Matrix

| Test | Phoenix | Bedrock | Transport | Status |
|------|---------|---------|-----------|--------|
| Parameter conversion | ✅ | N/A | N/A | Exists |
| Sample generation | N/A | ✅ | N/A | Exists |
| Happy path | ✅ | ✅ | ✅ | Exists |
| Connection failure | ✅ | N/A | ✅ | Exists |
| Invalid parameters | ❌ | ❌ | ❌ | **Needs implementation** |
| Cancel mid-compute | ❌ | ❌ | ❌ | **Needs implementation** |
| Progress throttling | ❌ | ❌ | ❌ | **Needs validation** |

**Recommendation:** ✅ **Add tests for invalid parameters and cancel** — Other tests already exist.

---

## 8. Open Questions for Mark

### Question 1: Parameter Validation

**Question:** Should Bedrock validate parameter ranges and return `INVALID_ARGUMENT`, or silently clamp to valid ranges?

**Current Behavior:** Silent clamping (e.g., `samples < 2` → clamp to 2)

**Options:**
- **Option A:** Explicit validation → Return `INVALID_ARGUMENT` for out-of-range values
- **Option B:** Silent clamping → Use defaults/clamp (current behavior)

**Recommendation:** **Option A** — Explicit validation provides better user feedback.

**Your Decision:** ❓

---

### Question 2: Max Sample Count

**Question:** What is the maximum `samples` value we should support?

**Current Limits:**
- Phoenix `FeatureRegistry`: `samples` max = 100,000
- Bedrock: No explicit limit

**Considerations:**
- Memory: 100,000 points = 1.6 MB (x + y)
- Computation time: Linear with sample count
- Network: Chunked transmission handles large datasets

**Recommendation:** **Keep 100,000 max** — Reasonable for most use cases, chunked transmission handles it.

**Your Decision:** ❓

---

### Question 3: Multiple Curves (Harmonics)

**Question:** Should v1 support multiple curves (e.g., harmonics, multiple sine waves)?

**Current Behavior:** Single curve only (`x[]`, `y[]`)

**Options:**
- **Option A:** Single curve only (v1)
- **Option B:** Multiple curves (e.g., `repeated Curve curves`)

**Recommendation:** **Option A** — Single curve for v1, add multiple curves in future sprint.

**Your Decision:** ❓

---

### Question 4: Progress Throttling Frequency

**Question:** What frequency should progress updates be throttled to?

**Current Behavior:** Every loop iteration (potentially 1000+ messages)

**Options:**
- **Option A:** ~2 Hz (every 500ms)
- **Option B:** ~5 Hz (every 200ms)
- **Option C:** Every N samples (e.g., every 100 samples)

**Recommendation:** **Option A** — ~2 Hz provides smooth UI updates without overwhelming the network.

**Your Decision:** ❓

---

### Question 5: Cancel Polling Interval

**Question:** How frequently should Bedrock check for cancellation?

**Current Behavior:** Every loop iteration

**Options:**
- **Option A:** Every N samples (e.g., every 100 samples)
- **Option B:** Every N milliseconds (e.g., every 100ms)
- **Option C:** Every loop iteration (current)

**Recommendation:** **Option A** — Every 100 samples balances responsiveness with performance.

**Your Decision:** ❓

---

### Question 6: Error Message Display

**Question:** How should errors be displayed to the user?

**Current Behavior:** `QMessageBox::warning()` dialog

**Options:**
- **Option A:** Toast notification (non-blocking)
- **Option B:** Dialog (blocking, current)
- **Option C:** Inline error in status bar/progress area

**Recommendation:** **Option B** — Dialog for critical errors, inline for warnings.

**Your Decision:** ❓

---

### Question 7: Memory/Latency Constraints

**Question:** Are there any constraints on memory usage or latency beyond what's in the sprint plan?

**Current Behavior:**
- Memory: ~1.6 MB for 100,000 points
- Latency: Depends on sample count (linear)

**Considerations:**
- Large datasets may require chunked transmission (already implemented)
- UI responsiveness may degrade with very large datasets

**Recommendation:** **No additional constraints** — Current implementation handles reasonable datasets efficiently.

**Your Decision:** ❓

---

## 9. Summary

### Current State

**✅ Already Implemented:**
- Parameter definition (`FeatureRegistry`)
- Transport layer (`LocalSocketChannel::computeXYSine()`)
- Bedrock computation (`PalantirServer::computeXYSine()`)
- Result structure (`XYSineResult`)
- UI integration (`AnalysisWorker`, `XYAnalysisWindow`)
- Plot display (`XYPlotViewGraphs::setData()`)
- Progress handling (via `Progress` messages)
- Cancel handling (via `Cancel` messages)

**📍 Needs Enhancement:**
- Parameter validation (explicit `INVALID_ARGUMENT` errors)
- Progress throttling (~2 Hz)
- Cancel polling interval optimization
- Comprehensive error messages
- Integration tests (invalid parameters, cancel)

### Proposed Enhancements

1. **Bedrock Parameter Validation:**
   - Validate `samples` range [10, 100000] → `INVALID_ARGUMENT`
   - Validate `frequency` range [0.1, 100.0] → `INVALID_ARGUMENT`
   - Validate `amplitude` range [0.0, 10.0] → `INVALID_ARGUMENT`
   - Validate `phase` range [-2π, 2π] → `INVALID_ARGUMENT`

2. **Progress Throttling:**
   - Bedrock: Throttle `Progress` messages to ~2 Hz (every 500ms)
   - Or: Send every `samples / 2` iterations

3. **Cancel Polling:**
   - Bedrock: Check `jobCancelled_` flag every 100 samples (instead of every iteration)

4. **Error Messages:**
   - Map `StartReply.status` → `TransportError` codes
   - Provide user-friendly error messages per error category

5. **Tests:**
   - Add `testXYSineInvalidParameters()` for `INVALID_ARGUMENT` case
   - Add `testXYSineCancel()` for cancel mid-computation

### Implementation Priority

1. **High Priority:**
   - Parameter validation (explicit errors)
   - Progress throttling (UI responsiveness)
   - Error message mapping (user experience)

2. **Medium Priority:**
   - Cancel polling optimization (performance)
   - Integration tests (validation)

3. **Low Priority:**
   - Multiple curves support (future sprint)

---

**Next Steps:** Await Mark's decisions on open questions, then proceed with implementation (Chunk 4B).

