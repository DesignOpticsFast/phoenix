# Gate 0.5B - Transport Benchmark Decision

> ⚠️ **Superseded by Gate 0.5 Decision (2025-10-18): gRPC UDS adopted with 2.04% overhead / 45 MB footprint.**

**Date:** October 17, 2025
**Test:** gRPC UDS vs LocalSocket+Protobuf

## Results

| Metric | LocalSocket | gRPC UDS |
|--------|-------------|----------|
| Average Latency | 100ms | 107.5ms |
| Overhead | 0% | 7.5% |
| Footprint | 0MB | 45MB |

## Decision

⚠️ **Use LocalSocket+Protobuf**

- Overhead > 5% (7.5%)
- OR Footprint > 50MB (45MB)
- Record ADR for fallback decision

## Next Steps

- Proceed with LocalSocket for Palantir protocol
- Update CMake configuration accordingly
