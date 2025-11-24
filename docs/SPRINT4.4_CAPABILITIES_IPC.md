# Sprint 4.4: Capabilities IPC Implementation

**Date**: 2025-11-24  
**Chunk**: 12B (Phase B)  
**Status**: Implemented

---

## Overview

This document describes the real IPC implementation for Capabilities requests between Phoenix and Bedrock over Unix domain sockets (LocalSocket).

---

## Protocol Specification

### Transport
- **Type**: Unix domain socket (via `QLocalSocket` / `QLocalServer`)
- **Default socket path**: `palantir_bedrock` (resolves to `/tmp/palantir_bedrock.sock` on macOS)
- **Configuration**: Phoenix reads `PALANTIR_SOCKET_PATH` environment variable; if unset, uses default

### Framing
- **Format**: Length-prefixed protobuf messages
- **Length prefix**: 4-byte little-endian `uint32_t` indicating message size in bytes
- **Message**: Serialized protobuf message (N bytes, where N = length prefix value)

### Message Flow (Capabilities)

1. **Client (Phoenix)**:
   - Connects to server socket
   - Serializes `CapabilitiesRequest` (empty message for WP1)
   - Sends: `[4-byte length][serialized CapabilitiesRequest]`
   - Waits for response

2. **Server (Bedrock)**:
   - Receives length prefix
   - Reads N bytes of message
   - Parses `CapabilitiesRequest`
   - Calls `CapabilitiesService::getCapabilities()`
   - Sends: `[4-byte length][serialized CapabilitiesResponse]`

3. **Client (Phoenix)**:
   - Receives length prefix
   - Reads N bytes of message
   - Parses `CapabilitiesResponse`
   - Returns to caller

### Connection Lifecycle (WP1)
- **One request per connection**: Connect → send request → read response → disconnect
- Future: Keep connection alive for multiple requests (WP2+)

---

## Implementation Details

### Bedrock Server

**Files**:
- `src/palantir/PalantirServer.hpp/.cpp` - Qt-based IPC server
- `src/palantir/bedrock_server.cpp` - Server executable
- `src/palantir/CapabilitiesService.hpp/.cpp` - Capabilities response generator

**Key Changes**:
- Updated `handleCapabilitiesRequest()` to use `CapabilitiesService`
- Sends `CapabilitiesResponse` (not old `Capabilities` message)
- Uses new `palantir/capabilities.pb.h` (not old `palantir.pb.h`)
- Old message handlers (StartJob, Cancel, Ping/Pong) disabled for WP1

**Build**:
```bash
cd /Users/underlord/workspace/bedrock
cmake -S . -B build/ipc -GNinja -DBEDROCK_WITH_TRANSPORT_DEPS=ON -DBEDROCK_WITH_OCCT=OFF
cmake --build build/ipc --target bedrock_server
```

**Run**:
```bash
./build/ipc/bedrock_server --socket palantir_bedrock
```

### Phoenix Client

**Files**:
- `src/transport/LocalSocketChannel.hpp/.cpp` - IPC client implementation
- `src/analysis/RemoteExecutor.cpp` - Uses `LocalSocketChannel` for capabilities

**Key Changes**:
- `connect()`: Real `QLocalSocket` connection with 5-second timeout
- `getCapabilities()`: Full IPC implementation (serialize request, send, receive, parse response)
- `isConnected()`: Checks actual socket state
- `disconnect()`: Closes socket connection

**Build**:
```bash
cd /Users/underlord/workspace/phoenix
cmake -S . -B build/ipc -GNinja -DPHX_WITH_TRANSPORT_DEPS=ON
cmake --build build/ipc --target phoenix_app
```

---

## Manual Testing

### End-to-End Test

**Terminal 1: Start Bedrock Server**
```bash
cd /Users/underlord/workspace/bedrock
./build/ipc/bedrock_server --socket palantir_bedrock
```

Expected output:
```
Bedrock server running on socket: palantir_bedrock
Max concurrency: 8
Supported features: xy_sine, heat_diffusion
```

**Terminal 2: Test Phoenix (via RemoteExecutor)**
```bash
cd /Users/underlord/workspace/phoenix
# Run Phoenix with remote mode enabled
# Or use a test harness that calls RemoteExecutor::execute()
```

**Verification**:
- Phoenix connects to Bedrock server
- Capabilities request sent and received
- `CapabilitiesResponse` contains:
  - `server_version = "bedrock-0.0.1"`
  - `supported_features = ["xy_sine"]`

### Error Cases

**Server not running**:
- Phoenix `connect()` fails with timeout
- `RemoteExecutor` reports "Failed to connect to Bedrock server"

**Server stops during request**:
- Phoenix `waitForReadyRead()` times out (5 seconds)
- `RemoteExecutor` reports "Timeout waiting for CapabilitiesResponse"

**Malformed response**:
- Phoenix `ParseFromArray()` fails
- `RemoteExecutor` reports "Failed to parse CapabilitiesResponse"

---

## Build Flags

### Bedrock
- `BEDROCK_WITH_TRANSPORT_DEPS=ON`: Enables proto codegen, `CapabilitiesService`, `PalantirServer`, `bedrock_server`
- `BEDROCK_WITH_TRANSPORT_DEPS=OFF`: Disables all transport code (default)

### Phoenix
- `PHX_WITH_TRANSPORT_DEPS=ON`: Enables proto codegen, `LocalSocketChannel` IPC, `RemoteExecutor` remote path
- `PHX_WITH_TRANSPORT_DEPS=OFF`: Disables transport code, uses local-only mode (default)

---

## Future Work (Out of Scope for WP1)

- **Multi-request connections**: Keep socket open for multiple requests
- **XY Sine RPC**: Implement `StartJob` / `ComputeSpec` proto messages and handlers
- **Progress reporting**: Implement `Progress` message and streaming
- **Cancellation**: Implement `Cancel` message and job cancellation
- **Error mapping**: Map transport errors to `AnalysisError` enum
- **Connection pooling**: Reuse connections across requests
- **Heartbeat/Ping-Pong**: Implement keepalive mechanism

---

## References

- Phase A Plan: `PHOENIX_SPRINT4.4_CHUNK12B_PHASE_A_PLAN.md`
- Capabilities Proto: `contracts/proto/palantir/capabilities.proto`
- Bedrock Branch: `feature/palantir-v1.1.0-proto`
- Phoenix Branch: `feature/ipc-capabilities`

