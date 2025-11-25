# IPC Envelope Protocol

**Version:** 1.0  
**Date:** 2025-01-25  
**Sprint:** 4.5  
**Status:** Active

---

## Overview

The Palantir IPC protocol uses an envelope-based wire format for all client-server communication. This document describes the protocol specification, wire format, validation requirements, and error handling rules.

---

## Wire Format

### Frame Structure

```
[4-byte length][serialized MessageEnvelope]
```

Where:
- **Length**: 4-byte little-endian `uint32_t` (size of serialized `MessageEnvelope` in bytes)
- **MessageEnvelope**: Protobuf-serialized envelope message

### Example

For a CapabilitiesRequest:
```
Length: 0x00000004 (4 bytes)
Envelope: [serialized MessageEnvelope, 4 bytes]
Total: 8 bytes on wire
```

---

## MessageEnvelope Structure

### Protobuf Definition

```protobuf
message MessageEnvelope {
  uint32 version = 1;                    // Protocol version (1 = initial)
  MessageType type = 2;                   // Message type enum
  bytes payload = 3;                      // Serialized inner message
  map<string, string> metadata = 4;       // Optional metadata
}
```

### Field Descriptions

1. **version** (uint32, field 1)
   - **Current Value:** `1`
   - **Reserved:** `0` (invalid/reserved)
   - **Validation:** Must be `1` on receive, must set to `1` on send
   - **Future:** Version negotiation will use this field

2. **type** (MessageType enum, field 2)
   - **Values:** `CAPABILITIES_REQUEST`, `CAPABILITIES_RESPONSE`, `XY_SINE_REQUEST`, `XY_SINE_RESPONSE`, `ERROR_RESPONSE`
   - **Validation:** Must be valid enum value
   - **Usage:** Determines which handler processes the payload

3. **payload** (bytes, field 3)
   - **Content:** Serialized inner protobuf message (e.g., `CapabilitiesRequest`, `XYSineRequest`)
   - **Size:** Variable (0 bytes for empty messages, up to `MAX_MESSAGE_SIZE`)
   - **Validation:** Must parse as the message type indicated by `type` field

4. **metadata** (map<string, string>, field 4)
   - **Content:** Optional key-value pairs for tracing, flags, extensibility
   - **Current Usage:** Empty in most cases
   - **Future:** Can be used for request tracing, feature flags, etc.

---

## Size Constraints

### Maximum Message Size

- **Server Limit:** `MAX_MESSAGE_SIZE = 10 * 1024 * 1024` (10 MB)
- **Client Limit:** Should respect server limit
- **Envelope Overhead:** ~10-20 bytes per message (version + type + metadata)

### Minimum Message Size

- **Theoretical Minimum:** ~4 bytes (empty payload, minimal envelope)
- **Practical Minimum:** ~4-6 bytes for valid envelopes
- **Validation:** Do not reject small envelopes (empty `CapabilitiesRequest` is valid at 4 bytes)

---

## Required Validation Steps

### Client-Side (Sending)

1. **Create Inner Message:**
   ```cpp
   palantir::CapabilitiesRequest request;
   // ... populate request ...
   ```

2. **Create Envelope:**
   ```cpp
   auto envelope = phoenix::transport::makeEnvelope(
       palantir::MessageType::CAPABILITIES_REQUEST,
       request,
       {},  // metadata (optional)
       &error
   );
   ```

3. **Validate Envelope:**
   - Check `envelope.has_value()`
   - Verify `envelope->version() == 1`
   - Verify `envelope->type()` matches expected type

4. **Serialize:**
   ```cpp
   std::string serialized;
   envelope->SerializeToString(&serialized);
   ```

5. **Send Frame:**
   ```cpp
   uint32_t length = static_cast<uint32_t>(serialized.size());
   QByteArray data;
   data.append(reinterpret_cast<const char*>(&length), 4);
   data.append(serialized.data(), serialized.size());
   socket->write(data);
   socket->flush();
   ```

### Server-Side (Receiving)

1. **Read Length Prefix:**
   ```cpp
   QByteArray lengthBytes = socket->read(4);
   uint32_t envelopeLength;
   std::memcpy(&envelopeLength, lengthBytes.data(), 4);
   ```

2. **Validate Size:**
   - Check `envelopeLength <= MAX_MESSAGE_SIZE`
   - Check `socket->bytesAvailable() >= envelopeLength`

3. **Read Envelope:**
   ```cpp
   QByteArray envelopeBytes = socket->read(envelopeLength);
   ```

4. **Parse Envelope:**
   ```cpp
   palantir::MessageEnvelope envelope;
   if (!bedrock::palantir::parseEnvelope(
           std::string(envelopeBytes.data(), envelopeBytes.size()),
           envelope,
           &error)) {
       // Handle parse error
   }
   ```

5. **Validate Envelope:**
   - Check `envelope.version() == 1` (reject if not)
   - Check `envelope.type()` is valid enum value
   - Extract payload: `envelope.payload()`

6. **Parse Inner Message:**
   ```cpp
   palantir::CapabilitiesRequest request;
   if (!request.ParseFromArray(
           envelope.payload().data(),
           static_cast<int>(envelope.payload().size()))) {
       // Handle parse error
   }
   ```

---

## Error Handling Rules

### Protocol Errors

1. **Invalid Version:**
   - **Action:** Send `ERROR_RESPONSE` with `INVALID_MESSAGE_FORMAT`
   - **Message:** `"Invalid envelope version: X"`

2. **Invalid Message Type:**
   - **Action:** Send `ERROR_RESPONSE` with `UNKNOWN_MESSAGE_TYPE`
   - **Message:** `"Unknown message type: X"`

3. **Message Too Large:**
   - **Action:** Send `ERROR_RESPONSE` with `MESSAGE_TOO_LARGE`
   - **Message:** `"Envelope length X exceeds limit Y"`

4. **Malformed Envelope:**
   - **Action:** Send `ERROR_RESPONSE` with `INVALID_MESSAGE_FORMAT`
   - **Message:** `"Malformed envelope: [parse error]"`

5. **Payload Parse Error:**
   - **Action:** Send `ERROR_RESPONSE` with `PROTOBUF_PARSE_ERROR`
   - **Message:** `"Failed to parse [MessageType]"`

### Transport Errors

1. **Connection Lost:**
   - **Action:** Clean up client state, log error
   - **No Response:** Cannot send error if connection is lost

2. **Timeout:**
   - **Client:** Return error to caller
   - **Server:** N/A (server doesn't timeout on reads)

3. **Partial Read:**
   - **Action:** Buffer partial data, wait for more
   - **Timeout:** If complete message not received within timeout, close connection

---

## Client/Server Handshake Expectations

### Connection Flow

1. **Client Connects:**
   - Client calls `QLocalSocket::connectToServer(socketPath)`
   - Client waits for `ConnectedState` (timeout: 5 seconds)

2. **Server Accepts:**
   - Server's `onNewConnection()` slot fires
   - Server adds client to `clientBuffers_` map
   - Server connects `readyRead` signal to `onClientReadyRead()`

3. **Ready for Communication:**
   - Client can send requests immediately after connection
   - Server processes requests asynchronously via Qt event loop

### Request/Response Flow

1. **Client Sends Request:**
   - Creates envelope with request payload
   - Sends `[length][envelope]` frame
   - Flushes socket

2. **Server Receives:**
   - `onClientReadyRead()` fires
   - `parseIncomingData()` extracts envelope
   - Dispatches to appropriate handler

3. **Server Sends Response:**
   - Handler creates response message
   - Creates envelope with response payload
   - Sends `[length][envelope]` frame
   - No explicit flush needed (Qt handles it)

4. **Client Receives:**
   - `readyRead()` signal fires (or polling detects data)
   - Client reads length prefix
   - Client reads complete envelope
   - Client parses and validates response

---

## Protocol Version Negotiation (Future)

### Current Behavior

- **Version 1 Only:** All messages must have `version = 1`
- **No Negotiation:** Reject any version != 1

### Future Enhancement

When protocol evolves:
1. Client sends request with `version = 2`
2. Server checks supported versions
3. If server supports v2: respond with `version = 2`
4. If server only supports v1: respond with `ERROR_RESPONSE` indicating supported version
5. Client can retry with compatible version

---

## Message Types

### Request Types

- `CAPABILITIES_REQUEST` - Query server capabilities
- `XY_SINE_REQUEST` - Request XY sine wave computation

### Response Types

- `CAPABILITIES_RESPONSE` - Server capabilities
- `XY_SINE_RESPONSE` - XY sine computation results
- `ERROR_RESPONSE` - Error information

---

## Metadata Usage (Future)

The `metadata` field is currently unused but reserved for:

1. **Request Tracing:**
   - `trace_id`: Unique identifier for request/response pair
   - `span_id`: Span identifier for distributed tracing

2. **Feature Flags:**
   - `feature_flags`: Comma-separated list of enabled features
   - `protocol_features`: Protocol-level feature flags

3. **Debugging:**
   - `debug`: Enable verbose logging for this request
   - `client_version`: Client version string

---

## Implementation References

### Phoenix Client

- **Envelope Helpers:** `src/transport/EnvelopeHelpers.cpp`
- **Transport:** `src/transport/LocalSocketChannel.cpp`
- **Tests:** `tests/envelope_helpers_test.cpp`

### Bedrock Server

- **Envelope Helpers:** `src/palantir/EnvelopeHelpers.cpp`
- **Transport:** `src/palantir/PalantirServer.cpp`
- **Tests:** `tests/palantir/EnvelopeHelpers_test.cpp`
- **Integration Tests:** `tests/integration/`

---

## Related Documents

- `ADR-0002-Envelope-Based-Palantir-Framing.md` - Architectural decision record
- `WS1_IMPLEMENTATION_PLAN.md` - Implementation details
- `WS3_INTEGRATION_HARNESS.md` - Integration test documentation

---

## Status

✅ **Protocol v1 Active**

- Envelope-based transport implemented
- Both Capabilities and XY Sine RPCs validated
- Integration tests passing
- No known protocol-level issues

