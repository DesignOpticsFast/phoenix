# WP2.B Plan – Transport Selection + LocalSocketChannel

## Code Inspection Summary

### Existing Transport Code

**TransportClient (src/transport/TransportClient.hpp):**
- Abstract base class with virtual methods:
  - `connect()` - returns bool
  - `disconnect()` - void
  - `isConnected()` - returns bool
  - `backendName()` - returns QString
- No implementation file (pure interface)
- No factory/selection mechanism yet

**GrpcUdsChannel (src/transport/GrpcUdsChannel.{hpp,cpp}):**
- Fully implemented in WP2.A
- TCP-based gRPC channel (localhost:50051)
- Has `echo()` method for Echo RPC
- Uses gRPC C++ API

**LocalSocketChannel (src/transport/LocalSocketChannel.{hpp,cpp}):**
- Currently stubbed
- Constructor takes QString socketPath (defaults to "palantir_bedrock")
- All methods return false/no-op
- Needs full QLocalSocket implementation

### Bedrock LocalSocket Server

**PalantirServer (bedrock/src/palantir/PalantirServer.{hpp,cpp}):**
- Uses `QLocalServer` and `QLocalSocket`
- Default socket name: `"palantir_bedrock"` (Qt LocalSocket name, not file path)
- Message protocol:
  - Length-prefixed messages (4-byte little-endian length + protobuf data)
  - Uses Palantir protobuf messages (palantir.pb.h)
  - Handles: StartJob, Cancel, CapabilitiesRequest, Ping
- **Finding:** No explicit Echo RPC handler found in PalantirServer
- **Question:** Does Bedrock have a separate Echo service over LocalSocket, or should we use Ping?

**Message Framing (from PalantirServer.cpp):**
```cpp
// Send: 4-byte length (little-endian) + serialized protobuf
uint32_t length = static_cast<uint32_t>(serialized.size());
data.append(reinterpret_cast<const char*>(&length), 4);
data.append(serialized.data(), serialized.size());

// Receive: Read 4-byte length, then read that many bytes
```

### Test Infrastructure

**transport_sanity_tests.cpp:**
- Uses QtTest framework
- Has `testStubNames()`, `testStubConnection()`, `testGrpcEcho()`
- Echo test uses `QSKIP()` when server unavailable
- Reads endpoint from `PHOENIX_GRPC_ENDPOINT` env var

---

## Questions & Clarifications Needed

### 1. Bedrock LocalSocket Echo Protocol ⚠️

**Issue:** Bedrock's PalantirServer doesn't appear to have an Echo RPC handler.

**Questions:**
- Does Bedrock have a separate Echo service over LocalSocket?
- Should we use the Ping/Pong mechanism instead?
- Or should we implement a simple Echo handler in Bedrock for WP2.B?
- What protobuf message types should LocalSocketChannel use?

**Recommendation:** 
- Option A: Use CapabilitiesRequest as a simple test (already wired, returns server capabilities)
- Option B: Use Ping/Pong (handlePing exists but may not be wired into handleMessage - needs verification)
- Option C: Add Ping parsing to Bedrock's handleMessage (requires Bedrock changes, violates "don't modify Bedrock" constraint)

**Proposed:** Use CapabilitiesRequest for WP2.B since:
1. It's already fully wired in Bedrock's handleMessage()
2. Provides a simple round-trip test (request → response)
3. Doesn't require Bedrock changes
4. Can switch to Ping/Pong later if needed

**Alternative:** If Ping is actually wired (need to verify), use Ping/Pong as it's more echo-like.

### 2. Transport Selection Architecture

**Current State:**
- `TransportClient` is an abstract interface
- No factory or selection mechanism exists
- Channels are created directly by tests

**Proposed Approach:**
- Create a `TransportFactory` or `TransportClient::create()` static method
- Factory reads `PHOENIX_TRANSPORT` env var
- Returns appropriate channel instance
- For "auto", factory tries gRPC first, falls back to LocalSocket

**Alternative:** Keep channels separate, add selection logic in tests/application code.

**Recommendation:** Add `TransportClient::create()` static factory method for clean abstraction.

### 3. LocalSocketChannel Interface Design

**Proposed Interface:**
```cpp
class LocalSocketChannel : public TransportClient {
public:
    explicit LocalSocketChannel(const QString& socketName = QString());
    ~LocalSocketChannel() override;
    
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    QString backendName() const override;
    
    // Echo-equivalent method (using CapabilitiesRequest)
    bool requestCapabilities(QStringList& features);  // Returns supported features on success
};
```

**Connection Lifecycle:**
- `connect()`: Create QLocalSocket, connect to socket name, wait for connected
- `disconnect()`: Close socket, reset state
- `isConnected()`: Check socket state == QLocalSocket::ConnectedState

**Message Framing:**
- Send: 4-byte length (little-endian) + protobuf serialized data
- Receive: Read 4-byte length, then read that many bytes into buffer
- Parse protobuf from buffer

### 4. Error Handling

**Proposed:**
- Map QLocalSocket errors to simple bool return
- Log errors via qWarning/qDebug
- Return false on connection/RPC failures
- No complex error matrix yet (deferred to WP2.E)

### 5. Transport Selection Logic

**Proposed Behavior:**

| Env Var | Behavior |
|---------|----------|
| `PHOENIX_TRANSPORT=grpc` | Use GrpcUdsChannel only |
| `PHOENIX_TRANSPORT=localsocket` | Use LocalSocketChannel only |
| `PHOENIX_TRANSPORT=auto` (default) | Try gRPC first → if connection fails → try LocalSocket |
| Unset/empty | Same as "auto" |

**Implementation:**
- Factory method reads env var
- For "auto": Try gRPC connect(), if fails or times out, try LocalSocket
- Log which backend was selected

**Alternative Names Considered:**
- `PHOENIX_TRANSPORT_BACKEND` - more explicit but longer
- `PHOENIX_CHANNEL` - shorter but less clear
- **Chosen:** `PHOENIX_TRANSPORT` - matches existing `PHOENIX_GRPC_ENDPOINT` pattern

### 6. Test Design

**Proposed Tests:**

1. **testLocalSocketConnection()**
   - Create LocalSocketChannel
   - Try to connect
   - If succeeds: verify isConnected(), disconnect, verify !isConnected()
   - If fails: skip test

2. **testLocalSocketCapabilities()**
   - Connect to LocalSocket server
   - Send CapabilitiesRequest message
   - Receive Capabilities response, verify features list present
   - Skip if server unavailable

3. **testTransportSelectionGrpc()**
   - Set `PHOENIX_TRANSPORT=grpc`
   - Create channel via factory
   - Verify it's GrpcUdsChannel instance
   - Try to connect (may succeed or skip)

4. **testTransportSelectionLocalSocket()**
   - Set `PHOENIX_TRANSPORT=localsocket`
   - Create channel via factory
   - Verify it's LocalSocketChannel instance
   - Try to connect (may succeed or skip)

5. **testTransportSelectionAuto()**
   - Set `PHOENIX_TRANSPORT=auto` (or unset)
   - Create channel via factory
   - Verify it tries gRPC first
   - If gRPC fails, verify it falls back to LocalSocket
   - Log which backend was actually used

### 7. CMake Changes

**Dependencies:**
- LocalSocket uses Qt's QLocalSocket (already available via Qt6::Core)
- No additional dependencies needed
- May need to link against protobuf if we use Palantir messages

**Proto Files:**
- Question: Do we need palantir.proto in Phoenix, or can we use a simpler approach?
- Option A: Copy palantir.proto and generate stubs (matches Bedrock)
- Option B: Use raw QByteArray messages with manual framing (simpler but less type-safe)
- **Recommendation:** For WP2.B, use Option B (manual framing) to avoid proto complexity. Can add proto later if needed.

**Test Targets:**
- No new test targets needed
- Extend existing `transport_sanity_tests` executable

---

## Proposed Implementation Plan

### Phase 1: LocalSocketChannel Implementation

1. **Update LocalSocketChannel.hpp:**
   - Add QLocalSocket member
   - Add message buffer for reading
   - Add `ping()` method signature
   - Include QLocalSocket headers

2. **Implement LocalSocketChannel.cpp:**
   - `connect()`: Create QLocalSocket, connect to socket name, wait for connected state
   - `disconnect()`: Close socket, clear buffer
   - `isConnected()`: Check socket state
   - `backendName()`: Return "LocalSocket"
   - `requestCapabilities()`: Send CapabilitiesRequest, receive Capabilities, parse features
   - Helper methods: `sendMessage()`, `readMessage()`, `parseIncomingData()`

3. **Message Framing:**
   - Implement length-prefixed message send/receive
   - Use QByteArray for message buffers
   - For CapabilitiesRequest: Need to create palantir::CapabilitiesRequest protobuf message
   - **Decision needed:** Copy palantir.proto or use manual message construction?

### Phase 2: Transport Selection Factory

1. **Add TransportClient factory method:**
   - `static std::unique_ptr<TransportClient> create(const QString& backend = QString())`
   - Reads `PHOENIX_TRANSPORT` env var if backend is empty
   - Returns appropriate channel instance

2. **Implement selection logic:**
   - "grpc" → return GrpcUdsChannel
   - "localsocket" → return LocalSocketChannel
   - "auto" or empty → try gRPC, fallback to LocalSocket

3. **Add helper method:**
   - `static QStringList availableBackends()` - returns list of supported backends

### Phase 3: Tests

1. **Extend transport_sanity_tests.cpp:**
   - Add `testLocalSocketConnection()`
   - Add `testLocalSocketPing()`
   - Add `testTransportSelectionGrpc()`
   - Add `testTransportSelectionLocalSocket()`
   - Add `testTransportSelectionAuto()`

2. **Test Patterns:**
   - Use `QSKIP()` when server unavailable
   - Use `QVERIFY()` for assertions
   - Log backend selection in auto mode

### Phase 4: CMake & Build

1. **Update CMakeLists.txt:**
   - Ensure Qt6::Core is linked (already done)
   - No new dependencies needed

2. **Build & Test:**
   - Build phoenix_transport
   - Build transport_sanity_tests
   - Run tests on dev-01

---

## Open Questions for User

1. **Bedrock LocalSocket Protocol:** Should we use CapabilitiesRequest (already wired) or Ping/Pong (may need Bedrock fix)? **Recommendation:** Use CapabilitiesRequest for WP2.B.

2. **Proto Files:** Should LocalSocketChannel use Palantir protobuf messages (requires copying palantir.proto and generating stubs), or manually construct protobuf messages? **Recommendation:** Copy palantir.proto for type safety, similar to how we copied bedrock_echo.proto in WP2.A.

3. **Factory Location:** Should the factory method be in TransportClient.hpp, or a separate TransportFactory class?

4. **Auto Fallback Timing:** How long should we wait for gRPC connection before falling back to LocalSocket? (Proposed: 2-3 seconds)

5. **Socket Name:** Should LocalSocketChannel use the same default socket name as Bedrock ("palantir_bedrock"), or make it configurable via env var?

---

## Next Steps

**Waiting for user clarification on:**
1. Confirm use of CapabilitiesRequest vs Ping/Pong (recommend CapabilitiesRequest)
2. Confirm copying palantir.proto approach (recommend yes, similar to WP2.A)
3. Factory design preference (recommend TransportClient::create() static method)
4. Auto fallback timeout duration (recommend 2-3 seconds)
5. Socket name configuration (recommend env var PHOENIX_LOCALSOCKET_NAME with default "palantir_bedrock")

Once clarified, proceed with implementation.

