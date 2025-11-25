# Workstream 1 – Transport & IPC Correctness
## Implementation Plan

**Status**: ✅ Complete  
**Date**: 2025-01-25  
**Branch**: `sprint/4.5-envelope-migration`  
**Scope**: Envelope-based Palantir protocol implementation

---

## 1. Transport Architecture Summary (Current State)

### 1.1 Phoenix Client (`LocalSocketChannel.cpp`)

**Current Wire Format**: `[4-byte length][payload]`
- **No MessageType byte** - format is simpler than Bedrock expects
- **No envelope** - direct protobuf serialization

**Current Implementation**:
- **File**: `src/transport/LocalSocketChannel.cpp`
- **Functions**:
  - `getCapabilities()`: Only implemented RPC method
  - **Send flow**:
    1. Serialize `CapabilitiesRequest` to string
    2. Create `[4-byte length][serialized request]` frame
    3. Write to `QLocalSocket`
  - **Receive flow**:
    1. Read 4-byte length prefix
    2. Read `length` bytes of payload
    3. Parse `CapabilitiesResponse` directly from payload

**Length Prefixing**:
- 4-byte little-endian `uint32_t`
- No size limit check in Phoenix client (relies on server)
- No timeout handling for partial reads (uses `waitForReadyRead(5000)`)

**Error Handling**:
- Returns `std::nullopt` on any error
- Error messages via `QString* outError` parameter
- No structured error responses (no `ErrorResponse` handling)

**Threading**:
- Single-threaded (Qt event loop)
- No explicit thread-safety concerns (one client instance per connection)

**Missing Features**:
- No XY Sine RPC implementation
- No envelope support
- No version checking
- No metadata support
- No error response handling

---

### 1.2 Bedrock Server (`PalantirServer.cpp`)

**Current Wire Format**: `[4-byte length][1-byte MessageType][payload]`
- **Has MessageType byte** - more advanced than Phoenix client
- **Backward compatibility** - also supports `[4-byte length][payload]` format

**Current Implementation**:
- **File**: `src/palantir/PalantirServer.cpp` (Bedrock repo)
- **Functions**:
  - `readMessageWithType()`: Reads `[length][type][payload]` format
  - `sendMessage()`: Writes `[length][type][payload]` format
  - `readMessage()`: Backward compatibility for `[length][payload]` format
  - `parseIncomingData()`: Main message parsing loop
  - `handleCapabilitiesRequest()`: Capabilities handler
  - `handleXYSineRequest()`: XY Sine handler

**Length Prefixing**:
- 4-byte little-endian `uint32_t`
- **Size limit check**: `MAX_MESSAGE_SIZE = 10MB` (checked before reading)
- Thread-safe buffer management with `clientBuffersMutex_`

**Error Handling**:
- `sendErrorResponse()`: Sends structured `ErrorResponse` with `ErrorCode`
- Error codes: `MESSAGE_TOO_LARGE`, `UNKNOWN_MESSAGE_TYPE`, `PROTOBUF_PARSE_ERROR`, etc.
- Errors sent as `ERROR_RESPONSE` message type

**Threading**:
- **Thread-safe client buffers**: `clientBuffers_` protected by `clientBuffersMutex_`
- **Job threads**: Separate threads for compute jobs (protected by `jobMutex_`)
- **Qt signals/slots**: Connection/disconnection handled via Qt event system
- **Concurrent clients**: Multiple clients can connect simultaneously

**Current Message Flow**:
1. Client connects → `onNewConnection()` → buffer initialized
2. Data arrives → `onClientReadyRead()` → `parseIncomingData()`
3. `parseIncomingData()`:
   - Tries new format: `readMessageWithType()` → extract type + payload
   - Falls back to old format: `readMessage()` → try parsing as XYSineRequest or CapabilitiesRequest
4. Dispatch by `MessageType` → handler function
5. Handler processes → `sendMessage()` → `[length][type][payload]` format

**Backward Compatibility**:
- Currently supports both formats (new format preferred, old format fallback)
- Deprecation warning logged on first old-format message
- **This backward compatibility will be removed in Workstream 1**

---

### 1.3 Key Differences (Current State)

| Aspect | Phoenix Client | Bedrock Server |
|--------|---------------|---------------|
| **Wire Format** | `[length][payload]` | `[length][type][payload]` |
| **MessageType** | Not used | Used (1-byte enum) |
| **Size Limits** | Not checked | Checked (10MB) |
| **Error Responses** | Not handled | Handled (`ErrorResponse`) |
| **Thread Safety** | Single-threaded | Multi-threaded (mutexes) |
| **Backward Compat** | N/A | Yes (old format fallback) |

**Problem**: Phoenix and Bedrock use **incompatible wire formats** currently.

---

## 2. Target Behavior Summary (From Control Doc & ADR)

### 2.1 Wire Format (Target)

**Format**: `[4-byte length][serialized MessageEnvelope]`

Where:
- **Length**: 4-byte little-endian `uint32_t` (size of serialized `MessageEnvelope`)
- **MessageEnvelope**: Protobuf message containing:
  - `version` (uint32): Protocol version (1 = initial, 0 = invalid)
  - `type` (MessageType enum): Message type identifier
  - `payload` (bytes): Serialized inner message
  - `metadata` (map<string,string>): Optional metadata

### 2.2 Encoding Logic (Client - Phoenix)

**Flow**:
1. Serialize inner message (e.g., `CapabilitiesRequest`) to `std::string`
2. Create `MessageEnvelope`:
   - Set `version = 1`
   - Set `type = CAPABILITIES_REQUEST` (or appropriate enum)
   - Set `payload = serialized_inner_message`
   - Optionally set `metadata` (empty for now, future: tracing IDs)
3. Serialize `MessageEnvelope` to `std::string`
4. Check size: `envelope_size <= MAX_MESSAGE_SIZE` (10MB)
5. Create frame: `[4-byte length][serialized MessageEnvelope]`
6. Write to socket

**Helper Function** (to be created):
```cpp
bool sendEnvelopeMessage(QLocalSocket* socket, 
                         palantir::MessageType type,
                         const google::protobuf::Message& innerMessage,
                         const std::map<std::string, std::string>& metadata,
                         QString* outError);
```

### 2.3 Decoding Logic (Server - Bedrock)

**Flow**:
1. Read 4-byte length prefix (little-endian `uint32_t`)
2. **Validate length**: `length <= MAX_MESSAGE_SIZE` (reject if too large)
3. Read `length` bytes (may require multiple reads)
4. Parse `MessageEnvelope` from bytes
5. **Validate version**: `envelope.version() == 1` (reject if != 1)
6. **Validate type**: `envelope.type()` is valid `MessageType` enum
7. Extract `payload` and `type`
8. Deserialize inner message based on `type`:
   - `CAPABILITIES_REQUEST` → `CapabilitiesRequest`
   - `XY_SINE_REQUEST` → `XYSineRequest`
   - `ERROR_RESPONSE` → reject (server shouldn't receive this)
9. Process request

**Helper Function** (to be created):
```cpp
bool readEnvelopeMessage(QLocalSocket* client,
                         palantir::MessageEnvelope& outEnvelope,
                         QString* outError);
```

### 2.4 Version Validation

- **Client**: Always set `version = 1` when creating envelope
- **Server**: Validate `version == 1`, reject with `ERROR_RESPONSE` if version mismatch
- **Error Code**: `UNSUPPORTED_FEATURE` for version mismatch (future: version negotiation)

### 2.5 Metadata Handling

- **Current**: Metadata is optional, can be empty map
- **Future**: May contain tracing IDs, flags, etc.
- **Validation**: No size limit on metadata map for now (protobuf handles it)
- **Behavior**: Server ignores metadata for now (no processing required)

### 2.6 Error Propagation

**Client (Phoenix)**:
- On envelope send failure: Return `std::nullopt`, set `outError`
- On envelope receive failure: Return `std::nullopt`, set `outError`
- On `ERROR_RESPONSE` received: Extract error code/message, return `std::nullopt`, set `outError`

**Server (Bedrock)**:
- On envelope parse failure: Send `ERROR_RESPONSE` with `PROTOCOL_ERROR`
- On version mismatch: Send `ERROR_RESPONSE` with `UNSUPPORTED_FEATURE`
- On type mismatch: Send `ERROR_RESPONSE` with `UNKNOWN_MESSAGE_TYPE`
- On payload parse failure: Send `ERROR_RESPONSE` with `PROTOBUF_PARSE_ERROR`

### 2.7 Size Limit Enforcement

- **Limit**: `MAX_MESSAGE_SIZE = 10 * 1024 * 1024` (10MB)
- **Client**: Check envelope size before sending (reject if too large)
- **Server**: Check length prefix before reading (reject if too large)
- **Error Code**: `MESSAGE_TOO_LARGE` for oversized messages

### 2.8 Handling of Oversized Messages

- **Client**: Check `envelope.SerializeToString()` size, reject before sending
- **Server**: Check length prefix, reject before reading envelope bytes
- **Behavior**: Send `ERROR_RESPONSE` with `MESSAGE_TOO_LARGE`, close connection (optional)

### 2.9 Handling of Malformed Envelopes

- **Server**: If `MessageEnvelope::ParseFromArray()` fails:
  - Send `ERROR_RESPONSE` with `PROTOCOL_ERROR`
  - Clear client buffer to prevent further parsing
  - Continue listening for next message (don't disconnect)

### 2.10 Handling of Unsupported MessageType

- **Server**: If `envelope.type()` is not in expected set:
  - Send `ERROR_RESPONSE` with `UNKNOWN_MESSAGE_TYPE`
  - Continue listening (don't disconnect)

---

## 3. Complete Change Plan

### 3.1 Phoenix Client Changes (`LocalSocketChannel.cpp`)

#### Files to Modify:
- `src/transport/LocalSocketChannel.cpp`
- `src/transport/LocalSocketChannel.hpp` (if needed for new helper functions)

#### Functions to Modify:

**1. `getCapabilities()` - REPLACE**
- **Current**: Direct `[length][payload]` format
- **New**: Use envelope-based format
- **Changes**:
  - Create `MessageEnvelope` with `version=1`, `type=CAPABILITIES_REQUEST`
  - Serialize envelope, prefix with length
  - Read envelope response, extract `CapabilitiesResponse` from payload
  - Handle `ERROR_RESPONSE` type

**2. New Helper Function: `sendEnvelopeMessage()` - ADD**
- **Purpose**: Encode and send envelope-based message
- **Signature**:
  ```cpp
  static bool sendEnvelopeMessage(QLocalSocket* socket,
                                   palantir::MessageType type,
                                   const google::protobuf::Message& innerMessage,
                                   const std::map<std::string, std::string>& metadata,
                                   QString* outError);
  ```
- **Logic**:
  1. Serialize inner message
  2. Create `MessageEnvelope`, set fields
  3. Serialize envelope
  4. Check size limit
  5. Write `[length][serialized envelope]`

**3. New Helper Function: `readEnvelopeMessage()` - ADD**
- **Purpose**: Read and parse envelope-based message
- **Signature**:
  ```cpp
  static bool readEnvelopeMessage(QLocalSocket* socket,
                                  palantir::MessageEnvelope& outEnvelope,
                                  QString* outError);
  ```
- **Logic**:
  1. Read 4-byte length
  2. Validate size limit
  3. Read envelope bytes
  4. Parse `MessageEnvelope`
  5. Validate version == 1

**4. New Helper Function: `extractPayload()` - ADD**
- **Purpose**: Extract inner message from envelope based on type
- **Signature**:
  ```cpp
  template<typename MessageType>
  static bool extractPayload(const palantir::MessageEnvelope& envelope,
                              MessageType& outMessage,
                              QString* outError);
  ```
- **Logic**:
  1. Check envelope type matches expected type
  2. Parse inner message from `envelope.payload()`
  3. Return success/failure

#### Includes to Add:
```cpp
#include "palantir/envelope.pb.h"
#include "palantir/error.pb.h"
```

#### Constants to Add:
```cpp
static constexpr uint32_t MAX_MESSAGE_SIZE = 10 * 1024 * 1024; // 10MB
static constexpr uint32_t PROTOCOL_VERSION = 1;
```

#### Legacy Code to Remove:
- None (current code doesn't use legacy format, just simple format)

---

### 3.2 Bedrock Server Changes (`PalantirServer.cpp`)

#### Files to Modify:
- `src/palantir/PalantirServer.cpp` (Bedrock repo)
- `src/palantir/PalantirServer.hpp` (if needed for new helper functions)

#### Functions to Modify:

**1. `sendMessage()` - REPLACE**
- **Current**: `[length][type][payload]` format
- **New**: `[length][serialized MessageEnvelope]` format
- **Changes**:
  - Create `MessageEnvelope` with `version=1`, `type`, `payload`
  - Serialize envelope, prefix with length
  - Remove MessageType byte from wire format

**2. `readMessageWithType()` - REPLACE**
- **Current**: Reads `[length][type][payload]` format
- **New**: Reads `[length][serialized MessageEnvelope]` format
- **Rename**: `readEnvelopeMessage()` (more descriptive)
- **Changes**:
  - Read length prefix
  - Read envelope bytes
  - Parse `MessageEnvelope`
  - Validate version == 1
  - Extract type and payload from envelope

**3. `parseIncomingData()` - MODIFY**
- **Current**: Tries new format, falls back to old format
- **New**: Only uses envelope format (remove backward compatibility)
- **Changes**:
  - Remove `readMessage()` fallback
  - Remove deprecation warning
  - Only call `readEnvelopeMessage()`
  - Dispatch by `envelope.type()`

**4. `handleMessage()` - REMOVE or SIMPLIFY**
- **Current**: Handles old format fallback
- **New**: Not needed (envelope-only format)
- **Action**: Remove or simplify (may be unused after changes)

**5. `readMessage()` - REMOVE**
- **Current**: Backward compatibility for `[length][payload]` format
- **New**: Not needed (envelope-only format)
- **Action**: Remove entirely

**6. New Helper Function: `createEnvelope()` - ADD**
- **Purpose**: Create `MessageEnvelope` from inner message
- **Signature**:
  ```cpp
  static palantir::MessageEnvelope createEnvelope(
      palantir::MessageType type,
      const google::protobuf::Message& innerMessage,
      const std::map<std::string, std::string>& metadata = {});
  ```
- **Logic**:
  1. Serialize inner message
  2. Create envelope, set version=1, type, payload, metadata
  3. Return envelope

#### Includes to Verify:
- `palantir/envelope.pb.h` (already included)
- `palantir/error.pb.h` (already included)

#### Constants to Verify:
- `MAX_MESSAGE_SIZE = 10 * 1024 * 1024` (already defined)
- Add: `PROTOCOL_VERSION = 1`

#### Legacy Code to Remove:
- **`readMessage()` function**: Entire function (lines ~682-718)
- **Backward compatibility in `parseIncomingData()`**: Remove old format fallback (lines ~785-793)
- **Deprecation warning**: Remove (lines ~232-237)
- **`handleMessage()` old format logic**: Remove (lines ~230-255)

---

### 3.3 Error Code Mapping

**Error Code Usage** (from `error.proto`):
- **Oversized message**: `MESSAGE_TOO_LARGE` (exists)
- **Malformed envelope**: `INVALID_MESSAGE_FORMAT` (exists)
- **Version mismatch**: `INVALID_MESSAGE_FORMAT` (use this, `UNSUPPORTED_FEATURE` doesn't exist)
- **Unknown type**: `UNKNOWN_MESSAGE_TYPE` (exists)
- **Payload parse failure**: `PROTOBUF_PARSE_ERROR` (exists)

---

### 3.4 Structure of New Envelope Read/Write Path

#### Client (Phoenix) - Send Path:
```
getCapabilities()
  → sendEnvelopeMessage()
    → Serialize CapabilitiesRequest
    → Create MessageEnvelope (version=1, type=CAPABILITIES_REQUEST, payload=...)
    → Serialize MessageEnvelope
    → Check size <= MAX_MESSAGE_SIZE
    → Write [4-byte length][serialized envelope]
```

#### Client (Phoenix) - Receive Path:
```
getCapabilities()
  → readEnvelopeMessage()
    → Read 4-byte length
    → Check length <= MAX_MESSAGE_SIZE
    → Read length bytes
    → Parse MessageEnvelope
    → Validate version == 1
  → extractPayload<CapabilitiesResponse>()
    → Check envelope.type() == CAPABILITIES_RESPONSE
    → Parse CapabilitiesResponse from envelope.payload()
  → Return CapabilitiesResponse
```

#### Server (Bedrock) - Receive Path:
```
onClientReadyRead()
  → parseIncomingData()
    → readEnvelopeMessage()
      → Read 4-byte length
      → Check length <= MAX_MESSAGE_SIZE
      → Read length bytes
      → Parse MessageEnvelope
      → Validate version == 1
    → Dispatch by envelope.type()
      → CAPABILITIES_REQUEST → handleCapabilitiesRequest()
      → XY_SINE_REQUEST → handleXYSineRequest()
      → ERROR_RESPONSE → reject (unexpected)
      → default → sendErrorResponse(UNKNOWN_MESSAGE_TYPE)
```

#### Server (Bedrock) - Send Path:
```
handleCapabilitiesRequest()
  → createEnvelope(CAPABILITIES_RESPONSE, response)
  → sendMessage()
    → Serialize MessageEnvelope
    → Check size <= MAX_MESSAGE_SIZE
    → Write [4-byte length][serialized envelope]
```

---

### 3.5 Temporary Instrumentation Logs

**Optional (for debugging)**:
- Log envelope size before sending (client)
- Log envelope version/type on receive (server)
- Log metadata keys (if non-empty)
- Log error responses sent/received

**Recommendation**: Add minimal logging for envelope version/type, remove after testing.

---

## 4. Error & Edge Case Matrix

| Error Case | Client (Phoenix) | Server (Bedrock) | Error Code |
|-----------|------------------|------------------|------------|
| **Oversized message** | Check envelope size before send, reject with `outError` | Check length prefix, send `ERROR_RESPONSE`, clear buffer | `MESSAGE_TOO_LARGE` |
| **Negative size** | Not possible (uint32_t), but check for 0 | Not possible (uint32_t), but check for 0 | `INVALID_MESSAGE_FORMAT` |
| **Zero-length payload** | Valid (empty message), allow | Valid (empty message), allow | N/A (not an error) |
| **Envelope parse failure** | Return `std::nullopt`, set `outError` | Send `ERROR_RESPONSE`, clear buffer | `INVALID_MESSAGE_FORMAT` |
| **Unsupported version** | Always send version=1 | Validate version==1, send `ERROR_RESPONSE` | `UNSUPPORTED_FEATURE` (or `INVALID_MESSAGE_FORMAT`) |
| **Unsupported type** | Not applicable (client sends known types) | Send `ERROR_RESPONSE`, continue listening | `UNKNOWN_MESSAGE_TYPE` |
| **Payload parse failure** | Return `std::nullopt`, set `outError` | Send `ERROR_RESPONSE` | `PROTOBUF_PARSE_ERROR` |
| **Server disconnect mid-RPC** | Detect via socket state, return `std::nullopt` | N/A (server side) | `CONNECTION_CLOSED` |
| **Client disconnect mid-RPC** | N/A (client side) | Detect via socket state, cleanup buffer | N/A (cleanup only) |
| **Timeout reading length** | `waitForReadyRead()` timeout, return `std::nullopt` | N/A (server uses event-driven reads) | `CONNECTION_TIMEOUT` |
| **Timeout reading envelope** | `waitForReadyRead()` timeout, return `std::nullopt` | N/A (server uses event-driven reads) | `CONNECTION_TIMEOUT` |
| **Metadata ignored** | N/A (client sets metadata) | Ignore metadata (no processing) | N/A (not an error) |
| **Invalid MessageType enum value** | Not possible (type-safe enum) | Check enum validity, send `ERROR_RESPONSE` | `UNKNOWN_MESSAGE_TYPE` |
| **Partial length read** | `waitForReadyRead()` handles this | Buffer accumulates data | N/A (handled by buffering) |
| **Partial envelope read** | `waitForReadyRead()` handles this | Buffer accumulates data | N/A (handled by buffering) |

---

## 5. Security & Safety Requirements

### 5.1 Length Validation

- **Before allocation**: Check `length <= MAX_MESSAGE_SIZE` before reading envelope bytes
- **Integer overflow**: Use `uint32_t` (max 4GB), but enforce `MAX_MESSAGE_SIZE = 10MB` limit
- **Zero length**: Reject length == 0 (invalid envelope)
- **Implementation**: Check length prefix immediately after reading, before buffer allocation

### 5.2 Envelope Size Validation

- **Before parsing**: Check serialized envelope size <= `MAX_MESSAGE_SIZE`
- **After parsing**: Validate envelope fields (version, type) before processing
- **Implementation**: Check length prefix on both client and server

### 5.3 MessageType Enum Validation

- **Client**: Use type-safe enum (compile-time check)
- **Server**: Validate enum value is in expected set (runtime check)
- **Invalid values**: Reject with `UNKNOWN_MESSAGE_TYPE`
- **Implementation**: Use `switch` statement with `default` case

### 5.4 Metadata Size Limit

- **Current**: No explicit limit (protobuf handles it)
- **Future consideration**: May need to limit map size (e.g., max 100 entries, max 1KB total)
- **For now**: Trust protobuf serialization limits
- **Recommendation**: Add metadata size check in future if abuse is observed

### 5.5 Thread Safety

**Phoenix (Client)**:
- Single-threaded (Qt event loop)
- No explicit locking needed (one client instance per connection)

**Bedrock (Server)**:
- **Client buffers**: Protected by `clientBuffersMutex_` (already in place)
- **Job tracking**: Protected by `jobMutex_` (already in place)
- **Envelope parsing**: Must hold `clientBuffersMutex_` when reading from buffer
- **Envelope sending**: No locking needed (Qt signals/slots are thread-safe for `write()`)

### 5.6 Buffer Management

- **Client**: Read into temporary buffer, parse, discard
- **Server**: Accumulate in `clientBuffers_`, parse, remove processed bytes
- **Buffer overflow**: Prevented by `MAX_MESSAGE_SIZE` limit
- **Memory safety**: Use `QByteArray` (Qt-managed memory)

### 5.7 Version Validation

- **Client**: Always set `version = 1` (hardcoded)
- **Server**: Validate `version == 1`, reject others
- **Future**: Version negotiation (out of scope for Workstream 1)

---

## 6. Open Questions

### 6.1 Metadata Size Limit

**Question**: Should we enforce a metadata size limit now, or defer to future?

**Options**:
- **A**: No limit now (trust protobuf, add later if needed)
- **B**: Add limit now (e.g., max 100 entries, max 1KB total)

**Recommendation**: **A** (no limit now, add later if abuse observed)

**Decision needed**: Confirm approach

---

### 6.2 Version Mismatch Behavior

**Question**: Should version mismatch trigger server disconnect, or just error response?

**Options**:
- **A**: Send `ERROR_RESPONSE`, continue listening (allow retry)
- **B**: Send `ERROR_RESPONSE`, disconnect client (force reconnect)

**Recommendation**: **A** (continue listening, allow client to retry with correct version)

**Decision needed**: Confirm approach

---

### 6.3 Invalid Envelope Behavior

**Question**: Should invalid envelopes trigger server disconnect, or just error response + buffer clear?

**Options**:
- **A**: Send `ERROR_RESPONSE`, clear buffer, continue listening
- **B**: Send `ERROR_RESPONSE`, disconnect client

**Recommendation**: **A** (clear buffer, continue listening - more resilient)

**Decision needed**: Confirm approach

---

### 6.4 Tracing IDs in Metadata

**Question**: Should we introduce tracing IDs in metadata now, or defer to future?

**Options**:
- **A**: Defer (metadata empty for now)
- **B**: Add basic tracing ID generation now

**Recommendation**: **A** (defer to future sprint, keep Workstream 1 focused)

**Decision needed**: Confirm approach

---

### 6.5 Error Code for Version Mismatch

**Question**: Which error code should we use for version mismatch?

**Answer**: `INVALID_MESSAGE_FORMAT` (confirmed: `UNSUPPORTED_FEATURE` doesn't exist in `error.proto`)

**Decision**: Use `INVALID_MESSAGE_FORMAT` for version mismatches (version is part of message format)

---

### 6.6 XY Sine RPC in Phoenix

**Question**: Should we implement XY Sine RPC in Phoenix client as part of Workstream 1, or defer?

**Options**:
- **A**: Implement XY Sine RPC in Phoenix (complete envelope migration)
- **B**: Defer XY Sine RPC (only migrate Capabilities for now)

**Recommendation**: **A** (implement XY Sine RPC to fully exercise envelope protocol)

**Decision needed**: Confirm scope

---

### 6.7 Unit Tests

**Question**: Should unit tests be written as part of Workstream 1, or deferred to Workstream 3?

**Per control document**: "Unit tests for envelope framing and error paths" are part of Workstream 1 gate.

**Recommendation**: Write unit tests in Workstream 1 (required for gate)

**Decision needed**: Confirm test scope and location

---

## 7. Explicit STOP

**This planning document is complete.**

**No code changes have been made.**

**Awaiting approval before proceeding with implementation.**

---

## Summary

### Files to Modify:
1. **Phoenix**: `src/transport/LocalSocketChannel.cpp` (and `.hpp` if needed)
2. **Bedrock**: `src/palantir/PalantirServer.cpp` (and `.hpp` if needed)

### Key Changes:
- Replace `[length][type][payload]` with `[length][serialized MessageEnvelope]`
- Add envelope encoding/decoding helpers
- Remove backward compatibility code
- Add version validation
- Normalize error handling

### Estimated Effort:
- Phoenix changes: ~2-3 hours
- Bedrock changes: ~2-3 hours
- Unit tests: ~2-3 hours
- **Total**: ~6-9 hours

### Risks:
- Breaking change (no backward compatibility)
- Coordination required (Phoenix + Bedrock must be updated together)
- Thread safety (Bedrock server already has mutexes, should be safe)

---

**Ready for review and approval.**

