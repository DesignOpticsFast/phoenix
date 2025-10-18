# ADR-20251017: Transport Protocol Choice for Palantir

> ⚠️ **Superseded by Phase 0.5 Gate Decision (2025-10-18): Qt 6.10 + Graphs / gRPC UDS baseline.**

**Date:** October 17, 2025  
**Status:** Accepted  
**Context:** Phoenix Sprint 4 - Palantir Protocol Implementation  

## Decision

**Use LocalSocket + Protobuf for Palantir protocol communication between Phoenix and Bedrock.**

## Rationale

### Gate 0.5B Transport Benchmark Results

| Metric | LocalSocket | gRPC UDS | Threshold | Decision |
|--------|-------------|----------|-----------|----------|
| Average Latency | 100ms | 107.5ms | - | ✅ LocalSocket |
| Overhead | 0% | 7.5% | < 5% | ❌ gRPC exceeds threshold |
| Footprint | 0MB | 45MB | < 50MB | ✅ gRPC acceptable |

### Key Factors

1. **Performance:** gRPC overhead (7.5%) exceeds the 5% threshold
2. **Simplicity:** LocalSocket provides direct, low-overhead communication
3. **Dependencies:** Reduces external dependencies (gRPC libraries)
4. **Debugging:** Easier to debug and troubleshoot local communication

## Implementation Details

### LocalSocket Configuration
- **Socket Name:** `palantir_bedrock` + PID suffix (avoid stale connections)
- **Framing:** 4-byte little-endian length prefix + protobuf payload
- **Reconnection:** Phoenix retries every 1s up to 5s on disconnect
- **Error Handling:** Use canonical error codes (OK, INVALID_ARGUMENT, etc.)

### Protobuf Messages
- ComputeSpec, StartReply, Progress, ResultMeta, DataChunk
- Cancel, JobId, Capabilities, Pong
- Result data: ResultMeta (dtype "f64", shape [N]) + DataChunk blocks

## Consequences

### Positive
- ✅ Lower latency and overhead
- ✅ Simpler debugging and maintenance
- ✅ Reduced external dependencies
- ✅ Better performance for local communication

### Negative
- ❌ No built-in load balancing (not needed for local communication)
- ❌ No automatic retry mechanisms (implemented manually)
- ❌ Less standardized than gRPC (acceptable for internal protocol)

## Alternatives Considered

### gRPC UDS
- **Rejected:** 7.5% overhead exceeds 5% threshold
- **Note:** May be reconsidered for future remote communication needs

### HTTP/JSON
- **Rejected:** Higher overhead than binary protocols
- **Note:** Not suitable for high-frequency communication

## Implementation Status

- ✅ Transport benchmark completed
- ✅ Decision documented
- 🔄 Implementation in progress (Phase 2)

## References

- [Gate 0.5B Transport Benchmark Results](../results/gate0_5_transport_decision.md)
- [Phoenix Sprint 4 Control Document](phoenix-sprint4-control-rev5.md)



