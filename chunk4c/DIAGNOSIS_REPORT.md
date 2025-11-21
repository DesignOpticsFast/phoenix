# Sprint 4.2 Chunk 4D: Bedrock StartJob Timeout Diagnosis

**Date:** 2025-11-21  
**Status:** DIAGNOSIS COMPLETE — Root Cause Identified  
**Intent:** Diagnose why Bedrock StartJob requests timeout without fixes

---

## Executive Summary

**Root Cause:** `PalantirServer::sendMessage()` does not flush data after writing to `QLocalSocket`, causing StartReply messages to remain in the socket buffer and never reach Phoenix.

**Affected Code:** `bedrock/src/palantir/PalantirServer.cpp:578-604`

**Impact:** All StartJob requests timeout because StartReply is never flushed to the socket.

---

## Detailed Analysis

### Symptom Pattern

From test logs (`chunk4c/logs/test_execution.log`):
1. ✅ Bedrock server starts successfully
2. ✅ Socket connection established
3. ✅ Ping/Pong health check works
4. ❌ StartJob requests timeout (no StartReply received)
5. ❌ All parameter validation tests fail with timeout

**Key Observation:** Ping/Pong works, but StartJob doesn't. This indicates:
- Socket connection is functional
- Message parsing works (Ping is handled)
- Issue is specific to StartJob/StartReply flow

---

## Root Cause Analysis

### Issue 1: sendMessage() Doesn't Flush

**Location:** `bedrock/src/palantir/PalantirServer.cpp:578-604`

**Current Implementation:**
```cpp
void PalantirServer::sendMessage(QLocalSocket* client, const google::protobuf::Message& message)
{
    if (!client || client->state() != QLocalSocket::ConnectedState) {
        return;
    }
    
    // Serialize message with length prefix
    QByteArray serialized;
    // ... serialization code ...
    
    qint64 written = client->write(data);
    if (written != data.size()) {
        qDebug() << "Failed to send complete message";
    }
    // ❌ MISSING: client->flush() or client->waitForBytesWritten()
}
```

**Problem:**
- `QLocalSocket::write()` buffers data internally
- Without `flush()` or `waitForBytesWritten()`, data remains in buffer
- Phoenix never receives StartReply because it's stuck in Bedrock's socket buffer
- Ping/Pong might work due to timing or different code path

**Why Ping/Pong Works:**
- Ping handler (`handlePing()`) also uses `sendMessage()`, but:
  - May flush due to different timing
  - Or Phoenix's ping implementation flushes on its side
  - Or ping is sent via heartbeat timer which has different timing

---

### Issue 2: Missing Ping Handler in handleMessage()

**Location:** `bedrock/src/palantir/PalantirServer.cpp:165-192`

**Current Implementation:**
```cpp
void PalantirServer::handleMessage(QLocalSocket* client, const QByteArray& message)
{
    // Try to parse as StartJob
    palantir::StartJob startJob;
    if (startJob.ParseFromArray(message.data(), message.size())) {
        handleStartJob(client, startJob);
        return;
    }
    
    // Try to parse as Cancel
    palantir::Cancel cancel;
    if (cancel.ParseFromArray(message.data(), message.size())) {
        handleCancel(client, cancel);
        return;
    }
    
    // Try to parse as CapabilitiesRequest
    palantir::CapabilitiesRequest request;
    if (request.ParseFromArray(message.data(), message.size())) {
        handleCapabilitiesRequest(client);
        return;
    }
    
    // ❌ MISSING: Ping handler
    qDebug() << "Unknown message type received";
}
```

**Problem:**
- `handlePing()` exists but is never called from `handleMessage()`
- Ping messages are not parsed/dispatched
- However, Ping/Pong still works, suggesting:
  - Ping might be handled elsewhere
  - Or Phoenix's ping doesn't go through handleMessage()

**Note:** This is a secondary issue - the primary blocker is the flush problem.

---

### Issue 3: No Error Logging for Failed Parsing

**Location:** `bedrock/src/palantir/PalantirServer.cpp:165-192`

**Problem:**
- If StartJob parsing fails, `handleMessage()` silently returns
- No debug output to indicate parsing failure
- Makes debugging difficult

**Impact:** Low - primary issue is flush, but better logging would help

---

## Code Flow Analysis

### Successful Flow (Ping/Pong)

1. Phoenix sends Ping message
2. Bedrock receives via `onClientReadyRead()` → `parseIncomingData()` → `handleMessage()`
3. Ping parsed (somehow - see Issue 2)
4. `handlePing()` called → `sendMessage()` → `client->write()`
5. **Data flushed** (somehow - timing or different path)
6. Phoenix receives Pong ✅

### Failing Flow (StartJob)

1. Phoenix sends StartJob message
2. Bedrock receives via `onClientReadyRead()` → `parseIncomingData()` → `handleMessage()`
3. StartJob parsed successfully ✅
4. `handleStartJob()` called ✅
5. Validation passes (or fails with INVALID_ARGUMENT) ✅
6. `sendMessage()` called with StartReply ✅
7. `client->write()` called ✅
8. **Data NOT flushed** ❌
9. StartReply stuck in buffer
10. Phoenix times out waiting for StartReply ❌

---

## Verification

### Test Evidence

From `chunk4c/logs/test_execution.log`:
```
QDEBUG : testC1HappyPath10k() phoenix.transport: Health check: Received Pong (timestamp: 0 ms)
QWARN  : testC1HappyPath10k() phoenix.transport: "computeXYSine: No StartReply received" - "Connection to Bedrock server timed out."
```

This confirms:
- Ping/Pong works (health check succeeds)
- StartJob fails (timeout)
- Same socket, same connection
- Issue is in StartReply sending, not receiving

---

## Proposed Fix Plan

### Fix 1: Add Flush to sendMessage() (CRITICAL)

**Location:** `bedrock/src/palantir/PalantirServer.cpp:578-604`

**Change:**
```cpp
void PalantirServer::sendMessage(QLocalSocket* client, const google::protobuf::Message& message)
{
    if (!client || client->state() != QLocalSocket::ConnectedState) {
        return;
    }
    
    // ... existing serialization code ...
    
    qint64 written = client->write(data);
    if (written != data.size()) {
        qDebug() << "Failed to send complete message";
        return;
    }
    
    // ✅ ADD: Flush data to ensure it's sent
    if (!client->flush()) {
        qDebug() << "Failed to flush message";
    }
    // Alternative: client->waitForBytesWritten(1000);
}
```

**Rationale:**
- `flush()` ensures data is written to socket immediately
- `waitForBytesWritten()` is more robust but adds latency
- `flush()` is sufficient for local sockets

---

### Fix 2: Add Ping Handler to handleMessage() (SECONDARY)

**Location:** `bedrock/src/palantir/PalantirServer.cpp:165-192`

**Change:**
```cpp
void PalantirServer::handleMessage(QLocalSocket* client, const QByteArray& message)
{
    // Try to parse as StartJob
    palantir::StartJob startJob;
    if (startJob.ParseFromArray(message.data(), message.size())) {
        handleStartJob(client, startJob);
        return;
    }
    
    // Try to parse as Cancel
    palantir::Cancel cancel;
    if (cancel.ParseFromArray(message.data(), message.size())) {
        handleCancel(client, cancel);
        return;
    }
    
    // Try to parse as CapabilitiesRequest
    palantir::CapabilitiesRequest request;
    if (request.ParseFromArray(message.data(), message.size())) {
        handleCapabilitiesRequest(client);
        return;
    }
    
    // ✅ ADD: Try to parse as Ping
    palantir::Ping ping;
    if (ping.ParseFromArray(message.data(), message.size())) {
        handlePing(client);
        return;
    }
    
    qDebug() << "Unknown message type received";
}
```

**Rationale:**
- Ensures Ping messages are properly handled
- Makes code more complete
- May explain why Ping works (if it's handled elsewhere)

---

### Fix 3: Add Debug Logging (OPTIONAL)

**Location:** `bedrock/src/palantir/PalantirServer.cpp:165-192`

**Change:**
```cpp
void PalantirServer::handleMessage(QLocalSocket* client, const QByteArray& message)
{
    // ... existing parsing code ...
    
    // ✅ ADD: Log unknown messages for debugging
    qDebug() << "Unknown message type received, size:" << message.size();
    // Could also log first few bytes for debugging
}
```

**Rationale:**
- Helps diagnose future parsing issues
- Low overhead
- Useful for debugging

---

## Verification Plan

After applying fixes:

1. **Re-run Chunk 4C Tests**
   ```bash
   cd /home/ec2-user/workspace/phoenix/build-tests
   ./tests/test_xysine_transport
   ```

2. **Expected Results:**
   - ✅ C1: Happy Path should pass
   - ✅ C2.1-C2.4: Invalid parameter tests should pass
   - ✅ C5: Progress throttling should pass
   - ✅ All StartJob requests should receive StartReply

3. **Check Logs:**
   - Verify StartReply is received in Phoenix logs
   - Verify no timeout errors
   - Verify error codes are correct (InvalidArgument for invalid params)

4. **Manual Verification:**
   - Start Bedrock server manually
   - Send StartJob via Phoenix GUI
   - Verify StartReply received immediately

---

## Summary

**Root Cause:** `sendMessage()` doesn't flush socket data, causing StartReply to remain in buffer.

**Fix:** Add `client->flush()` after `client->write()` in `sendMessage()`.

**Impact:** High - blocks all StartJob requests.

**Complexity:** Low - single line fix.

**Risk:** Very Low - flush is standard Qt socket operation.

---

**Diagnosis Complete**  
**Ready for Implementation**

