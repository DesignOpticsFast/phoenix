# ADR 0002: Envelope-Based Palantir Framing

**Status:** Accepted  
**Date:** 2025-01-25  
**Context:** Sprint 4.5 Fixup - Phase 0.1

## Context

### Current State

The Palantir transport protocol currently uses a legacy wire format:

```
[4-byte length][1-byte MessageType][payload]
```

Where:
- **Length**: 4-byte little-endian unsigned integer (total message size including type byte)
- **MessageType**: Single byte enum value (0-255)
- **Payload**: Serialized protobuf message (e.g., `CapabilitiesRequest`, `XYSineRequest`)

This format is implemented in `LocalSocketChannel.cpp` via `readMessageWithType()` and `sendMessageWithType()` helper functions.

### Limitations of Current Format

1. **No Protocol Versioning**: Cannot evolve the protocol without breaking compatibility
2. **No Metadata Support**: No way to attach tracing information, flags, or extensibility hints
3. **Type Ambiguity**: Raw byte for message type provides no validation or structure
4. **Debugging Difficulties**: No structured envelope makes logging and troubleshooting harder
5. **Limited Extensibility**: Adding new fields or capabilities requires wire format changes

### Sprint 4.5 Goals

Sprint 4.5 is a hardening sprint focused on:
- **Protocol Clarity**: Unambiguous, versioned wire format
- **Extensibility**: Support for metadata and future protocol evolution
- **Better Debugging**: Structured envelopes for improved logging and diagnostics
- **Security Hardening**: Clear separation of transport vs payload concerns
- **Integration Tests**: Reliable end-to-end verification of IPC correctness

## Decision

Adopt `MessageEnvelope` as the sole on-wire payload format for all Palantir messages.

### Wire Format

```
wire = [4-byte length][serialized MessageEnvelope]
```

Where:
- **Length**: 4-byte little-endian unsigned integer (size of serialized `MessageEnvelope`)
- **MessageEnvelope**: Protobuf message containing:
  - `version` (uint32): Protocol version (1 = initial envelope version, 0 = invalid/reserved)
  - `type` (MessageType enum): Message type identifier
  - `payload` (bytes): Serialized inner message (e.g., `CapabilitiesRequest`, `XYSineRequest`)
  - `metadata` (map<string,string>): Optional metadata for tracing, flags, future extensibility

### MessageEnvelope Definition

```protobuf
message MessageEnvelope {
  uint32 version = 1;                    // Protocol version
  MessageType type = 2;                   // Message type enum
  bytes payload = 3;                      // Serialized inner message
  map<string, string> metadata = 4;       // Optional metadata
}
```

### No Legacy Framing

- **Remove** the `[length][type][payload]` format entirely
- **No runtime backward-compatibility mode**
- **Clean break** from previous protocol

### Migration Strategy

- **Big Bang Cutover**: Implement envelope framing on both Phoenix and Bedrock simultaneously
- **Feature Branch**: All changes on `sprint/4.5-envelope-migration` branch
- **Coordinated Update**: Phoenix + Bedrock updated together and tested together
- **Git-Based Rollback**: Rollback is via git revert, not protocol-based compatibility

## Consequences

### Positive

1. **Versioned Protocol**: Can evolve protocol while maintaining compatibility (future versions can negotiate)
2. **Extensible Metadata**: Support for tracing, flags, and future features without wire format changes
3. **Easier Debugging**: Structured envelope enables better logging, error reporting, and diagnostics
4. **Clear Separation**: Transport concerns (length prefixing) vs payload concerns (envelope structure)
5. **Type Safety**: Enum-based type field instead of raw byte provides validation
6. **Future-Proof**: Can add fields to `MessageEnvelope` without breaking wire format (protobuf backward compatibility)

### Negative / Tradeoffs

1. **Slight Overhead**: ~10-20 bytes per message for envelope structure (acceptable for local IPC)
2. **Breaking Change**: Cannot communicate with pre-4.5 Phoenix/Bedrock instances
3. **Migration Effort**: Requires rewriting read/write functions on both Phoenix and Bedrock sides
4. **Field Renumbering**: `MessageEnvelope` field numbers changed (type: 1→2, payload: 2→3, added version=1, metadata=4) - this is an intentional breaking change accepted for Sprint 4.5
5. **Coordination Required**: Phoenix and Bedrock must be updated in lockstep

### Field Renumbering Note

The `MessageEnvelope` proto definition has been updated with new field numbers:
- `version = 1` (new)
- `type = 2` (was 1)
- `payload = 3` (was 2)
- `metadata = 4` (new)

This is a **breaking change** that is acceptable because:
- The protocol is still in active development
- No external installed base exists
- Sprint 4.5 explicitly chose a big-bang, non-backwards-compatible migration

## Implementation Notes

### Length Prefixing

- Occurs in Palantir read/write functions:
  - **Phoenix**: `LocalSocketChannel` (to be updated in Workstream 1)
  - **Bedrock**: `PalantirServer` (to be updated in Workstream 1)
- 4-byte little-endian encoding/decoding
- Size limit enforcement: 10MB `MAX_MESSAGE_SIZE`

### Envelope Construction/Parsing

**Client (Phoenix) Flow**:
1. Serialize inner message (e.g., `CapabilitiesRequest`)
2. Create `MessageEnvelope`:
   - Set `version = 1`
   - Set `type = CAPABILITIES_REQUEST`
   - Set `payload = serialized_inner_message`
   - Optionally set `metadata` (tracing, flags)
3. Serialize `MessageEnvelope` to bytes
4. Prefix with 4-byte length
5. Send over socket

**Server (Bedrock) Flow**:
1. Read 4-byte length prefix
2. Read `MessageEnvelope` bytes (up to length)
3. Parse `MessageEnvelope` protobuf
4. Validate `version == 1` (reject unknown versions with `ERROR_RESPONSE`)
5. Extract `type` and `payload`
6. Deserialize inner message based on `type`
7. Process request

### Error Handling and Size Limits

- **Oversized Messages**: Reject before parsing envelope (check length prefix against `MAX_MESSAGE_SIZE`)
- **Malformed Envelope**: Return `ERROR_RESPONSE` with `PROTOCOL_ERROR` code
- **Version Mismatch**: Return `ERROR_RESPONSE` with `UNSUPPORTED_FEATURE` code (for future version negotiation)
- **Invalid Type**: Return `ERROR_RESPONSE` with `PROTOCOL_ERROR` code

## Migration Strategy

### Feature Branch

- **Branch Name**: `sprint/4.5-envelope-migration`
- **Base**: `main` branch
- **Scope**: All envelope-related changes (proto, ADR, transport code, tests)

### Testing Expectations

- **Unit Tests**: Envelope encode/decode, error paths, size limit enforcement
- **Integration Tests**: Capabilities + XY Sine + error cases (Workstream 3)
- **CI**: Full test suite on feature branch before merge

### No Runtime Backward Compatibility

- Old `[length][type][payload]` format support removed entirely
- Cannot communicate with pre-4.5 Phoenix/Bedrock instances
- Rollback is git-based (revert feature branch merge), not protocol-based

## Future Work / Non-Goals

### Out of Scope for This ADR

- **Arrow Flight / gRPC Transport**: Future transport layer (separate sprint)
- **Stats Sidecar / Backpressure**: Future observability features (separate sprint)
- **Protocol Version Negotiation**: Future enhancement if needed (currently version=1 is fixed)
- **Compression**: Future optimization (not needed for local IPC)

### Future Enhancements (Post-Sprint 4.5)

- Protocol version negotiation (if version > 1 is needed)
- Structured metadata schemas (beyond string map)
- Message fragmentation for very large payloads
- Connection pooling and keep-alive (transport layer improvements)

## References

- `docs/sprint4.5/FIXUP_SPRINT_CONTROL.md` - Sprint 4.5 control document
- `contracts/proto/palantir/envelope.proto` - MessageEnvelope definition
- `src/transport/LocalSocketChannel.cpp` - Current transport implementation (to be updated)
- `docs/architecture/transport_protocol.md` - Transport protocol documentation (to be updated)

