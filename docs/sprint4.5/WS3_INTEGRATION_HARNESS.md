# Workstream 3 – Integration Test Harness

**Date:** 2025-01-25  
**Sprint:** 4.5  
**Workstream:** 3  
**Status:** ✅ Complete

---

## Overview

The integration test harness provides end-to-end testing of the envelope-based Palantir transport protocol. It validates the complete client-server communication path using real `PalantirServer` instances and a minimal test client.

---

## Architecture

### Components

1. **`IntegrationTestServerFixture`** - Manages in-process server lifecycle
2. **`IntegrationTestClient`** - Minimal C++ client for testing
3. **Integration Test Cases** - GoogleTest-based test suites

### Test Flow

```
Test Setup
  ↓
Start IntegrationTestServerFixture (in-process PalantirServer)
  ↓
Create IntegrationTestClient
  ↓
Connect to server via QLocalSocket
  ↓
Send envelope-encoded request
  ↓
Receive envelope-encoded response
  ↓
Validate response content
  ↓
Test Teardown
```

---

## IntegrationTestServerFixture

### Purpose

Manages the lifecycle of an in-process `PalantirServer` for integration testing. The server runs in the same process as the test, eliminating the need for external server processes.

### Location

- **Header:** `bedrock/tests/integration/IntegrationTestServerFixture.hpp`
- **Implementation:** `bedrock/tests/integration/IntegrationTestServerFixture.cpp`

### Key Methods

```cpp
class IntegrationTestServerFixture {
public:
    bool startServer();
    void stopServer();
    QString socketPath() const;
    bool isRunning() const;
};
```

### How It Works

1. **Server Creation:**
   - Creates a `PalantirServer` instance
   - Generates a unique socket path (UUID-based)
   - Starts the server on the generated socket path

2. **Event Loop Management:**
   - Ensures `QCoreApplication` exists
   - Processes Qt events to allow server initialization
   - Uses `QThread::msleep()` to give server time to start

3. **Cleanup:**
   - Stops server on fixture destruction
   - Cleans up socket file

### Usage Example

```cpp
class MyIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(fixture_.startServer());
        QCoreApplication::processEvents();
        QThread::msleep(100);
    }
    
    void TearDown() override {
        fixture_.stopServer();
    }
    
    IntegrationTestServerFixture fixture_;
};

TEST_F(MyIntegrationTest, MyRPCTest) {
    IntegrationTestClient client;
    ASSERT_TRUE(client.connect(fixture_.socketPath()));
    // ... test logic ...
}
```

---

## IntegrationTestClient

### Purpose

A minimal C++ client that mirrors Phoenix's `LocalSocketChannel` behavior. It uses the live envelope transport helpers to send and receive messages.

### Location

- **Header:** `bedrock/tests/integration/IntegrationTestClient.hpp`
- **Implementation:** `bedrock/tests/integration/IntegrationTestClient.cpp`

### Key Methods

```cpp
class IntegrationTestClient {
public:
    bool connect(const QString& socketPath);
    void disconnect();
    bool isConnected() const;
    
    // RPC-specific helpers
    bool getCapabilities(palantir::CapabilitiesResponse& outResponse, QString& outError);
    bool sendXYSineRequest(const palantir::XYSineRequest& request, 
                          palantir::XYSineResponse& outResponse, 
                          QString& outError);
    
private:
    // Low-level envelope transport
    bool sendEnvelope(palantir::MessageType type, 
                     const google::protobuf::Message& message, 
                     QString& outError);
    bool receiveEnvelope(palantir::MessageEnvelope& outEnvelope, 
                        QString& outError);
};
```

### How It Works

1. **Connection:**
   - Uses `QLocalSocket` to connect to server
   - Waits for connection with timeout (5 seconds)

2. **Sending Messages:**
   - Creates `MessageEnvelope` using `bedrock::palantir::makeEnvelope()`
   - Serializes envelope to bytes
   - Writes `[4-byte length][serialized envelope]` to socket
   - Flushes socket

3. **Receiving Messages:**
   - Reads 4-byte length prefix
   - Waits for complete envelope using `QTest::qWait()` to spin event loop
   - Reads envelope bytes
   - Parses using `bedrock::palantir::parseEnvelope()`
   - Validates envelope version (must be 1)

4. **Event Loop Handling:**
   - Uses `QTest::qWait()` to properly spin Qt event loop
   - Ensures `readyRead()` signals are processed
   - Handles timeouts gracefully

---

## Adding New RPC Integration Tests

### Step 1: Add Client Helper Method

In `IntegrationTestClient.hpp`:

```cpp
bool myNewRPC(const palantir::MyRequest& request, 
              palantir::MyResponse& outResponse, 
              QString& outError);
```

In `IntegrationTestClient.cpp`:

```cpp
bool IntegrationTestClient::myNewRPC(const palantir::MyRequest& request, 
                                     palantir::MyResponse& outResponse, 
                                     QString& outError)
{
    // Send request
    if (!sendEnvelope(palantir::MessageType::MY_RPC_REQUEST, request, outError)) {
        return false;
    }
    
    // Receive envelope
    palantir::MessageEnvelope envelope;
    if (!receiveEnvelope(envelope, outError)) {
        return false;
    }
    
    // Validate response type
    if (envelope.type() != palantir::MessageType::MY_RPC_RESPONSE) {
        outError = QString("Unexpected message type: %1").arg(static_cast<int>(envelope.type()));
        return false;
    }
    
    // Parse inner response
    const std::string& payload = envelope.payload();
    if (!outResponse.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
        outError = "Failed to parse MyResponse from envelope payload";
        return false;
    }
    
    return true;
}
```

### Step 2: Create Test File

Create `bedrock/tests/integration/MyRPCIntegrationTest.cpp`:

```cpp
#include "IntegrationTestServerFixture.hpp"
#include "IntegrationTestClient.hpp"

#ifdef BEDROCK_WITH_TRANSPORT_DEPS
#include <gtest/gtest.h>
#include "palantir/my_rpc.pb.h"
#include <QCoreApplication>
#include <QThread>

class MyRPCIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!QCoreApplication::instance()) {
            static int argc = 1;
            static char* argv[] = { const_cast<char*>("integration_tests"), nullptr };
            app_ = std::make_unique<QCoreApplication>(argc, argv);
        }
        ASSERT_TRUE(fixture_.startServer());
        QCoreApplication::processEvents();
        QThread::msleep(100);
    }
    
    void TearDown() override {
        fixture_.stopServer();
        QCoreApplication::processEvents();
    }
    
    IntegrationTestServerFixture fixture_;
    std::unique_ptr<QCoreApplication> app_;
};

TEST_F(MyRPCIntegrationTest, MyRPCRequestResponse) {
    IntegrationTestClient client;
    ASSERT_TRUE(client.connect(fixture_.socketPath()));
    
    QCoreApplication::processEvents();
    QThread::msleep(100);
    QCoreApplication::processEvents();
    
    palantir::MyRequest request;
    // ... populate request ...
    
    palantir::MyResponse response;
    QString error;
    bool success = client.myNewRPC(request, response, error);
    
    ASSERT_TRUE(success) << error.toStdString();
    
    // ... validate response ...
}

#else
#include <gtest/gtest.h>
TEST(MyRPCIntegrationTest, DISABLED_RequiresTransportDeps) {
    GTEST_SKIP() << "Integration tests require BEDROCK_WITH_TRANSPORT_DEPS=ON";
}
#endif
```

### Step 3: Update CMakeLists.txt

In `bedrock/tests/integration/CMakeLists.txt`:

```cmake
add_executable(integration_tests
    integration_main.cpp
    IntegrationTestServerFixture.cpp
    IntegrationTestClient.cpp
    CapabilitiesIntegrationTest.cpp
    XYSineIntegrationTest.cpp
    MyRPCIntegrationTest.cpp  # Add this line
)
```

### Step 4: Run Test

```bash
cd build
./tests/integration/integration_tests --gtest_filter=MyRPCIntegrationTest.*
```

---

## Typical Invocation Examples

### Running All Integration Tests

```bash
cd bedrock/build
./tests/integration/integration_tests
```

### Running Specific Test

```bash
./tests/integration/integration_tests --gtest_filter=CapabilitiesIntegrationTest.*
./tests/integration/integration_tests --gtest_filter=XYSineIntegrationTest.*
```

### Running with Verbose Output

```bash
./tests/integration/integration_tests --gtest_filter=*.* --gtest_color=yes
```

### Running in CI

```bash
ctest -R CapabilitiesIntegrationTest
ctest -R XYSineIntegrationTest
```

---

## Event Loop Considerations

### Why Event Loop Management Matters

Qt's `QLocalSocket` uses signals and slots for asynchronous I/O. The `readyRead()` signal is queued and only fires when the event loop runs. In tests, we must ensure the event loop processes events.

### Best Practices

1. **After Connection:**
   ```cpp
   QCoreApplication::processEvents();
   QThread::msleep(100);  // Give server time to register client
   QCoreApplication::processEvents();
   ```

2. **In Client Receive Loop:**
   ```cpp
   while (elapsed < timeoutMs && socket_->bytesAvailable() < 4) {
       QTest::qWait(10);  // Spins event loop for 10ms
       elapsed += 10;
   }
   ```

3. **After Sending:**
   - `flush()` ensures data is queued
   - No need to wait for `bytesWritten()` signal (write() already validates acceptance)

---

## Troubleshooting

### Test Hangs

- **Symptom:** Test hangs waiting for response
- **Cause:** Event loop not processing `readyRead()` signals
- **Fix:** Ensure `QTest::qWait()` is used in receive loops, not `QThread::msleep()`

### Client Timeout

- **Symptom:** Client times out before server processes request
- **Cause:** Server not processing `onClientReadyRead()` in time
- **Fix:** Add `QCoreApplication::processEvents()` after connection setup

### Server Not Starting

- **Symptom:** `startServer()` returns false
- **Cause:** Socket path conflict or permissions issue
- **Fix:** Check socket path is unique (UUID-based paths should be unique)

---

## Related Documents

- `WS3_XYSineIntegration.md` - XY Sine RPC integration test details
- `WS1_IMPLEMENTATION_PLAN.md` - Envelope transport implementation
- `ADR-0002-Envelope-Based-Palantir-Framing.md` - Protocol architecture decision

---

## Status

✅ Integration test harness complete and validated with:
- Capabilities RPC integration test
- XY Sine RPC integration test
- Both tests passing end-to-end
- No deadlocks or hangs observed

