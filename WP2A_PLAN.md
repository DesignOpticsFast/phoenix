# WP2.A Plan – Phoenix gRPC UDS Channel Skeleton

## Code Inspection Summary

### Existing Transport Code Structure

**TransportClient (src/transport/TransportClient.{hpp,cpp}):**
- Currently a stub/placeholder
- Likely has interface for channel management
- Needs integration with GrpcUdsChannel

**GrpcUdsChannel (src/transport/GrpcUdsChannel.{hpp,cpp}):**
- Currently stubbed
- Needs full gRPC UDS implementation
- Must integrate with Palantir contracts

**LocalSocketChannel (src/transport/LocalSocketChannel.{hpp,cpp}):**
- Stub (not needed for WP2.A)
- Will be implemented in WP2.B

### Palantir Contracts

**Location:** `contracts/` submodule
- Shared proto definitions
- Need to check for Echo RPC proto
- Generated gRPC stubs location TBD

### Bedrock Echo Server

**Location:** `bedrock/src/palantir/bedrock_server.cpp`
- Echo RPC server implementation
- UDS path configuration needs verification
- Default path likely `/tmp/bedrock-grpc.sock` or similar

---

## Questions & Clarifications Needed

### 1. Palantir Contracts Structure
- **Question:** What is the exact path structure of the contracts submodule?
- **Question:** Where are the generated gRPC C++ stubs located?
- **Question:** Are the stubs already generated, or do we need to generate them in Phoenix's CMake?

### 2. gRPC Integration
- **Question:** Is gRPC already linked in Phoenix CMakeLists.txt?
- **Question:** What gRPC version/compatibility is expected?
- **Question:** Should we use gRPC's C++ API directly or wrap it?

### 3. UDS Path Configuration
- **Question:** What is Bedrock's default UDS path? (Need to verify from bedrock_server.cpp)
- **Question:** Should PHOENIX_UDS_PATH override, or should we use a different env var name?
- **Question:** What should the default path be if env var is not set?

### 4. TransportClient Interface
- **Question:** What is the current TransportClient interface shape?
- **Question:** How should GrpcUdsChannel integrate with TransportClient?
- **Question:** Should TransportClient own the channel, or just use it?

### 5. Error Handling
- **Question:** What is the "error matrix" mentioned in the prompt?
- **Question:** What error types should we map from gRPC status codes?
- **Question:** Should we create a custom error type or use existing Qt error handling?

### 6. Test Harness
- **Question:** Should the sanity test be a QtTest-based test or a standalone binary?
- **Question:** Should it assume Bedrock server is already running, or start it?
- **Question:** What level of assertion is needed (just "no crash" or full response validation)?

---

## Proposed Plan (Pending Inspection)

### Phase 1: Code Inspection
1. Read TransportClient interface
2. Read GrpcUdsChannel stub
3. Inspect contracts submodule structure
4. Check Bedrock UDS path configuration
5. Verify gRPC dependencies in CMake

### Phase 2: Interface Design
1. Define GrpcUdsChannel public interface
2. Define integration with TransportClient
3. Define error mapping strategy
4. Define UDS path configuration approach

### Phase 3: Implementation
1. Implement GrpcUdsChannel core methods
2. Integrate with TransportClient
3. Add UDS path configuration (env var + default)
4. Add basic error mapping

### Phase 4: Testing
1. Create sanity test binary
2. Test Echo RPC call over UDS
3. Verify error handling
4. Update CMake for test

### Phase 5: Integration
1. Ensure Phoenix builds
2. Run sanity test on dev-01
3. Commit and push to sprint/4.2

---

## Next Steps

**Waiting for code inspection results before finalizing plan...**


