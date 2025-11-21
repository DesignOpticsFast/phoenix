# Sprint 4.2 Chunk 4C: Automated Heavy-Lift Testing Plan

**Date:** 2025-11-21  
**Status:** PLANNING — Awaiting Approval  
**Intent:** Automate maximum possible integration testing (transport layer only), leaving only GUI plot rendering for manual verification

---

## ⚠️ UNDERLORD WORKFLOW DIRECTIVE (CRITICAL)

**All work MUST operate on the canonical repo working copy:**
- `/home/ec2-user/workspace/phoenix/`
- `/home/ec2-user/workspace/bedrock/`

**ZIP files are reference only.**

---

## Executive Summary

**Automation Strategy:** Extend existing `test_xysine_transport.cpp` with comprehensive automated tests that exercise the transport layer directly (headless) without GUI dependency.

**What Will Be Automated:**
- ✅ Bedrock server startup/shutdown (via QProcess)
- ✅ Phoenix `LocalSocketChannel` direct calls (extend existing test)
- ✅ All transport layer tests (C1-C5)
- ✅ Error code mapping validation
- ✅ Progress throttling measurement
- ✅ Cancel behavior verification
- ✅ Log capture and analysis
- ✅ Test report generation

**What Remains Manual:**
- ⚠️ GUI plot rendering verification (visual check only)

---

## 1. Build Strategy

### 1.1 Bedrock Build

**Location:** `/home/ec2-user/workspace/bedrock`

**Build Command:**
```bash
cd /home/ec2-user/workspace/bedrock
mkdir -p build-chunk4c
cmake -B build-chunk4c \
    -DBEDROCK_WITH_TRANSPORT_DEPS=ON \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build-chunk4c --target bedrock_server -j$(nproc)
```

**Build Log Capture:**
- `chunk4c/build_logs/bedrock_cmake.log` — CMake configuration output
- `chunk4c/build_logs/bedrock_build.log` — Build compilation output

**Verification:**
- Binary exists: `build-chunk4c/src/palantir/bedrock_server`
- Binary is executable
- Binary runs without errors

### 1.2 Phoenix Test Build

**Location:** `/home/ec2-user/workspace/phoenix`

**Build Command:**
```bash
cd /home/ec2-user/workspace/phoenix
mkdir -p build-chunk4c
cmake -B build-chunk4c \
    -DPHX_WITH_TRANSPORT_DEPS=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON
cmake --build build-chunk4c --target test_xysine_transport -j$(nproc)
```

**Build Log Capture:**
- `chunk4c/build_logs/phoenix_cmake.log` — CMake configuration output
- `chunk4c/build_logs/phoenix_build.log` — Build compilation output

**Verification:**
- Test executable exists: `build-chunk4c/tests/test_xysine_transport`
- Test executable is runnable

---

## 2. Test Extension Strategy

### 2.1 Extend Existing Test File

**File:** `/home/ec2-user/workspace/phoenix/tests/test_xysine_transport.cpp`

**Current Tests:**
- `testXYSineComputeSuccess()` — Basic happy path (100 samples)
- `testXYSineParameterConversion()` — Parameter conversion test
- `testXYSineConnectionFailure()` — Connection failure test

**New Tests to Add:**
- `testC1HappyPath10k()` — C1: Happy path with 10k samples (detailed validation)
- `testC2InvalidSamples()` — C2.1: Invalid samples (5)
- `testC2InvalidFrequency()` — C2.2: Invalid frequency (0.05)
- `testC2InvalidAmplitude()` — C2.3: Invalid amplitude (-1.0)
- `testC2InvalidSamplesLarge()` — C2.4: Invalid samples (200000)
- `testC3BedrockUnavailable()` — C3: Bedrock unavailable (connection failure)
- `testC4CancelMidRun()` — C4: Cancel mid-run (100k samples)
- `testC5ProgressThrottling()` — C5: Progress throttling (50k samples)

### 2.2 Bedrock Process Management Helper

**Add to test file:**
```cpp
class BedrockProcessHelper {
public:
    BedrockProcessHelper() : process_(nullptr) {}
    
    bool start(const QString& socketName = "palantir_bedrock") {
        // Find Bedrock binary
        QString bedrockDir = "/home/ec2-user/workspace/bedrock";
        QString binaryPath = findBedrockBinary(bedrockDir);
        
        if (binaryPath.isEmpty()) {
            qWarning() << "Bedrock binary not found";
            return false;
        }
        
        // Start process
        process_ = new QProcess();
        process_->setProgram(binaryPath);
        process_->setArguments({"--socket", socketName});
        
        QString logFile = QString("chunk4c/logs/bedrock_runtime.log");
        process_->setStandardOutputFile(logFile);
        process_->setStandardErrorFile(logFile);
        
        process_->start();
        
        if (!process_->waitForStarted(5000)) {
            qWarning() << "Bedrock failed to start";
            return false;
        }
        
        // Wait for socket availability
        QLocalSocket testSocket;
        for (int i = 0; i < 50; ++i) {
            testSocket.connectToServer(socketName);
            if (testSocket.waitForConnected(100)) {
                testSocket.disconnectFromServer();
                return true;
            }
            QThread::msleep(100);
        }
        
        qWarning() << "Bedrock socket not available after 5 seconds";
        return false;
    }
    
    void stop() {
        if (process_ && process_->state() != QProcess::NotRunning) {
            process_->terminate();
            if (!process_->waitForFinished(3000)) {
                process_->kill();
                process_->waitForFinished(1000);
            }
        }
    }
    
    ~BedrockProcessHelper() {
        stop();
        delete process_;
    }
    
private:
    QString findBedrockBinary(const QString& bedrockDir) {
        // Try build-chunk4c first, then fallback to other build dirs
        QStringList candidates = {
            bedrockDir + "/build-chunk4c/src/palantir/bedrock_server",
            bedrockDir + "/build-default/src/palantir/bedrock_server",
            bedrockDir + "/build/src/palantir/bedrock_server"
        };
        
        for (const QString& candidate : candidates) {
            if (QFile::exists(candidate)) {
                return candidate;
            }
        }
        return QString();
    }
    
    QProcess* process_;
};
```

---

## 3. Automated Test Scenarios

### Test C1: Happy Path (10k samples)

**Implementation:**
```cpp
void XYSineTransportTests::testC1HappyPath10k()
{
    BedrockProcessHelper bedrock;
    if (!bedrock.start()) {
        QSKIP("Bedrock server not available - skipping C1 test");
    }
    
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString socketName = env.value("PHOENIX_LOCALSOCKET_NAME", "palantir_bedrock");
    
    LocalSocketChannel client(socketName);
    QVERIFY(client.connect());
    
    // Track progress
    QList<QPair<qint64, double>> progressTimestamps;
    QElapsedTimer progressTimer;
    progressTimer.start();
    
    auto progressCallback = [&progressTimestamps, &progressTimer](double percent, const QString& status) {
        progressTimestamps.append({progressTimer.elapsed(), percent});
        qDebug() << "Progress:" << percent << "% -" << status;
    };
    
    // Test parameters
    QMap<QString, QVariant> params;
    params.insert("frequency", 1.0);
    params.insert("amplitude", 1.0);
    params.insert("phase", 0.0);
    params.insert("samples", 10000);
    
    // Compute
    XYSineResult result;
    QElapsedTimer timer;
    timer.start();
    
    bool success = client.computeXYSine(params, result, progressCallback);
    qint64 elapsed = timer.elapsed();
    
    QVERIFY(success);
    QCOMPARE(client.lastError(), TransportError::NoError);
    
    // Validate result size
    QCOMPARE(result.x.size(), 10000);
    QCOMPARE(result.y.size(), 10000);
    
    // Validate X domain [0, 2π]
    const double twoPi = 2.0 * M_PI;
    QVERIFY(std::abs(result.x[0] - 0.0) < 1e-10);
    QVERIFY(std::abs(result.x[9999] - twoPi) < 1e-6);
    
    // Validate Y range [-1, 1]
    double minY = *std::min_element(result.y.begin(), result.y.end());
    double maxY = *std::max_element(result.y.begin(), result.y.end());
    QVERIFY(minY >= -1.1 && maxY <= 1.1);
    
    // Validate timing (< 2 seconds)
    QVERIFY(elapsed < 2000);
    
    // Validate progress updates received
    QVERIFY(progressTimestamps.size() > 0);
    
    qDebug() << "C1 PASSED: size=" << result.x.size() 
             << ", timing=" << elapsed << "ms"
             << ", progress_updates=" << progressTimestamps.size();
    
    client.disconnect();
}
```

**Validation Points:**
- ✅ `TransportError::NoError`
- ✅ Result size = 10,000 points
- ✅ X domain: [0, 2π]
- ✅ Y range: [-1, 1]
- ✅ Timing < 2 seconds
- ✅ Progress callbacks received

---

### Test C2: Invalid Parameters (4 Sub-Tests)

**Implementation:**
```cpp
void XYSineTransportTests::testC2InvalidSamples()
{
    BedrockProcessHelper bedrock;
    if (!bedrock.start()) {
        QSKIP("Bedrock server not available");
    }
    
    LocalSocketChannel client("palantir_bedrock");
    QVERIFY(client.connect());
    
    QMap<QString, QVariant> params;
    params.insert("samples", 5);  // Invalid: too small
    
    XYSineResult result;
    bool success = client.computeXYSine(params, result);
    
    QVERIFY(!success);
    QCOMPARE(client.lastError(), TransportError::InvalidArgument);
    
    QString errorString = client.lastErrorString();
    QVERIFY(errorString.contains("samples", Qt::CaseInsensitive) || 
            errorString.contains("5", Qt::CaseInsensitive));
    
    client.disconnect();
}

void XYSineTransportTests::testC2InvalidFrequency()
{
    BedrockProcessHelper bedrock;
    if (!bedrock.start()) {
        QSKIP("Bedrock server not available");
    }
    
    LocalSocketChannel client("palantir_bedrock");
    QVERIFY(client.connect());
    
    QMap<QString, QVariant> params;
    params.insert("frequency", 0.05);  // Invalid: too small
    params.insert("samples", 1000);
    
    XYSineResult result;
    bool success = client.computeXYSine(params, result);
    
    QVERIFY(!success);
    QCOMPARE(client.lastError(), TransportError::InvalidArgument);
    
    QString errorString = client.lastErrorString();
    QVERIFY(errorString.contains("frequency", Qt::CaseInsensitive));
    
    client.disconnect();
}

void XYSineTransportTests::testC2InvalidAmplitude()
{
    BedrockProcessHelper bedrock;
    if (!bedrock.start()) {
        QSKIP("Bedrock server not available");
    }
    
    LocalSocketChannel client("palantir_bedrock");
    QVERIFY(client.connect());
    
    QMap<QString, QVariant> params;
    params.insert("amplitude", -1.0);  // Invalid: negative
    params.insert("samples", 1000);
    
    XYSineResult result;
    bool success = client.computeXYSine(params, result);
    
    QVERIFY(!success);
    QCOMPARE(client.lastError(), TransportError::InvalidArgument);
    
    QString errorString = client.lastErrorString();
    QVERIFY(errorString.contains("amplitude", Qt::CaseInsensitive));
    
    client.disconnect();
}

void XYSineTransportTests::testC2InvalidSamplesLarge()
{
    BedrockProcessHelper bedrock;
    if (!bedrock.start()) {
        QSKIP("Bedrock server not available");
    }
    
    LocalSocketChannel client("palantir_bedrock");
    QVERIFY(client.connect());
    
    QMap<QString, QVariant> params;
    params.insert("samples", 200000);  // Invalid: too large
    
    XYSineResult result;
    bool success = client.computeXYSine(params, result);
    
    QVERIFY(!success);
    QCOMPARE(client.lastError(), TransportError::InvalidArgument);
    
    QString errorString = client.lastErrorString();
    QVERIFY(errorString.contains("samples", Qt::CaseInsensitive) || 
            errorString.contains("200000", Qt::CaseInsensitive));
    
    client.disconnect();
}
```

**Validation Points:**
- ✅ All 4 cases return `TransportError::InvalidArgument`
- ✅ Error messages contain parameter names
- ✅ No crashes or exceptions

---

### Test C3: Bedrock Unavailable

**Implementation:**
```cpp
void XYSineTransportTests::testC3BedrockUnavailable()
{
    // Ensure Bedrock is NOT running
    BedrockProcessHelper bedrock;
    // Don't start it
    
    // Use invalid socket name
    LocalSocketChannel client("nonexistent_socket_chunk4c_test");
    
    // Attempt connection
    bool connected = client.connect();
    QVERIFY(!connected);
    
    // Validate error code
    TransportError error = client.lastError();
    QVERIFY(error == TransportError::ConnectionFailed || 
            error == TransportError::ConnectionTimeout);
    
    // Validate error message
    QString errorString = client.lastErrorString();
    QVERIFY(errorString.contains("Bedrock", Qt::CaseInsensitive) || 
            errorString.contains("connect", Qt::CaseInsensitive) ||
            errorString.contains("timeout", Qt::CaseInsensitive));
    
    // Attempt computation (should also fail)
    QMap<QString, QVariant> params;
    params.insert("samples", 1000);
    XYSineResult result;
    bool success = client.computeXYSine(params, result);
    
    QVERIFY(!success);
    QVERIFY(client.lastError() == TransportError::ConnectionFailed || 
            client.lastError() == TransportError::ConnectionTimeout);
}
```

**Validation Points:**
- ✅ `TransportError::ConnectionFailed` or `ConnectionTimeout`
- ✅ Error message contains guidance
- ✅ `computeXYSine()` also fails appropriately

---

### Test C4: Cancel Mid-Run

**Implementation:**
```cpp
void XYSineTransportTests::testC4CancelMidRun()
{
    BedrockProcessHelper bedrock;
    if (!bedrock.start()) {
        QSKIP("Bedrock server not available");
    }
    
    LocalSocketChannel client("palantir_bedrock");
    QVERIFY(client.connect());
    
    // Prepare large job
    QMap<QString, QVariant> params;
    params.insert("samples", 100000);  // Large job
    
    // Track cancellation
    bool cancelReceived = false;
    QString jobId;
    
    auto progressCallback = [&cancelReceived](double percent, const QString& status) {
        if (status == "CANCELLED") {
            cancelReceived = true;
        }
    };
    
    // Start computation in a separate thread to allow cancellation
    QThread* computeThread = QThread::create([&]() {
        XYSineResult result;
        client.computeXYSine(params, result, progressCallback);
    });
    
    computeThread->start();
    
    // Wait a bit, then cancel
    QThread::msleep(200);
    
    // Get job ID (from currentJobId)
    jobId = client.currentJobId();
    QVERIFY(!jobId.isEmpty());
    
    // Cancel the job
    bool cancelSuccess = client.cancelJob(jobId);
    QVERIFY(cancelSuccess);
    
    // Wait for computation to finish
    computeThread->wait(5000);
    
    // Verify cancellation was received
    // Note: Since computeXYSine is synchronous, cancellation may happen
    // after computation completes, which is acceptable
    
    // Verify error code
    TransportError error = client.lastError();
    if (error == TransportError::Cancelled) {
        qDebug() << "C4 PASSED: Cancellation detected";
    } else if (error == TransportError::NoError) {
        // Computation completed before cancel (acceptable)
        qDebug() << "C4 PASSED: Computation completed before cancel";
    } else {
        QFAIL(QString("Unexpected error: %1").arg(static_cast<int>(error)).toLocal8Bit().data());
    }
    
    delete computeThread;
    client.disconnect();
}
```

**Note:** Since `computeXYSine()` is synchronous, true mid-computation cancellation requires threading. This test uses a thread to allow cancellation during computation.

**Validation Points:**
- ✅ Cancel message sent successfully
- ✅ `TransportError::Cancelled` received (or computation completes first)
- ✅ No hangs or crashes

---

### Test C5: Progress Throttling

**Implementation:**
```cpp
void XYSineTransportTests::testC5ProgressThrottling()
{
    BedrockProcessHelper bedrock;
    if (!bedrock.start()) {
        QSKIP("Bedrock server not available");
    }
    
    LocalSocketChannel client("palantir_bedrock");
    QVERIFY(client.connect());
    
    // Track progress timestamps
    QList<QPair<qint64, double>> progressTimestamps;
    QElapsedTimer progressTimer;
    progressTimer.start();
    
    auto progressCallback = [&progressTimestamps, &progressTimer](double percent, const QString& status) {
        progressTimestamps.append({progressTimer.elapsed(), percent});
    };
    
    // Prepare medium-sized job
    QMap<QString, QVariant> params;
    params.insert("samples", 50000);
    
    // Compute
    XYSineResult result;
    bool success = client.computeXYSine(params, result, progressCallback);
    
    QVERIFY(success);
    
    // Analyze progress timestamps
    QVERIFY(progressTimestamps.size() >= 2);
    
    // Calculate intervals
    QList<qint64> intervals;
    for (int i = 1; i < progressTimestamps.size(); ++i) {
        qint64 delta = progressTimestamps[i].first - progressTimestamps[i-1].first;
        intervals.append(delta);
    }
    
    // Calculate average interval
    qint64 totalInterval = 0;
    for (qint64 interval : intervals) {
        totalInterval += interval;
    }
    double avgInterval = static_cast<double>(totalInterval) / intervals.size();
    
    // Calculate frequency (Hz)
    double frequency = 1000.0 / avgInterval;
    
    qDebug() << "Progress analysis:";
    qDebug() << "  Total updates:" << progressTimestamps.size();
    qDebug() << "  Average interval:" << avgInterval << "ms";
    qDebug() << "  Frequency:" << frequency << "Hz";
    qDebug() << "  Expected: ~2 Hz (Phoenix UI throttled from ≤5 Hz Bedrock)";
    
    // Validate throttling (should be ~2 Hz, allow 1.5-3 Hz range)
    QVERIFY(frequency >= 1.5 && frequency <= 3.0);
    
    // Verify initial 0% and final 100%
    QVERIFY(std::abs(progressTimestamps[0].second - 0.0) < 1e-6);
    QVERIFY(progressTimestamps.last().second >= 99.0);
    
    qDebug() << "C5 PASSED: Progress throttling verified";
    
    client.disconnect();
}
```

**Validation Points:**
- ✅ Progress updates received
- ✅ Frequency ~2 Hz (Phoenix UI throttled)
- ✅ Initial 0% progress sent
- ✅ Final 100% progress sent

---

## 4. Log Capture Strategy

### 4.1 Log Directory Structure

```
chunk4c/
├── build_logs/
│   ├── bedrock_cmake.log
│   ├── bedrock_build.log
│   ├── phoenix_cmake.log
│   └── phoenix_build.log
├── logs/
│   ├── bedrock_runtime.log          # Bedrock stdout/stderr
│   └── test_execution.log           # Test execution log
└── INTEGRATION_TEST_REPORT.md       # Generated test report
```

### 4.2 Log Capture Implementation

**Add to test class:**
```cpp
class XYSineTransportTests : public QObject {
    Q_OBJECT
    
private slots:
    void initTestCase();  // Setup
    void cleanupTestCase();  // Teardown
    
    // ... existing and new tests ...
    
private:
    void setupLogging();
    BedrockProcessHelper* bedrockHelper_;
    QFile* logFile_;
};

void XYSineTransportTests::initTestCase()
{
    // Create log directory
    QDir().mkpath("chunk4c/logs");
    
    // Setup logging
    setupLogging();
    
    // Start Bedrock (for tests that need it)
    bedrockHelper_ = new BedrockProcessHelper();
    // Don't start here - let each test decide
}

void XYSineTransportTests::cleanupTestCase()
{
    if (bedrockHelper_) {
        bedrockHelper_->stop();
        delete bedrockHelper_;
    }
    
    if (logFile_) {
        logFile_->close();
        delete logFile_;
    }
}
```

---

## 5. Test Report Generation

### 5.1 Report Structure

**File:** `chunk4c/INTEGRATION_TEST_REPORT.md`

**Sections:**
1. Executive Summary
2. Test Results (Detailed)
3. Error Code Mapping Verification
4. Timing Measurements
5. Progress Throttling Analysis
6. Issues Found
7. Recommendations

### 5.2 Report Generation

**Add to test class:**
```cpp
void XYSineTransportTests::generateReport()
{
    QFile reportFile("chunk4c/INTEGRATION_TEST_REPORT.md");
    if (!reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open report file";
        return;
    }
    
    QTextStream out(&reportFile);
    
    out << "# Sprint 4.2 Chunk 4C: Integration Test Report\n\n";
    out << "**Test Date:** " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    out << "**Environment:** dev-01 (Linux)\n\n";
    
    // ... generate report sections ...
    
    reportFile.close();
}
```

---

## 6. Execution Flow

### 6.1 Main Test Runner Script

**File:** `chunk4c/run_integration_tests.sh`

```bash
#!/bin/bash
set -e

TEST_DIR="/home/ec2-user/workspace/phoenix/chunk4c"
BEDROCK_DIR="/home/ec2-user/workspace/bedrock"
PHOENIX_DIR="/home/ec2-user/workspace/phoenix"

# Create directories
mkdir -p "$TEST_DIR"/{build_logs,logs}

# Build Bedrock
echo "Building Bedrock..."
cd "$BEDROCK_DIR"
cmake -B build-chunk4c -DBEDROCK_WITH_TRANSPORT_DEPS=ON -DCMAKE_BUILD_TYPE=Release \
    2>&1 | tee "$TEST_DIR/build_logs/bedrock_cmake.log"
cmake --build build-chunk4c --target bedrock_server -j$(nproc) \
    2>&1 | tee "$TEST_DIR/build_logs/bedrock_build.log"

# Build Phoenix tests
echo "Building Phoenix tests..."
cd "$PHOENIX_DIR"
cmake -B build-chunk4c -DPHX_WITH_TRANSPORT_DEPS=ON -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
    2>&1 | tee "$TEST_DIR/build_logs/phoenix_cmake.log"
cmake --build build-chunk4c --target test_xysine_transport -j$(nproc) \
    2>&1 | tee "$TEST_DIR/build_logs/phoenix_build.log"

# Run tests
echo "Running integration tests..."
cd "$PHOENIX_DIR/build-chunk4c"
./tests/test_xysine_transport 2>&1 | tee "$TEST_DIR/logs/test_execution.log"

echo "Tests complete. Check chunk4c/INTEGRATION_TEST_REPORT.md for results."
```

---

## 7. Implementation Plan

### Phase 1: Extend Test File (45 min)
1. Add `BedrockProcessHelper` class to test file
2. Add new test methods (C1-C5)
3. Add progress tracking infrastructure
4. Add log capture setup

### Phase 2: Build & Execute (30 min)
1. Build Bedrock
2. Build Phoenix tests
3. Run all tests
4. Capture logs

### Phase 3: Generate Report (15 min)
1. Parse test results
2. Calculate statistics
3. Generate markdown report

**Total Estimated Time:** ~1.5 hours

---

## 8. Validation Checklist

### Automated Validations

- [ ] Bedrock builds successfully
- [ ] Phoenix tests build successfully
- [ ] Bedrock starts and socket is available
- [ ] C1: Happy path completes successfully
- [ ] C1: Result data is correct (size, domain, range)
- [ ] C2: All 4 invalid parameter cases return `InvalidArgument`
- [ ] C2: Error messages contain parameter names
- [ ] C3: Connection failure returns `ConnectionFailed` or `ConnectionTimeout`
- [ ] C3: Error message contains guidance
- [ ] C4: Cancel works (API call succeeds)
- [ ] C4: `Cancelled` status received (or computation completes)
- [ ] C5: Progress frequency ~2 Hz
- [ ] C5: Initial 0% and final 100% progress sent

### Manual Validations (GUI Only)

- [ ] Plot renders correctly
- [ ] Error messages display in status bar (not dialogs)
- [ ] Cancel button works in UI
- [ ] Progress bar updates smoothly

---

## 9. Open Questions

### Question 1: Cancel Test Threading

**Question:** Is threading acceptable for cancel test, or should we use a different approach?

**Recommendation:** Use `QThread` to allow cancellation during synchronous `computeXYSine()` call.

**Your Decision:** ❓

### Question 2: Bedrock Binary Path

**Question:** Where will Bedrock binary be located after build?

**Assumption:** `bedrock/build-chunk4c/src/palantir/bedrock_server`

**Fallback:** Search multiple build directories.

**Your Decision:** ❓

---

## Summary

**Automation Plan:**
1. ✅ Extend existing `test_xysine_transport.cpp` with comprehensive tests
2. ✅ Add `BedrockProcessHelper` for process management
3. ✅ Implement all 5 test scenarios (C1-C5)
4. ✅ Add progress tracking and throttling analysis
5. ✅ Capture all logs
6. ✅ Generate comprehensive test report
7. ⚠️ Leave only GUI plot rendering for manual verification

**Test Coverage:**
- ✅ Happy path (10k samples)
- ✅ Invalid parameters (4 cases)
- ✅ Bedrock unavailable
- ✅ Cancel behavior
- ✅ Progress throttling

**Ready for Implementation:** ✅ Yes — Awaiting approval to proceed.
