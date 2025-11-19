# WP2.A Plan – Phoenix gRPC UDS Channel Skeleton

## Code Inspection Summary

### Existing Transport Code Structure

**TransportClient (src/transport/TransportClient.hpp):**
- Abstract base class with virtual methods:
  - `connect()` - returns bool
  - `disconnect()` - void
  - `isConnected()` - returns bool
  - `backendName()` - returns QString
- No implementation file (pure interface)

**GrpcUdsChannel (src/transport/GrpcUdsChannel.{hpp,cpp}):**
- Currently stubbed implementation
- Inherits from TransportClient
- Constructor takes optional QString socketPath (defaults to "palantir_bedrock")
- All methods return false/no-op
- Needs full gRPC UDS implementation

**LocalSocketChannel (src/transport/LocalSocketChannel.{hpp,cpp}):**
- Stub (not needed for WP2.A)
- Will be implemented in WP2.B

### Palantir Contracts

**Location:** `contracts/` submodule
- Currently only contains `docs/` directory
- Proto files location TBD (may need to check Bedrock's proto location)
- Bedrock has `proto/bedrock_echo.proto` with EchoService definition

**Echo Proto Definition (from bedrock/proto/bedrock_echo.proto):**
```protobuf
syntax = "proto3";
package bedrock.echo;

service EchoService {
  rpc Echo(EchoRequest) returns (EchoReply);
}

message EchoRequest {
  string message = 1;
}

message EchoReply {
  string message = 1;
}
```

### Bedrock Server State

**Finding:** Bedrock has BOTH transport mechanisms:

1. **QLocalSocket-based PalantirServer:**
   - Uses `QLocalServer` and `QLocalSocket`
   - Default socket name: `"palantir_bedrock"` (Qt LocalSocket name)
   - Used for Palantir protocol (StartJob, Cancel, etc.)

2. **gRPC-based Echo Service:**
   - Echo RPC implementation exists (`src/echo/echo_service_impl.cpp`)
   - Current test uses TCP (`localhost:50051`), not UDS
   - Uses `grpc::ServerBuilder` and `grpc::InsecureServerCredentials()`
   - **For WP2.A:** Need to configure Bedrock to use UDS instead of TCP, or assume it will

**Conclusion:** Bedrock supports gRPC, but currently uses TCP. WP2.A should implement gRPC UDS client assuming Bedrock will provide UDS server.

### CMake State

**Phoenix CMakeLists.txt:**
- Has proto generation scaffolding (lines 306-335)
- Finds `protoc` and `grpc_cpp_plugin` but doesn't generate code yet
- No gRPC libraries linked yet
- `PHX_WITH_PALANTIR_CONTRACTS` option exists but proto generation is "Future"

**Existing Test:**
- `tests/transport_sanity_tests.cpp` - Basic stub tests
- Uses QtTest framework
- Tests that stubs return expected values

---

## Critical Questions & Clarifications Needed

### 1. Transport Protocol Clarification ✅

**Finding:** Bedrock has gRPC Echo service, but currently uses TCP (`localhost:50051`).

**Questions:**
- Should Bedrock be configured to use UDS for WP2.A, or will Phoenix connect to TCP?
- If UDS, what path should Bedrock use? (e.g., `/tmp/bedrock-grpc.sock`)
- Or should Phoenix connect to TCP for now and switch to UDS later?

**Recommendation:** 
- Implement gRPC UDS client in Phoenix (as per prompt)
- Use default UDS path: `unix:/tmp/bedrock-grpc.sock` (gRPC UDS format)
- Note: Bedrock may need UDS server configuration separately

### 2. Proto Generation & Contracts

**Questions:**
- Where are the Palantir contracts proto files? (contracts/ submodule seems empty)
- Should we use Bedrock's `proto/bedrock_echo.proto` directly?
- Do we need to generate gRPC C++ stubs in Phoenix CMake?
- What gRPC version should we target?

**Proposed Approach:**
- Use Bedrock's `proto/bedrock_echo.proto` as reference
- Generate gRPC C++ stubs in Phoenix CMake
- Link against gRPC C++ libraries

### 3. UDS Path Configuration

**Current Bedrock:** Uses Qt LocalSocket name `"palantir_bedrock"` (not a file path)

**For gRPC UDS:**
- gRPC UDS uses file paths like `/tmp/bedrock-grpc.sock`
- Need to determine Bedrock's gRPC UDS path (if it exists)
- Env var: `PHOENIX_UDS_PATH` with default

**Proposed Default:** `/tmp/bedrock-grpc.sock` (standard gRPC UDS convention)

### 4. gRPC Dependencies

**Questions:**
- Is gRPC installed on dev-01?
- What version? (need to check system packages)
- Should we use system gRPC or vendor it?
- Do we need protobuf separately or bundled with gRPC?

**Proposed Approach:**
- Use system gRPC (if available)
- Add `find_package(gRPC REQUIRED)` to CMake
- Link `gRPC::grpc++` and `protobuf::libprotobuf`

### 5. GrpcUdsChannel Interface Design

**Proposed Interface:**
```cpp
class GrpcUdsChannel : public TransportClient {
public:
    explicit GrpcUdsChannel(const QString& socketPath = QString());
    ~GrpcUdsChannel() override;
    
    // TransportClient interface
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    QString backendName() const override;
    
    // Echo RPC method (for WP2.A)
    bool echo(const QString& message, QString& reply);
    
private:
    QString socketPath_;
    std::unique_ptr<grpc::Channel> channel_;
    std::unique_ptr<bedrock::echo::EchoService::Stub> stub_;
    bool connected_;
};
```

### 6. Error Handling

**Questions:**
- What is the "error matrix" mentioned in prompt?
- Should we map gRPC status codes to custom error types?
- For WP2.A, is basic error handling sufficient?

**Proposed Approach:**
- Map gRPC status codes to simple bool return (for WP2.A)
- Add error details later in WP2.E
- Log errors via qDebug/qWarning

### 7. Test Harness

**Proposed Test:**
- Extend `tests/transport_sanity_tests.cpp`
- Add new test: `testGrpcUdsEcho()`
- Assumes Bedrock gRPC server is running
- Calls Echo RPC and verifies response matches request

**Test Structure:**
```cpp
void testGrpcUdsEcho() {
    QString socketPath = qEnvironmentVariable("PHOENIX_UDS_PATH", "/tmp/bedrock-grpc.sock");
    GrpcUdsChannel channel(socketPath);
    
    QVERIFY(channel.connect());
    QVERIFY(channel.isConnected());
    
    QString request = "Hello, Bedrock!";
    QString reply;
    QVERIFY(channel.echo(request, reply));
    
    QCOMPARE(reply, request);  // Echo should return same message
    
    channel.disconnect();
    QVERIFY(!channel.isConnected());
}
```

---

## Proposed Implementation Plan

### Phase 1: Proto File Setup

**Action:** Set up proto file for code generation:
- Use Bedrock's `proto/bedrock_echo.proto` as source
- Copy to Phoenix `proto/` directory or reference contracts
- Ensure proto file is available for CMake generation

### Phase 2: Set Up Proto Generation (if gRPC)

1. **Add proto file to Phoenix:**
   - Copy `bedrock_echo.proto` or reference contracts submodule
   - Place in `proto/` or `contracts/proto/`

2. **Update CMakeLists.txt:**
   - Add `find_package(gRPC REQUIRED)`
   - Add `find_package(Protobuf REQUIRED)`
   - Generate gRPC C++ stubs using `protoc` and `grpc_cpp_plugin`
   - Add generated files to `phoenix_transport` target

3. **Generated Files:**
   - `bedrock_echo.pb.h` / `bedrock_echo.pb.cc` (protobuf)
   - `bedrock_echo.grpc.pb.h` / `bedrock_echo.grpc.pb.cc` (gRPC stubs)

### Phase 3: Implement GrpcUdsChannel

1. **Update GrpcUdsChannel.hpp:**
   - Add gRPC includes
   - Add channel_ and stub_ members
   - Add echo() method signature

2. **Implement GrpcUdsChannel.cpp:**
   - Constructor: Initialize socketPath from env var or default
   - `connect()`: Create gRPC channel to UDS path, create stub
   - `disconnect()`: Reset channel and stub
   - `isConnected()`: Check if channel is valid
   - `backendName()`: Return "gRPC (UDS)"
   - `echo()`: Call Echo RPC, handle errors

3. **UDS Path Logic:**
   ```cpp
   // gRPC UDS format: "unix:/path/to/socket"
   QString defaultPath = QStringLiteral("unix:/tmp/bedrock-grpc.sock");
   QString envPath = qEnvironmentVariable("PHOENIX_UDS_PATH");
   socketPath_ = envPath.isEmpty() ? defaultPath : envPath;
   
   // Ensure "unix:" prefix for gRPC
   if (!socketPath_.startsWith("unix:")) {
       socketPath_ = "unix:" + socketPath_;
   }
   ```

### Phase 4: Update CMake

1. **Add gRPC dependencies:**
   ```cmake
   find_package(gRPC REQUIRED)
   find_package(Protobuf REQUIRED)
   ```

2. **Link libraries to phoenix_transport:**
   ```cmake
   target_link_libraries(phoenix_transport PRIVATE
     gRPC::grpc++
     protobuf::libprotobuf
   )
   ```

3. **Add proto generation:**
   - Use `protobuf_generate_cpp()` or custom commands
   - Generate both .pb and .grpc.pb files

### Phase 5: Create Echo Test

1. **Extend transport_sanity_tests.cpp:**
   - Add `testGrpcUdsEcho()` method
   - Skip test if Bedrock server not available (optional)

2. **Test Requirements:**
   - Assumes Bedrock gRPC server running on UDS
   - Calls Echo RPC
   - Verifies response

### Phase 6: Build & Test

1. **Build Phoenix:**
   ```bash
   cmake -S . -B build/wp2a \
     -DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64 \
     -DBUILD_TESTING=ON
   cmake --build build/wp2a --parallel
   ```

2. **Run Test:**
   ```bash
   # Start Bedrock gRPC server (if needed)
   ctest --test-dir build/wp2a -R transport_sanity
   ```

---

## Open Questions for User

1. **Bedrock UDS Server:** Bedrock's Echo test uses TCP (`localhost:50051`). Should Bedrock be configured to use UDS (`/tmp/bedrock-grpc.sock`) for WP2.A, or will Phoenix connect to TCP temporarily?

2. **Contracts Location:** Should we copy `bedrock_echo.proto` from Bedrock to Phoenix, or use contracts submodule? (Currently contracts/ seems empty)

3. **gRPC Availability:** Is gRPC installed on dev-01? What version? Should we check system packages or vendor it?

4. **Error Matrix:** What is the "error matrix" mentioned in the prompt? Should we implement basic error handling for WP2.A, or defer to WP2.E?

5. **Test Assumptions:** Should the test assume Bedrock gRPC server is running, or skip if unavailable? Should we document how to start Bedrock server?

6. **UDS Path Default:** Confirm default UDS path `/tmp/bedrock-grpc.sock` is acceptable, or suggest alternative.

---

## Next Steps

**Waiting for user clarification on transport protocol mismatch before proceeding.**

Once clarified:
1. Finalize interface design
2. Implement proto generation
3. Implement GrpcUdsChannel
4. Add test
5. Build and verify

