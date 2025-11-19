#include <QtTest/QtTest>
#include "analysis/demo/XYSineDemo.hpp"
#include "analysis/AnalysisWorker.hpp"
#include "analysis/AnalysisProgress.hpp"
#include "transport/LocalSocketChannel.hpp"
#include <QSignalSpy>
#include <QProcessEnvironment>
#include <QThread>
#include <cmath>

class LocalXYSineTests : public QObject {
    Q_OBJECT

private slots:
    void testLocalXYSineMatchesBedrockMath();
    void testLocalXYSineParameterParsing();
    void testLocalXYSineSampleClamping();
    void testDemoModeBypassesBedrock();
};

void LocalXYSineTests::testLocalXYSineMatchesBedrockMath()
{
    // Test with known parameters matching Bedrock defaults
    QMap<QString, QVariant> params;
    params["frequency"] = 1.0;
    params["amplitude"] = 1.0;
    params["phase"] = 0.0;
    params["samples"] = 1000;
    
    XYSineResult result;
    QVERIFY(XYSineDemo::compute(params, result));
    
    // Verify array sizes match
    QCOMPARE(result.x.size(), 1000);
    QCOMPARE(result.y.size(), 1000);
    QCOMPARE(result.x.size(), result.y.size());
    
    // Verify x domain: first point should be 0, last should be 2π
    QVERIFY(std::abs(result.x[0] - 0.0) < 1e-10);
    QVERIFY(std::abs(result.x[999] - 2.0 * M_PI) < 1e-10);
    
    // Verify y values: sin(0) = 0, sin(π/2) ≈ 1 (at quarter point)
    QVERIFY(std::abs(result.y[0] - 0.0) < 1e-10);
    
    // Check quarter point (should be near amplitude = 1.0)
    int quarterIdx = 250;  // 1000 / 4
    double quarterY = result.y[quarterIdx];
    QVERIFY(std::abs(quarterY - 1.0) < 0.1);  // Allow some tolerance
    
    // Verify monotonic x values
    for (size_t i = 1; i < result.x.size(); ++i) {
        QVERIFY(result.x[i] > result.x[i-1]);
    }
}

void LocalXYSineTests::testLocalXYSineParameterParsing()
{
    // Test default values when params are missing
    QMap<QString, QVariant> emptyParams;
    XYSineResult result;
    QVERIFY(XYSineDemo::compute(emptyParams, result));
    
    // Should use defaults: frequency=1.0, amplitude=1.0, phase=0.0, samples=1000
    QCOMPARE(result.x.size(), 1000);
    QCOMPARE(result.y.size(), 1000);
    
    // Test custom parameters
    QMap<QString, QVariant> customParams;
    customParams["frequency"] = 2.0;
    customParams["amplitude"] = 0.5;
    customParams["phase"] = M_PI / 2.0;
    customParams["samples"] = 500;
    
    XYSineResult customResult;
    QVERIFY(XYSineDemo::compute(customParams, customResult));
    
    QCOMPARE(customResult.x.size(), 500);
    QCOMPARE(customResult.y.size(), 500);
    
    // Test n_samples alias (backwards compatibility)
    QMap<QString, QVariant> aliasParams;
    aliasParams["n_samples"] = 200;
    
    XYSineResult aliasResult;
    QVERIFY(XYSineDemo::compute(aliasParams, aliasResult));
    QCOMPARE(aliasResult.x.size(), 200);
    
    // Test that "samples" takes precedence over "n_samples"
    QMap<QString, QVariant> precedenceParams;
    precedenceParams["samples"] = 300;
    precedenceParams["n_samples"] = 200;  // Should be ignored
    
    XYSineResult precedenceResult;
    QVERIFY(XYSineDemo::compute(precedenceParams, precedenceResult));
    QCOMPARE(precedenceResult.x.size(), 300);
}

void LocalXYSineTests::testLocalXYSineSampleClamping()
{
    // Test that samples < 2 are clamped to 2
    QMap<QString, QVariant> params;
    params["samples"] = 1;
    
    XYSineResult result;
    QVERIFY(XYSineDemo::compute(params, result));
    QCOMPARE(result.x.size(), 2);  // Clamped to minimum 2
    
    // Test samples = 0
    params["samples"] = 0;
    XYSineResult result2;
    QVERIFY(XYSineDemo::compute(params, result2));
    QCOMPARE(result2.x.size(), 2);  // Clamped to minimum 2
    
    // Test samples = 2 (minimum valid)
    params["samples"] = 2;
    XYSineResult result3;
    QVERIFY(XYSineDemo::compute(params, result3));
    QCOMPARE(result3.x.size(), 2);
}

void LocalXYSineTests::testDemoModeBypassesBedrock()
{
    // Set demo mode environment variable
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PHOENIX_DEMO_MODE", "1");
    QProcessEnvironment::setSystemEnvironment(env);
    
    // Create AnalysisWorker
    AnalysisWorker* worker = new AnalysisWorker();
    QThread* thread = new QThread();
    worker->moveToThread(thread);
    
    // Set up parameters
    QMap<QString, QVariant> params;
    params["frequency"] = 1.0;
    params["amplitude"] = 1.0;
    params["phase"] = 0.0;
    params["samples"] = 100;
    
    worker->setParameters("xy_sine", params);
    
    // Set up signal spies
    QSignalSpy startedSpy(worker, &AnalysisWorker::started);
    QSignalSpy finishedSpy(worker, &AnalysisWorker::finished);
    QSignalSpy progressSpy(worker, &AnalysisWorker::progressChanged);
    
    // Connect thread signals
    connect(thread, &QThread::started, worker, &AnalysisWorker::run);
    connect(worker, &AnalysisWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    // Start thread
    thread->start();
    
    // Wait for completion (with timeout)
    QVERIFY(thread->wait(5000));
    
    // Verify signals were emitted
    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(finishedSpy.count(), 1);
    QVERIFY(progressSpy.count() >= 1);  // At least one progress update
    
    // Verify finished signal indicates success
    QList<QVariant> finishedArgs = finishedSpy.takeFirst();
    QVERIFY(finishedArgs[0].toBool() == true);  // success = true
    
    // Verify result contains valid XY data
    QVariant resultVariant = finishedArgs[1];
    QVERIFY(resultVariant.canConvert<XYSineResult>());
    
    XYSineResult result = resultVariant.value<XYSineResult>();
    QVERIFY(result.x.size() > 0);
    QVERIFY(result.y.size() > 0);
    QCOMPARE(result.x.size(), result.y.size());
    QCOMPARE(result.x.size(), 100u);  // Should match samples parameter
    
    // Clean up environment
    env.remove("PHOENIX_DEMO_MODE");
    QProcessEnvironment::setSystemEnvironment(env);
}

QTEST_MAIN(LocalXYSineTests)
#include "test_local_xysine.moc"

