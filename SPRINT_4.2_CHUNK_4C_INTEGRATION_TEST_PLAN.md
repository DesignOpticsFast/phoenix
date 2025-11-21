# Sprint 4.2 Chunk 4C: End-to-End XY Sine Integration Test Plan

**Date:** 2025-11-21  
**Status:** PLANNING ONLY — No test execution yet  
**Intent:** Plan comprehensive end-to-end testing of Phoenix ↔ Bedrock XY Sine integration

---

## ⚠️ UNDERLORD WORKFLOW DIRECTIVE (CRITICAL)

**From this point forward, all test execution MUST operate on the canonical repo working copy:**
- `/home/ec2-user/workspace/phoenix/`
- `/home/ec2-user/workspace/bedrock/`

**All test binaries, logs, and results MUST be generated from the live repo directories above.**

---

## 1. Test Environment Confirmation

### 1.1 Bedrock Server

**Binary Path:**
- **Build Location:** `bedrock/build-*/src/palantir/bedrock_server` (or `bedrock/build-*/bedrock_server`)
- **Source:** `bedrock/src/palantir/bedrock_server.cpp`
- **CMake Target:** `bedrock_server` (if configured)

**Launch Command:**
```bash
# Default socket name (palantir_bedrock)
./bedrock_server

# Custom socket name
./bedrock_server --socket custom_socket_name

# With logging to file
./bedrock_server 2>&1 | tee bedrock.log
```

**Default Socket Name:**
- `"palantir_bedrock"` (hardcoded default in `bedrock_server.cpp` line 17)
- Can be overridden via `--socket` command-line option

**Environment Variables:**
- None required (socket name via CLI flag)

**Expected Output (on startup):**
```
Bedrock server running on socket: palantir_bedrock
Max concurrency: <N>
Supported features: xy_sine, ...
```

### 1.2 Phoenix Application

**Binary Path:**
- **Build Location:** `phoenix/build-*/phoenix_app` (or similar)
- **Source:** `phoenix/src/main.cpp`
- **CMake Target:** `phoenix_app` (or similar)

**Launch Command:**
```bash
# Default socket (palantir_bedrock)
./phoenix_app

# Custom socket via environment variable
PHOENIX_LOCALSOCKET_NAME=custom_socket_name ./phoenix_app

# With logging to file
QT_LOGGING_RULES="phoenix.transport.debug=true" ./phoenix_app 2>&1 | tee phoenix.log
```

**Default Socket Name:**
- `"palantir_bedrock"` (default in `LocalSocketChannel.cpp` line 20)
- Can be overridden via `PHOENIX_LOCALSOCKET_NAME` environment variable

**Environment Variables:**
- `PHOENIX_LOCALSOCKET_NAME` — Socket name (default: `"palantir_bedrock"`)
- `PHOENIX_DEMO_MODE` — **MUST be unset or "0"** for Bedrock integration testing
- `QT_LOGGING_RULES` — Logging configuration (e.g., `"phoenix.transport.debug=true"`)

**Critical Prerequisite:**
- ✅ **`PHOENIX_DEMO_MODE` must NOT be set to "1"** — Demo mode bypasses Bedrock transport

### 1.3 Socket Name Alignment

**Default Configuration:**
- Bedrock: `"palantir_bedrock"` (CLI default)
- Phoenix: `"palantir_bedrock"` (env var default)
- ✅ **Aligned by default** — No configuration needed for basic testing

**Custom Configuration:**
- Both must use same socket name:
  ```bash
  # Terminal 1: Bedrock
  ./bedrock_server --socket test_socket
  
  # Terminal 2: Phoenix
  PHOENIX_LOCALSOCKET_NAME=test_socket ./phoenix_app
  ```

---

## 2. Step-by-Step Test Plans

### Test C1: Happy Path (10k Samples)

**Objective:** Verify successful end-to-end XY Sine computation with typical parameters.

**Prerequisites:**
- Bedrock server running on `palantir_bedrock`
- Phoenix built with `PHX_WITH_TRANSPORT_DEPS=ON`
- `PHOENIX_DEMO_MODE` not set (or set to "0")

**Steps:**
1. **Start Bedrock Server:**
   ```bash
   cd /home/ec2-user/workspace/bedrock/build-default
   ./src/palantir/bedrock_server 2>&1 | tee bedrock_happy.log &
   BEDROCK_PID=$!
   ```
   - Verify: "Bedrock server running on socket: palantir_bedrock"

2. **Start Phoenix:**
   ```bash
   cd /home/ec2-user/workspace/phoenix/build-default
   QT_LOGGING_RULES="phoenix.transport.debug=true" ./phoenix_app 2>&1 | tee phoenix_happy.log &
   PHOENIX_PID=$!
   ```

3. **In Phoenix UI:**
   - Open XY Analysis Window (File → New → XY Analysis, or toolbar)
   - Set parameters:
     - Frequency: `1.0`
     - Amplitude: `1.0`
     - Phase: `0.0`
     - Samples: `10000`
   - Click "Run"

4. **Observe:**
   - Progress bar updates (should be ~2 Hz)
   - Plot updates with sine wave
   - No error dialogs
   - Status bar shows success (or no error)

5. **Verify Plot:**
   - X domain: `[0, 2π]` (approximately 0 to 6.28)
   - Y range: `[-1, 1]` (for amplitude=1.0, frequency=1.0)
   - Smooth sine curve
   - Exactly 10,000 points

6. **Cleanup:**
   ```bash
   kill $PHOENIX_PID
   kill $BEDROCK_PID
   ```

**Expected Outcomes:**
- ✅ `TransportError::NoError` throughout
- ✅ Progress updates at ~2 Hz (UI throttled from ≤5 Hz Bedrock)
- ✅ Plot renders correctly with 10k points
- ✅ No crashes or exceptions
- ✅ Bedrock logs show: "Started job: <uuid>", progress messages, "Complete"
- ✅ Phoenix logs show: `[phoenix.transport] Connected to palantir_bedrock`, progress callbacks

**Expected Logs:**

**Bedrock (`bedrock_happy.log`):**
```
Bedrock server running on socket: palantir_bedrock
Client connected
Started job: <uuid>
Client disconnected
```

**Phoenix (`phoenix_happy.log`):**
```
[phoenix.transport] Connected to palantir_bedrock
[phoenix.transport] computeXYSine: Successfully computed 10000 samples
```

**Timing:**
- Connection: < 100ms
- Computation: < 1s for 10k samples
- Total: < 2s end-to-end

---

### Test C2: Invalid Parameters

**Objective:** Verify `INVALID_ARGUMENT` error handling and inline error display.

**Prerequisites:**
- Same as C1

**Test Cases:**

#### C2.1: Samples Too Small (`samples=5`)

**Steps:**
1. Start Bedrock (same as C1)
2. Start Phoenix (same as C1)
3. In Phoenix UI:
   - Set Samples: `5`
   - Click "Run"

**Expected Outcomes:**
- ✅ Bedrock returns `StartReply.status = "INVALID_ARGUMENT"`
- ✅ Error message: `"Parameter 'samples' out of range: 5 (valid: 10-100000)"`
- ✅ Phoenix maps to `TransportError::InvalidArgument`
- ✅ **UI shows inline status bar message** (NOT modal dialog)
- ✅ Status bar message: `"Parameter 'samples' out of range: 5 (valid: 10-100000)"`
- ✅ No plot update (computation failed)
- ✅ Run button re-enabled

**Expected Logs:**

**Bedrock:**
```
Started job: <uuid>
(No computation - rejected at validation)
```

**Phoenix:**
```
[phoenix.transport] computeXYSine: StartJob failed: INVALID_ARGUMENT - Parameter 'samples' out of range: 5 (valid: 10-100000)
```

#### C2.2: Frequency Out of Range (`frequency=0.05`)

**Steps:**
1. Set Frequency: `0.05` (below minimum 0.1)
2. Set Samples: `1000` (valid)
3. Click "Run"

**Expected Outcomes:**
- ✅ `TransportError::InvalidArgument`
- ✅ Error message: `"Parameter 'frequency' out of range: 0.05 (valid: 0.1-100.0)"`
- ✅ Inline status bar message (NOT dialog)

#### C2.3: Amplitude Out of Range (`amplitude=-1.0`)

**Steps:**
1. Set Amplitude: `-1.0` (below minimum 0.0)
2. Set Samples: `1000` (valid)
3. Click "Run"

**Expected Outcomes:**
- ✅ `TransportError::InvalidArgument`
- ✅ Error message: `"Parameter 'amplitude' out of range: -1.0 (valid: 0.0-10.0)"`
- ✅ Inline status bar message (NOT dialog)

#### C2.4: Samples Too Large (`samples=200000`)

**Steps:**
1. Set Samples: `200000` (above maximum 100000)
2. Click "Run"

**Expected Outcomes:**
- ✅ `TransportError::InvalidArgument`
- ✅ Error message: `"Parameter 'samples' out of range: 200000 (valid: 10-100000)"`
- ✅ Inline status bar message (NOT dialog)

**Common Verification:**
- ✅ No modal dialogs (`QMessageBox`) for `INVALID_ARGUMENT`
- ✅ Status bar shows error for 10 seconds
- ✅ UI remains responsive
- ✅ Can immediately retry with corrected parameters

---

### Test C3: Bedrock Unavailable

**Objective:** Verify `UNAVAILABLE` error handling and user-friendly dialog.

**Prerequisites:**
- Phoenix built with transport deps
- Bedrock **NOT running**

**Steps:**
1. **Ensure Bedrock is NOT running:**
   ```bash
   pkill -f bedrock_server
   # Verify: ps aux | grep bedrock_server (should show nothing)
   ```

2. **Start Phoenix:**
   ```bash
   cd /home/ec2-user/workspace/phoenix/build-default
   QT_LOGGING_RULES="phoenix.transport.debug=true" ./phoenix_app 2>&1 | tee phoenix_unavailable.log &
   PHOENIX_PID=$!
   ```

3. **In Phoenix UI:**
   - Open XY Analysis Window
   - Set valid parameters (Frequency: 1.0, Amplitude: 1.0, Phase: 0.0, Samples: 1000)
   - Click "Run"

4. **Observe:**
   - Connection attempt fails
   - Error dialog appears

**Expected Outcomes:**
- ✅ `TransportError::ConnectionFailed` or `ConnectionTimeout`
- ✅ **UI shows toast dialog** (`QMessageBox::information`)
- ✅ Dialog title: `"Server Unavailable"`
- ✅ Dialog message: `"Bedrock server is not available.\n\nPlease ensure Bedrock is running and try again."`
- ✅ No plot update
- ✅ Run button re-enabled

**Expected Logs:**

**Phoenix (`phoenix_unavailable.log`):**
```
[phoenix.transport] Connection timeout to palantir_bedrock - Connection timeout
[phoenix.transport] computeXYSine: Not connected
```

**Timing:**
- Connection timeout: ~3-5 seconds (as configured in `LocalSocketChannel::connect()`)

---

### Test C4: Cancel Mid-Run

**Objective:** Verify cancellation works correctly on large jobs.

**Prerequisites:**
- Bedrock running
- Phoenix running

**Steps:**
1. **Start Bedrock and Phoenix** (same as C1)

2. **In Phoenix UI:**
   - Set Samples: `100000` (large job for cancel testing)
   - Click "Run"
   - **Immediately click "Cancel"** (< 200ms after Run)

3. **Observe:**
   - Cancel button becomes visible
   - Computation stops quickly
   - Status updates

**Expected Outcomes:**
- ✅ Cancel signal sent to Bedrock (`Cancel` message)
- ✅ Bedrock detects cancellation (every 100 samples)
- ✅ Bedrock sends `ResultMeta.status = "CANCELLED"`
- ✅ Phoenix maps to `TransportError::Cancelled`
- ✅ **UI shows inline "Cancelled" message** in status bar (NOT dialog)
- ✅ Status bar message: `"Computation cancelled."`
- ✅ **Plot does NOT update** (no data received)
- ✅ Run button re-enabled
- ✅ Can immediately start new computation

**Expected Logs:**

**Bedrock:**
```
Started job: <uuid>
Cancelled job: <uuid>
```

**Phoenix:**
```
[phoenix.transport] cancelJob: Sent cancel for job <uuid>
[phoenix.transport] computeXYSine: Computation CANCELLED
```

**Timing:**
- Cancel response: < 200ms (as per decision #5: polling every 100 samples)
- UI update: < 100ms after cancel signal

**Verification:**
- ✅ No data chunks received after cancel
- ✅ Plot remains empty or unchanged
- ✅ No crashes or exceptions

---

### Test C5: Progress Throttling Sanity

**Objective:** Verify progress throttling works correctly (Bedrock ≤5 Hz, Phoenix UI ~2 Hz).

**Prerequisites:**
- Bedrock running
- Phoenix running
- Logging enabled

**Steps:**
1. **Start Bedrock with verbose logging:**
   ```bash
   ./bedrock_server 2>&1 | tee bedrock_progress.log &
   ```

2. **Start Phoenix with transport logging:**
   ```bash
   QT_LOGGING_RULES="phoenix.transport.debug=true" ./phoenix_app 2>&1 | tee phoenix_progress.log &
   ```

3. **In Phoenix UI:**
   - Set Samples: `50000` (large enough to see multiple progress updates)
   - Click "Run"
   - Observe progress bar updates

4. **Analyze Logs:**
   - Count Bedrock `Progress` messages
   - Count Phoenix `progressChanged()` emissions
   - Calculate frequencies

**Expected Outcomes:**

**Bedrock Progress Messages:**
- ✅ Initial: `sendProgress(jobId, 0.0, "Starting computation...")` (explicit 0%)
- ✅ During loop: Progress messages throttled to ≤5 Hz (every 200ms)
- ✅ Final: `sendProgress(jobId, 100.0, "Complete")`
- ✅ **Total messages:** For 50k samples, expect ~10-15 progress messages (not 50k)

**Phoenix UI Updates:**
- ✅ Progress bar updates throttled to ~2 Hz (every 500ms)
- ✅ **Total UI updates:** For 50k samples, expect ~5-8 updates (not 50k)
- ✅ Smooth progress bar animation (no jank)

**Expected Logs:**

**Bedrock (`bedrock_progress.log`):**
```
Started job: <uuid>
(Progress messages at ~5 Hz intervals)
Complete
```

**Phoenix (`phoenix_progress.log`):**
```
[phoenix.transport] Connected to palantir_bedrock
(progressChanged emissions at ~2 Hz intervals)
```

**Verification Method:**
```bash
# Count Bedrock progress messages
grep -c "Computing..." bedrock_progress.log

# Count Phoenix progress emissions
grep -c "progressChanged" phoenix_progress.log

# Calculate frequency (if test runs for T seconds):
# Bedrock frequency = (count - 2) / T  (exclude 0% and 100%)
# Phoenix frequency = (count) / T
```

**Expected Frequencies:**
- Bedrock: ≤5 Hz (every 200ms)
- Phoenix UI: ~2 Hz (every 500ms)

---

## 3. Expected Outputs by Scenario

### 3.1 TransportError Mapping

| Scenario | Bedrock Status | TransportError | UI Display |
|----------|----------------|----------------|------------|
| Happy Path | `StartReply.status = "OK"`, `ResultMeta.status = "SUCCEEDED"` | `NoError` | Plot updates, no error |
| Invalid Samples | `StartReply.status = "INVALID_ARGUMENT"` | `InvalidArgument` | Inline status bar (10s) |
| Invalid Frequency | `StartReply.status = "INVALID_ARGUMENT"` | `InvalidArgument` | Inline status bar (10s) |
| Invalid Amplitude | `StartReply.status = "INVALID_ARGUMENT"` | `InvalidArgument` | Inline status bar (10s) |
| Bedrock Down | Connection timeout/refused | `ConnectionFailed` or `ConnectionTimeout` | Toast dialog |
| Cancel | `ResultMeta.status = "CANCELLED"` | `Cancelled` | Inline status bar (5s) |
| Server Error | `ResultMeta.status = "FAILED"` | `ServerError` | Toast dialog (critical) |

### 3.2 UI Behavior Matrix

| Error Type | Display Method | Duration | Modal? |
|------------|----------------|----------|--------|
| `InvalidArgument` | Status bar | 10 seconds | No |
| `ConnectionFailed` / `ConnectionTimeout` | Toast (`QMessageBox::information`) | Until dismissed | Yes |
| `Cancelled` | Status bar | 5 seconds | No |
| `ServerError` | Toast (`QMessageBox::critical`) | Until dismissed | Yes |
| `PermissionDenied` | Toast (`QMessageBox::warning`) | Until dismissed | Yes |

### 3.3 Log Patterns

**Successful Computation:**
```
Bedrock: "Started job: <uuid>"
Bedrock: "Client disconnected"
Phoenix: "[phoenix.transport] computeXYSine: Successfully computed <N> samples"
```

**Invalid Argument:**
```
Bedrock: (No job started - rejected at validation)
Phoenix: "[phoenix.transport] computeXYSine: StartJob failed: INVALID_ARGUMENT - <message>"
```

**Cancelled:**
```
Bedrock: "Cancelled job: <uuid>"
Phoenix: "[phoenix.transport] cancelJob: Sent cancel for job <uuid>"
Phoenix: "[phoenix.transport] computeXYSine: Computation CANCELLED"
```

**Connection Failed:**
```
Phoenix: "[phoenix.transport] Connection timeout to palantir_bedrock - Connection timeout"
Phoenix: "[phoenix.transport] computeXYSine: Not connected"
```

### 3.4 Timing Expectations

| Operation | Expected Time | Max Acceptable |
|-----------|--------------|----------------|
| Connection | < 100ms | < 500ms |
| Cancel Response | < 200ms | < 500ms |
| 10k Samples Compute | < 1s | < 2s |
| 100k Samples Compute | < 5s | < 10s |
| Progress Update (Bedrock) | Every 200ms (≤5 Hz) | Every 100-300ms |
| Progress Update (Phoenix UI) | Every 500ms (~2 Hz) | Every 400-600ms |

---

## 4. Log Capture Strategy

### 4.1 Phoenix Logs

**Logging Category:**
- `phoenix.transport` — Transport layer logs
- Enable with: `QT_LOGGING_RULES="phoenix.transport.debug=true"`

**Capture Method:**
```bash
# Method 1: Redirect stderr to file
QT_LOGGING_RULES="phoenix.transport.debug=true" ./phoenix_app 2>&1 | tee phoenix_test.log

# Method 2: Use Qt logging to file (if configured)
QT_LOGGING_RULES="phoenix.transport.debug=true" ./phoenix_app > phoenix_test.log 2>&1
```

**Key Log Patterns to Scan:**
- `[phoenix.transport] Connected to palantir_bedrock` — Connection success
- `[phoenix.transport] Connection timeout` — Connection failure
- `[phoenix.transport] computeXYSine: StartJob failed` — Invalid argument
- `[phoenix.transport] computeXYSine: Successfully computed` — Success
- `[phoenix.transport] cancelJob: Sent cancel` — Cancel sent
- `[phoenix.transport] computeXYSine: Computation CANCELLED` — Cancel received

**Log Analysis Commands:**
```bash
# Count connection attempts
grep -c "Connected to" phoenix_test.log

# Count errors
grep -c "Transport Error" phoenix_test.log

# Count progress updates
grep -c "progressChanged" phoenix_test.log

# Extract error messages
grep "Transport Error" phoenix_test.log
```

### 4.2 Bedrock Logs

**Logging Method:**
- Uses `qDebug()` (no category filtering)
- All output goes to stderr

**Capture Method:**
```bash
# Redirect stderr to file
./bedrock_server 2>&1 | tee bedrock_test.log

# Or redirect directly
./bedrock_server > bedrock_test.log 2>&1
```

**Key Log Patterns to Scan:**
- `Bedrock server running on socket:` — Server started
- `Client connected` — Phoenix connected
- `Started job: <uuid>` — Job started
- `Cancelled job: <uuid>` — Job cancelled
- `Client disconnected` — Phoenix disconnected

**Log Analysis Commands:**
```bash
# Count jobs started
grep -c "Started job" bedrock_test.log

# Count cancellations
grep -c "Cancelled job" bedrock_test.log

# Count client connections
grep -c "Client connected" bedrock_test.log

# Extract job IDs
grep "Started job" bedrock_test.log | sed 's/.*Started job: //'
```

### 4.3 Combined Log Analysis

**Cross-Reference Patterns:**
- Match Phoenix job ID with Bedrock job ID (if logged)
- Verify timing: Phoenix sends request → Bedrock receives → Bedrock responds → Phoenix receives
- Check for race conditions: Cancel sent before job completes

**Analysis Script (pseudo-code):**
```bash
# Extract timestamps and events
grep -E "(Started job|Cancelled job|Connected|Disconnected)" bedrock_test.log | \
  awk '{print $1, $2, $0}' > bedrock_events.txt

grep -E "(computeXYSine|cancelJob|Connected)" phoenix_test.log | \
  awk '{print $1, $2, $0}' > phoenix_events.txt

# Compare timing
# (Manual review or simple diff)
```

---

## 5. Prerequisites

### 5.1 Build Prerequisites

**Bedrock:**
- ✅ Built with `BEDROCK_WITH_TRANSPORT_DEPS=ON`
- ✅ `bedrock_server` executable exists
- ✅ Qt6 available (for `QLocalServer`)

**Phoenix:**
- ✅ Built with `PHX_WITH_TRANSPORT_DEPS=ON`
- ✅ `phoenix_app` executable exists
- ✅ Transport library linked
- ✅ Qt6 available

**Verification:**
```bash
# Check Bedrock build
cd /home/ec2-user/workspace/bedrock
ls -la build-*/src/palantir/bedrock_server

# Check Phoenix build
cd /home/ec2-user/workspace/phoenix
ls -la build-*/phoenix_app
```

### 5.2 Runtime Prerequisites

**Environment Variables:**
- ✅ `PHOENIX_DEMO_MODE` — **MUST be unset or "0"** (demo mode bypasses Bedrock)
- ✅ `PHOENIX_LOCALSOCKET_NAME` — Optional (default: `"palantir_bedrock"`)
- ✅ `QT_LOGGING_RULES` — Optional (for verbose logging)

**Process Management:**
- ✅ Bedrock server must be running before Phoenix connects
- ✅ Both processes can run on same machine (LocalSocket)
- ✅ No network configuration needed

**UI Prerequisites:**
- ✅ X11/display available (for GUI testing)
- ✅ Or use headless mode with `QT_QPA_PLATFORM=offscreen` (if supported)

### 5.3 Test Execution Mode

**Manual Testing:**
- ✅ Bedrock started manually in terminal
- ✅ Phoenix started manually in terminal
- ✅ UI interactions performed manually
- ✅ Logs captured manually

**Automated Testing (Future):**
- ⚠️ Not in scope for Chunk 4C
- ⚠️ Would require test harness (Sprint 4.3)

---

## 6. Structured Test Matrix

| Test ID | Test Name | Steps | Expected Outcome | Pass Criteria |
|---------|-----------|-------|------------------|---------------|
| **C1** | Happy Path (10k samples) | 1. Start Bedrock<br>2. Start Phoenix<br>3. Set params (freq=1, amp=1, phase=0, samples=10k)<br>4. Click Run<br>5. Verify plot | Plot shows sine wave<br>10k points<br>X: [0, 2π]<br>Y: [-1, 1]<br>No errors | ✅ Plot renders correctly<br>✅ No error dialogs<br>✅ `TransportError::NoError`<br>✅ Progress ~2 Hz |
| **C2.1** | Invalid Samples (5) | 1. Start Bedrock<br>2. Start Phoenix<br>3. Set samples=5<br>4. Click Run | Inline error in status bar<br>Error: "Parameter 'samples' out of range: 5 (valid: 10-100000)"<br>No plot update | ✅ `TransportError::InvalidArgument`<br>✅ Status bar message (10s)<br>✅ No modal dialog<br>✅ Run button re-enabled |
| **C2.2** | Invalid Frequency (0.05) | 1. Set frequency=0.05<br>2. Click Run | Inline error in status bar<br>Error: "Parameter 'frequency' out of range: 0.05 (valid: 0.1-100.0)" | ✅ `TransportError::InvalidArgument`<br>✅ Status bar message<br>✅ No modal dialog |
| **C2.3** | Invalid Amplitude (-1.0) | 1. Set amplitude=-1.0<br>2. Click Run | Inline error in status bar<br>Error: "Parameter 'amplitude' out of range: -1.0 (valid: 0.0-10.0)" | ✅ `TransportError::InvalidArgument`<br>✅ Status bar message<br>✅ No modal dialog |
| **C2.4** | Invalid Samples (200k) | 1. Set samples=200000<br>2. Click Run | Inline error in status bar<br>Error: "Parameter 'samples' out of range: 200000 (valid: 10-100000)" | ✅ `TransportError::InvalidArgument`<br>✅ Status bar message<br>✅ No modal dialog |
| **C3** | Bedrock Unavailable | 1. Ensure Bedrock NOT running<br>2. Start Phoenix<br>3. Set valid params<br>4. Click Run | Toast dialog appears<br>Title: "Server Unavailable"<br>Message: "Bedrock server is not available..."<br>No plot update | ✅ `TransportError::ConnectionFailed` or `ConnectionTimeout`<br>✅ Toast dialog (`QMessageBox::information`)<br>✅ No plot update<br>✅ Run button re-enabled |
| **C4** | Cancel Mid-Run (100k samples) | 1. Start Bedrock<br>2. Start Phoenix<br>3. Set samples=100000<br>4. Click Run<br>5. Immediately click Cancel (< 200ms) | Inline "Cancelled" message in status bar<br>No plot update<br>Computation stops quickly | ✅ Cancel sent to Bedrock<br>✅ `TransportError::Cancelled`<br>✅ Status bar message (5s)<br>✅ Cancel response < 200ms<br>✅ Plot does NOT update<br>✅ Run button re-enabled |
| **C5** | Progress Throttling (50k samples) | 1. Start Bedrock with logging<br>2. Start Phoenix with logging<br>3. Set samples=50000<br>4. Click Run<br>5. Analyze logs | Bedrock emits ≤5 Hz progress<br>Phoenix UI updates ~2 Hz<br>Smooth progress bar | ✅ Bedrock: ≤5 Hz (every 200ms)<br>✅ Phoenix UI: ~2 Hz (every 500ms)<br>✅ No jank or flood<br>✅ Explicit 0% progress sent |

---

## 7. Test Execution Checklist

### Pre-Test Setup

- [ ] Verify Bedrock builds successfully
- [ ] Verify Phoenix builds successfully
- [ ] Verify `PHOENIX_DEMO_MODE` is NOT set to "1"
- [ ] Verify socket names align (default `palantir_bedrock`)
- [ ] Prepare log capture directories
- [ ] Verify display/X11 available (for GUI testing)

### Test Execution

- [ ] **C1:** Happy Path — Execute and verify
- [ ] **C2.1:** Invalid Samples — Execute and verify
- [ ] **C2.2:** Invalid Frequency — Execute and verify
- [ ] **C2.3:** Invalid Amplitude — Execute and verify
- [ ] **C2.4:** Invalid Samples (large) — Execute and verify
- [ ] **C3:** Bedrock Unavailable — Execute and verify
- [ ] **C4:** Cancel Mid-Run — Execute and verify
- [ ] **C5:** Progress Throttling — Execute and verify

### Post-Test Analysis

- [ ] Review all log files
- [ ] Verify error code mappings
- [ ] Verify UI behavior matches expectations
- [ ] Verify timing meets requirements
- [ ] Document any anomalies or failures
- [ ] Create test report

---

## 8. Test Report Template

**Test Report Structure:**

```
# Sprint 4.2 Chunk 4C: XY Sine Integration Test Report

## Test Environment
- Bedrock Version: <version>
- Phoenix Version: <version>
- Build Configuration: <config>
- Test Date: <date>
- Tester: <name>

## Test Results

### C1: Happy Path (10k samples)
- Status: ✅ PASS / ❌ FAIL
- TransportError: <observed>
- Plot Rendered: Yes/No
- Timing: <actual time>
- Notes: <any issues>

### C2.1: Invalid Samples (5)
- Status: ✅ PASS / ❌ FAIL
- TransportError: <observed>
- UI Display: <observed>
- Notes: <any issues>

[... repeat for all tests ...]

## Summary
- Total Tests: 8
- Passed: <N>
- Failed: <N>
- Blocked: <N>

## Issues Found
- <list any issues>

## Recommendations
- <any follow-up actions>
```

---

## 9. Open Questions / Ambiguities

### Question 1: Explicit 0% Progress

**Question:** Does `computeXYSineWithProgress()` send explicit `sendProgress(jobId, 0.0, ...)` before entering the loop?

**Current State:** `processJob()` sends `sendProgress(jobId, 0.0, "Starting computation...")` at line 290, before calling `computeXYSineWithProgress()`.

**Answer:** ✅ **Yes** — Explicit 0% progress is sent in `processJob()` before computation starts.

**Verification:** Check Bedrock logs for "Starting computation..." message.

---

### Question 2: Chunk Assembly Timeout

**Question:** Should Phoenix enforce a total timeout for chunk assembly (e.g., 30 seconds)?

**Current State:** `LocalSocketChannel::computeXYSine()` has a 10-second timeout for `ResultMeta` reception, but no explicit timeout for total chunk assembly.

**Answer:** ⚠️ **Optional** — Can be added if issues found during testing (Phase D).

**Recommendation:** Test without timeout first, add if needed.

---

### Question 3: Multiple Windows

**Question:** Do tests require multiple Phoenix windows or can all tests run in a single window?

**Answer:** ✅ **Single window sufficient** — All tests can run sequentially in one XY Analysis Window.

**Recommendation:** Use single window, reset parameters between tests.

---

### Question 4: Bedrock Auto-Start

**Question:** Should tests assume Bedrock is manually started, or should test harness auto-start it?

**Answer:** ✅ **Manual start for Chunk 4C** — Tests assume Bedrock is manually started.

**Rationale:** Keeps test execution simple, allows manual log inspection.

**Future:** Auto-start could be added in Sprint 4.3 for automated testing.

---

## 10. Summary

**Test Plan Scope:**
- ✅ 8 test scenarios (C1-C5, with C2 split into 4 sub-tests)
- ✅ Comprehensive error code verification
- ✅ UI behavior verification
- ✅ Timing verification
- ✅ Log capture and analysis

**Test Execution:**
- ✅ Manual execution (no automation required)
- ✅ Logs captured to files
- ✅ Results documented in test report

**Success Criteria:**
- ✅ All tests pass
- ✅ Error codes map correctly
- ✅ UI displays errors appropriately
- ✅ Timing meets requirements
- ✅ No crashes or exceptions

**Next Steps:**
- Execute tests according to this plan
- Generate test report
- Document any issues found
- Proceed to Phase D (optional improvements) if needed

---

**Ready for Test Execution:** ✅ Yes — All prerequisites identified, test plans defined, expected outcomes specified.

