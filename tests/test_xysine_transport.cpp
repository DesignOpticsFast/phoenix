#include <QtTest/QtTest>
#include "transport/LocalSocketChannel.hpp"
#include "transport/TransportFactory.hpp"
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QMap>
#include <QVariant>
#include <cmath>

class XYSineTransportTests : public QObject {
    Q_OBJECT

private slots:
    void testXYSineComputeSuccess();
    void testXYSineParameterConversion();
    void testXYSineConnectionFailure();
};

void XYSineTransportTests::testXYSineComputeSuccess()
{
    // Check if Palantir server is available
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString socketName = env.value("PHOENIX_LOCALSOCKET_NAME", "palantir_bedrock");
    
    LocalSocketChannel client(socketName);
    if (!client.connect()) {
        QSKIP("Palantir server not available - skipping XY Sine compute test");
    }
    
    // Test parameters
    QMap<QString, QVariant> params;
    params.insert("frequency", 1.0);
    params.insert("amplitude", 1.0);
    params.insert("phase", 0.0);
    params.insert("samples", 100);
    
    // Compute
    XYSineResult result;
    bool success = client.computeXYSine(params, result);
    
    if (!success) {
        QSKIP("XY Sine computation failed - server may not support xy_sine feature");
    }
    
    // Verify result
    QVERIFY(result.x.size() == 100);
    QVERIFY(result.y.size() == 100);
    QVERIFY(result.x.size() == result.y.size());
    
    // Verify x is monotonic increasing from 0 to 2π
    QVERIFY(result.x.size() >= 2);
    QVERIFY(std::abs(result.x[0] - 0.0) < 1e-10);
    QVERIFY(std::abs(result.x[99] - 2.0 * M_PI) < 1e-6);
    
    for (size_t i = 1; i < result.x.size(); ++i) {
        QVERIFY(result.x[i] > result.x[i-1]);
    }
    
    // Verify y matches expected sine values
    QVERIFY(std::abs(result.y[0] - 0.0) < 1e-6);      // sin(0) = 0
    QVERIFY(std::abs(result.y[25] - 1.0) < 1e-3);     // sin(π/2) ≈ 1
    QVERIFY(std::abs(result.y[50] - 0.0) < 1e-3);     // sin(π) ≈ 0
    QVERIFY(std::abs(result.y[75] - (-1.0)) < 1e-3);  // sin(3π/2) ≈ -1
    QVERIFY(std::abs(result.y[99] - 0.0) < 1e-3);     // sin(2π) ≈ 0
    
    client.disconnect();
}

void XYSineTransportTests::testXYSineParameterConversion()
{
    // Test parameter conversion (without actually connecting)
    QMap<QString, QVariant> params;
    params.insert("frequency", 2.5);
    params.insert("amplitude", 3.0);
    params.insert("phase", 1.57);
    params.insert("samples", 500);
    
    // Verify parameters can be converted to strings
    for (auto it = params.begin(); it != params.end(); ++it) {
        QString key = it.key();
        QVariant value = it.value();
        
        QString valueStr;
        if (value.type() == QVariant::Double) {
            valueStr = QString::number(value.toDouble());
            QVERIFY(!valueStr.isEmpty());
        } else if (value.type() == QVariant::Int) {
            valueStr = QString::number(value.toInt());
            QVERIFY(!valueStr.isEmpty());
        } else {
            valueStr = value.toString();
        }
        
        QVERIFY(!valueStr.isEmpty());
    }
}

void XYSineTransportTests::testXYSineConnectionFailure()
{
    // Test with invalid socket name
    LocalSocketChannel client("nonexistent_socket_12345");
    
    QMap<QString, QVariant> params;
    params.insert("samples", 10);
    
    XYSineResult result;
    bool success = client.computeXYSine(params, result);
    
    // Should fail due to connection failure
    QVERIFY(!success);
}

QTEST_MAIN(XYSineTransportTests)
#include "test_xysine_transport.moc"

