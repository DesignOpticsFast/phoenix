# Workstream 3 – XY Sine RPC Integration Test

**Date:** 2025-01-25  
**Sprint:** 4.5  
**Workstream:** 3 - Chunk C  
**Status:** ✅ Complete

---

## Overview

The XY Sine integration test validates end-to-end envelope transport for numeric RPCs. It exercises the complete request/response cycle with mathematical correctness validation, ensuring the envelope protocol correctly handles complex protobuf messages with repeated numeric fields.

---

## Test Purpose

This test validates:

1. **Envelope Transport for Numeric RPCs** - Confirms envelope framing works for messages with repeated double fields
2. **Mathematical Correctness** - Validates server computation matches expected sine wave formula
3. **Response Parsing** - Ensures client correctly extracts and validates numeric arrays
4. **End-to-End Flow** - Verifies complete client → server → response → client cycle

---

## Request Format

### XYSineRequest Protobuf

```protobuf
message XYSineRequest {
  double frequency = 1;   // Sine wave frequency (default: 1.0)
  double amplitude = 2;    // Sine wave amplitude (default: 1.0)
  double phase = 3;        // Phase offset in radians (default: 0.0)
  int32 samples = 4;       // Number of sample points (default: 1000, min: 2)
}
```

### Test Request Values

```cpp
palantir::XYSineRequest request;
request.set_frequency(1.0);
request.set_samples(10);
request.set_amplitude(2.0);
request.set_phase(0.0);
```

**Serialized Size:** ~20 bytes (varies with protobuf encoding)

---

## Response Format

### XYSineResponse Protobuf

```protobuf
message XYSineResponse {
  repeated double x = 1;   // X values (domain: 0 to 2π)
  repeated double y = 2;   // Y values (sine wave computation)
  string status = 3;       // Status message (e.g., "OK")
}
```

### Response Structure

- **X Array:** `samples` values from `0` to `2π` (linearly spaced)
- **Y Array:** `samples` values computed as `amplitude * sin(2π * frequency * t + phase)`
- **Status:** "OK" for successful computation

**Serialized Size:** ~175 bytes for 10 samples (varies with values)

---

## Mathematical Correctness Validation

### Server Algorithm

The server computes sine wave using:

```cpp
for (int i = 0; i < samples; ++i) {
    double t = static_cast<double>(i) / (samples - 1.0);  // 0 to 1
    double x = t * 2.0 * M_PI;  // Scale to 0..2π domain
    double y = amplitude * std::sin(2.0 * M_PI * frequency * t + phase);
    
    xValues.push_back(x);
    yValues.push_back(y);
}
```

### Test Validation

The test validates:

1. **Array Sizes:**
   ```cpp
   ASSERT_EQ(response.x_size(), request.samples());
   ASSERT_EQ(response.y_size(), request.samples());
   ```

2. **Y Values (Sine Wave):**
   ```cpp
   for (int i = 0; i < request.samples(); ++i) {
       double t = static_cast<double>(i) / (request.samples() - 1.0);
       double expected = request.amplitude() * 
                        std::sin(2.0 * M_PI * request.frequency() * t + request.phase());
       EXPECT_NEAR(response.y(i), expected, 1e-9);
   }
   ```

3. **X Values (Domain):**
   ```cpp
   for (int i = 0; i < request.samples(); ++i) {
       double t = static_cast<double>(i) / (request.samples() - 1.0);
       double expectedX = t * 2.0 * M_PI;  // 0 to 2π
       EXPECT_NEAR(response.x(i), expectedX, 1e-9);
   }
   ```

**Tolerance:** `1e-9` (nanosecond precision for double comparisons)

---

## Test Output

### Successful Test Run

```
[TEST] Starting XYSineRequestResponse test
[CLIENT] sendEnvelope: type= 3 (XY_SINE_REQUEST), connected= true
[CLIENT] sendEnvelope: envelope payload size= 20
[CLIENT] sendEnvelope: serialized envelope size= 26
[CLIENT] sendEnvelope: SUCCESS - message sent
[SERVER] parseIncomingData: extracted message, type= 3, payload size= 20
[SERVER] handleXYSineRequest: starting
[SERVER] sendMessage: type= 4 (XY_SINE_RESPONSE), serialized size= 175
[SERVER] sendMessage: SUCCESS - message sent
[CLIENT] receiveEnvelope: SUCCESS
[TEST] sendXYSineRequest result: success= true, error= ""
[TEST] XYSineRequestResponse test completed successfully
[       OK ] XYSineIntegrationTest.XYSineRequestResponse (326 ms)
```

### Key Metrics

- **Request Envelope Size:** 26 bytes (4-byte length + 22-byte envelope)
- **Response Envelope Size:** 179 bytes (4-byte length + 175-byte envelope)
- **Test Duration:** ~326ms (includes server computation)
- **No Deadlocks:** Test completes cleanly

---

## How This Validates Envelope Transport

### 1. Complex Message Handling

XY Sine RPC validates that the envelope protocol correctly handles:
- **Repeated Fields:** X and Y arrays with multiple double values
- **Large Payloads:** Response envelope ~175 bytes (larger than Capabilities)
- **Numeric Precision:** Ensures no data corruption in repeated double fields

### 2. End-to-End Validation

The test confirms:
- **Client Serialization:** Request envelope correctly created and sent
- **Server Parsing:** Server correctly extracts request from envelope
- **Server Computation:** Server processes request and generates response
- **Server Serialization:** Response envelope correctly created and sent
- **Client Parsing:** Client correctly extracts response from envelope
- **Data Integrity:** All numeric values match expected computation

### 3. Transport Layer Correctness

This test validates:
- **Envelope Framing:** `[4-byte length][serialized MessageEnvelope]` format
- **Version Validation:** Envelope version = 1 checked on both sides
- **Type Validation:** Message type correctly set and validated
- **Payload Extraction:** Inner protobuf messages correctly extracted
- **Error Handling:** Proper error propagation through envelope layer

---

## Comparison with Capabilities Test

| Aspect | Capabilities | XY Sine |
|--------|-------------|---------|
| **Request Size** | 0 bytes (empty) | ~20 bytes |
| **Response Size** | ~32 bytes | ~175 bytes |
| **Field Types** | String, repeated string | Repeated double |
| **Validation** | Field presence | Mathematical correctness |
| **Complexity** | Simple | Complex (numeric arrays) |

**Key Difference:** XY Sine validates numeric precision and array handling, while Capabilities validates basic envelope transport.

---

## Test Location

- **Test File:** `bedrock/tests/integration/XYSineIntegrationTest.cpp`
- **Client Helper:** `IntegrationTestClient::sendXYSineRequest()`
- **Server Handler:** `PalantirServer::handleXYSineRequest()`

---

## Running the Test

### Standalone

```bash
cd bedrock/build
./tests/integration/integration_tests --gtest_filter=XYSineIntegrationTest.*
```

### Via CTest

```bash
cd bedrock/build
ctest -R XYSineIntegrationTest
```

### With Both Integration Tests

```bash
./tests/integration/integration_tests --gtest_filter="*IntegrationTest.*"
```

---

## Status

✅ XY Sine integration test complete and passing:
- Request/response cycle validated
- Mathematical correctness confirmed
- No deadlocks or hangs
- Envelope transport validated for numeric RPCs

---

## Related Documents

- `WS3_INTEGRATION_HARNESS.md` - Integration test harness overview
- `WS1_IMPLEMENTATION_PLAN.md` - Envelope transport implementation
- `ADR-0002-Envelope-Based-Palantir-Framing.md` - Protocol architecture

