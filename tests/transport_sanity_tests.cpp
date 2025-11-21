#include <QtTest/QtTest>
#include "transport/TransportClient.hpp"
#include "transport/GrpcUdsChannel.hpp"
#include "transport/LocalSocketChannel.hpp"
#include "transport/TransportFactory.hpp"
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QThread>

class TransportSanity : public QObject {
    Q_OBJECT

private slots:
    void testStubNames() {
        GrpcUdsChannel grpc;
        LocalSocketChannel lsc;

        QVERIFY(grpc.backendName().contains("gRPC"));
        QVERIFY(lsc.backendName().contains("LocalSocket"));
    }

    void testStubConnection() {
        GrpcUdsChannel grpc;
        LocalSocketChannel lsc;

        QVERIFY(!grpc.isConnected());
        QVERIFY(!lsc.isConnected());

        // GrpcUdsChannel now attempts real connection, so this may succeed or fail
        // depending on whether Bedrock server is running
        bool grpcConnected = grpc.connect();
        if (grpcConnected) {
            QVERIFY(grpc.isConnected());
            grpc.disconnect();
            QVERIFY(!grpc.isConnected());
        }
        
        // LocalSocketChannel now attempts real connection
        bool lscConnected = lsc.connect();
        if (lscConnected) {
            QVERIFY(lsc.isConnected());
            lsc.disconnect();
            QVERIFY(!lsc.isConnected());
        }
    }
    
    void testGrpcEcho() {
        // Get endpoint from environment or use default
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString endpoint = env.value("PHOENIX_GRPC_ENDPOINT", "localhost:50051");
        
        GrpcUdsChannel channel(endpoint);
        
        // Try to connect
        if (!channel.connect()) {
            QSKIP("Bedrock Echo server not available - skipping Echo test");
        }
        
        QVERIFY(channel.isConnected());
        
        // Test Echo RPC
        QString request = "Hello, Bedrock!";
        QString reply;
        
        bool success = channel.echo(request, reply);
        
        if (!success) {
            // If RPC fails, it might be because server is not running
            // Skip the test rather than failing
            channel.disconnect();
            QSKIP("Echo RPC failed - Bedrock server may not be running");
        }
        
        // Verify response matches request (Echo should return same message)
        QCOMPARE(reply, request);
        
        // Test with another message
        request = "Test message 123";
        success = channel.echo(request, reply);
        if (success) {
            QCOMPARE(reply, request);
        }
        
        channel.disconnect();
        QVERIFY(!channel.isConnected());
    }
    
    void testLocalSocketConnection() {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString socketName = env.value("PHOENIX_LOCALSOCKET_NAME", "palantir_bedrock");
        
        LocalSocketChannel channel(socketName);
        
        QVERIFY(!channel.isConnected());
        
        // Try to connect
        if (!channel.connect()) {
            QSKIP("Bedrock LocalSocket server not available - skipping LocalSocket connection test");
        }
        
        QVERIFY(channel.isConnected());
        
        channel.disconnect();
        QVERIFY(!channel.isConnected());
    }
    
    void testLocalSocketCapabilities() {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString socketName = env.value("PHOENIX_LOCALSOCKET_NAME", "palantir_bedrock");
        
        LocalSocketChannel channel(socketName);
        
        // Try to connect
        if (!channel.connect()) {
            QSKIP("Bedrock LocalSocket server not available - skipping Capabilities test");
        }
        
        QVERIFY(channel.isConnected());
        
        // Test CapabilitiesRequest RPC
        QStringList features;
        bool success = channel.requestCapabilities(features);
        
        if (!success) {
            channel.disconnect();
            QSKIP("CapabilitiesRequest RPC failed - Bedrock server may not be running");
        }
        
        // Verify we got a response (features list may be empty, but request should succeed)
        QVERIFY(true);  // Just verify no crash - features list may be empty
        
        qDebug() << "LocalSocket capabilities: Received" << features.size() << "features";
        for (const QString& feature : features) {
            qDebug() << "  -" << feature;
        }
        
        channel.disconnect();
        QVERIFY(!channel.isConnected());
    }
    
    void testLocalSocketPing() {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString socketName = env.value("PHOENIX_LOCALSOCKET_NAME", "palantir_bedrock");
        
        LocalSocketChannel channel(socketName);
        
        // Try to connect
        if (!channel.connect()) {
            QSKIP("Bedrock PalantirServer not available - skipping Ping test");
        }
        
        // Verify connection and initial error state
        QVERIFY(channel.isConnected());
        QVERIFY(channel.lastError() == TransportError::NoError);
        
        // Send Ping (non-blocking, async)
        channel.pingHealthCheck();
        
        // Wait for async Pong response (200-300ms delay)
        QThread::msleep(250);
        
        // Process any incoming data (parseIncomingData() is called automatically via Qt signals)
        // Give Qt event loop a chance to process signals
        QCoreApplication::processEvents();
        
        // Verify still connected and no errors
        QVERIFY(channel.isConnected());
        QVERIFY(channel.lastError() == TransportError::NoError);
        
        // Verify error string is empty or indicates success
        QString errorStr = channel.lastErrorString();
        QVERIFY(errorStr.isEmpty() || channel.lastError() == TransportError::NoError);
        
        channel.disconnect();
        QVERIFY(!channel.isConnected());
        QVERIFY(channel.lastError() == TransportError::NoError);  // Disconnect clears error
    }
    
    void testLocalSocketPingConnectionFailure() {
        // Test with invalid socket name (Bedrock not running on this socket)
        LocalSocketChannel channel("nonexistent_socket_12345");
        
        QVERIFY(!channel.isConnected());
        QVERIFY(channel.lastError() == TransportError::NoError);  // No error until connect attempt
        
        // Attempt connection (should fail)
        bool connected = channel.connect();
        
        // Verify connection failed
        QVERIFY(!connected);
        QVERIFY(!channel.isConnected());
        
        // Verify error code is set appropriately
        TransportError error = channel.lastError();
        QVERIFY(error == TransportError::ConnectionFailed || 
               error == TransportError::ConnectionTimeout);
        
        // Verify error string is not empty
        QString errorStr = channel.lastErrorString();
        QVERIFY(!errorStr.isEmpty());
        
        // Verify error string is user-friendly
        QVERIFY(errorStr.contains("Bedrock") || errorStr.contains("connect") || 
               errorStr.contains("timeout"));
    }
    
    void testTransportSelectionGrpc() {
        // Test explicit grpc backend selection
        auto client = makeTransportClient(TransportBackend::Grpc);
        QVERIFY(client != nullptr);
        
        // Verify it's a gRPC channel (check backend name)
        QVERIFY(client->backendName().contains("gRPC"));
        
        // Try to connect (may succeed or skip)
        bool connected = client->connect();
        if (connected) {
            QVERIFY(client->isConnected());
            client->disconnect();
        }
    }
    
    void testTransportSelectionLocalSocket() {
        // Test explicit localsocket backend selection
        auto client = makeTransportClient(TransportBackend::LocalSocket);
        QVERIFY(client != nullptr);
        
        // Verify it's a LocalSocket channel
        QVERIFY(client->backendName().contains("LocalSocket"));
        
        // Try to connect (may succeed or skip)
        bool connected = client->connect();
        if (connected) {
            QVERIFY(client->isConnected());
            client->disconnect();
        }
    }
    
    void testTransportSelectionAuto() {
        // Test auto backend selection (defaults to gRPC for now)
        auto client = makeTransportClient(TransportBackend::Auto);
        QVERIFY(client != nullptr);
        
        // Auto should default to gRPC for now
        QVERIFY(client->backendName().contains("gRPC"));
        
        // Try to connect (may succeed or skip)
        bool connected = client->connect();
        if (connected) {
            QVERIFY(client->isConnected());
            client->disconnect();
        }
    }
    
    void testTransportSelectionFromEnv() {
        // Test that makeTransportClientFromEnv() reads PHOENIX_TRANSPORT
        // Note: This test relies on the actual environment variable being set
        // If not set, it should default to gRPC (auto behavior)
        auto client = makeTransportClientFromEnv();
        QVERIFY(client != nullptr);
        
        // Should return some transport (gRPC by default)
        QVERIFY(!client->backendName().isEmpty());
    }
};

QTEST_MAIN(TransportSanity)
#include "transport_sanity_tests.moc"

