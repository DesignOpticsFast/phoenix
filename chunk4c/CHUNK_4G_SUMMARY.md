# Chunk 4G: Message Framing & Buffer Reset Fix - Summary

## Status: IN PROGRESS

### Fixes Applied

1. **Socket Buffer Clearing**: Added code to drain socket buffer before sending StartJob
   - Prevents stale Pong messages from corrupting message alignment
   - Logs show: "Cleared socket buffer, bytesAvailable: 0"

2. **Strict Framed Reads**: Replaced `readAll()` with strict length-prefixed reads
   - Read exactly 4 bytes for message length
   - Read exactly N bytes for message payload
   - Prevents partial frame misalignment

3. **INVALID_ARGUMENT Error Handling**: Fixed to reconnect readyRead handler
   - C2.x tests now receive error messages correctly
   - Error messages are being parsed but test assertions are failing

### Remaining Issues

1. **Protocol Mismatch**: Bedrock sends ResultMeta BEFORE DataChunks, but Phoenix expects it AFTER
   - Bedrock order: StartReply → ResultMeta → DataChunks
   - Phoenix expects: StartReply → Progress/DataChunks → ResultMeta
   - **Fix needed**: Update Phoenix to handle ResultMeta first, then read chunks

2. **Message Parsing**: ResultMeta is being parsed as Progress
   - Logs show: "Progress: 0 % - f64" (f64 is ResultMeta.dtype)
   - Protobuf ParseFromArray() succeeds for wrong message types
   - **Fix needed**: Check message-specific fields (dtype/shape for ResultMeta, progress_pct for Progress)

3. **Connection Closing**: Bedrock closes connection after sending all data
   - Phoenix gets "Remote closed" before reading ResultMeta
   - **Fix needed**: Read ResultMeta immediately after StartReply, before waiting for chunks

### Test Results

- **Passed**: 6 tests (C3, C4, baseline tests)
- **Failed**: 6 tests (C1, C2.1-C2.4, C5)
- **Skipped**: 1 test

### Next Steps

1. Fix message parsing order: Parse ResultMeta FIRST (check for dtype/shape fields)
2. Update protocol handling: Accept ResultMeta before DataChunks
3. Store ResultMeta when received, continue reading chunks
4. Re-run tests

