#include <QtTest/QtTest>
#include "transport/TransportClient.hpp"
#include "transport/GrpcUdsChannel.hpp"
#include "transport/LocalSocketChannel.hpp"

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

        QVERIFY(!grpc.connect());  // Stub always returns false
        QVERIFY(!lsc.connect());   // Stub always returns false

        QVERIFY(!grpc.isConnected());
        QVERIFY(!lsc.isConnected());
    }
};

QTEST_MAIN(TransportSanity)
#include "transport_sanity_tests.moc"

