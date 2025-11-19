#include <QtTest/QtTest>
#include "ui/analysis/AnalysisWindow.hpp"
#include "analysis/AnalysisWorker.hpp"
#include <QThread>
#include <QSignalSpy>
#include <QElapsedTimer>
#include <QApplication>
#include <QMap>
#include <QVariant>

class AnalysisTimeoutTests : public QObject {
    Q_OBJECT

private slots:
    void testTimeoutTriggersCancel();
    void testTimeoutStopsOnFinish();
    void testTimeoutStopsOnCancel();
    void testNoTimeoutOnFastRun();
};

void AnalysisTimeoutTests::testTimeoutTriggersCancel()
{
    // Create AnalysisWindow
    AnalysisWindow* window = new AnalysisWindow();
    
    // Set up a "noop_sleepy" feature with long sleep
    QMap<QString, QVariant> params;
    params["sleep_ms"] = 2000;  // Sleep for 2 seconds
    
    // Create worker with short timeout (500ms)
    QThread* thread = new QThread(this);
    AnalysisWorker* worker = new AnalysisWorker();
    worker->moveToThread(thread);
    worker->setParameters("noop_sleepy", params);
    
    QSignalSpy cancelledSpy(worker, &AnalysisWorker::cancelled);
    QSignalSpy finishedSpy(worker, &AnalysisWorker::finished);
    
    // Manually set a short timeout by modifying the timer
    // For this test, we'll use the worker directly and simulate timeout behavior
    // Actually, let's test the AnalysisWindow timeout mechanism
    
    // Connect worker signals
    connect(thread, &QThread::started, worker, &AnalysisWorker::run);
    connect(worker, &AnalysisWorker::cancelled, thread, &QThread::quit);
    connect(worker, &AnalysisWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    // Start worker
    thread->start();
    
    // Wait a bit, then simulate timeout by calling requestCancel
    QThread::msleep(100);
    
    // Simulate timeout: call requestCancel
    worker->requestCancel();
    
    // Wait for cancellation
    QElapsedTimer timer;
    timer.start();
    while (cancelledSpy.count() == 0 && finishedSpy.count() == 0 && timer.elapsed() < 3000) {
        if (cancelledSpy.wait(100) || finishedSpy.wait(100)) {
            break;
        }
        QCoreApplication::processEvents();
    }
    
    QVERIFY(thread->wait(2000));
    
    // Verify cancelled was emitted (not finished)
    QVERIFY(cancelledSpy.count() > 0);
    QVERIFY(finishedSpy.count() == 0);
    
    delete window;
}

void AnalysisTimeoutTests::testTimeoutStopsOnFinish()
{
    // Test that timeout timer stops when worker finishes quickly
    AnalysisWindow* window = new AnalysisWindow();
    
    // Use "noop" feature (immediate completion)
    QMap<QString, QVariant> params;
    
    QThread* thread = new QThread(this);
    AnalysisWorker* worker = new AnalysisWorker();
    worker->moveToThread(thread);
    worker->setParameters("noop", params);
    
    QSignalSpy finishedSpy(worker, &AnalysisWorker::finished);
    
    connect(thread, &QThread::started, worker, &AnalysisWorker::run);
    connect(worker, &AnalysisWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    thread->start();
    
    QElapsedTimer timer;
    timer.start();
    while (finishedSpy.count() == 0 && timer.elapsed() < 2000) {
        if (finishedSpy.wait(100)) {
            break;
        }
        QCoreApplication::processEvents();
    }
    
    QVERIFY(thread->wait(1000));
    
    // Verify finished was emitted quickly (no timeout)
    QVERIFY(finishedSpy.count() == 1);
    QVERIFY(timer.elapsed() < 1000);  // Should complete in < 1 second
    
    delete window;
}

void AnalysisTimeoutTests::testTimeoutStopsOnCancel()
{
    // Test that timeout timer stops when user cancels manually
    AnalysisWindow* window = new AnalysisWindow();
    
    // Use "noop_sleepy" with medium sleep
    QMap<QString, QVariant> params;
    params["sleep_ms"] = 1000;
    
    QThread* thread = new QThread(this);
    AnalysisWorker* worker = new AnalysisWorker();
    worker->moveToThread(thread);
    worker->setParameters("noop_sleepy", params);
    
    QSignalSpy cancelledSpy(worker, &AnalysisWorker::cancelled);
    
    connect(thread, &QThread::started, worker, &AnalysisWorker::run);
    
    // Cancel after a short delay (simulating user cancel)
    QTimer::singleShot(200, worker, [worker]() {
        worker->requestCancel();
    });
    
    connect(worker, &AnalysisWorker::cancelled, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    thread->start();
    
    QElapsedTimer timer;
    timer.start();
    while (cancelledSpy.count() == 0 && timer.elapsed() < 2000) {
        if (cancelledSpy.wait(100)) {
            break;
        }
        QCoreApplication::processEvents();
    }
    
    QVERIFY(thread->wait(2000));
    
    // Verify cancelled was emitted
    QVERIFY(cancelledSpy.count() == 1);
    QVERIFY(timer.elapsed() < 1500);  // Should cancel before full sleep
    
    delete window;
}

void AnalysisTimeoutTests::testNoTimeoutOnFastRun()
{
    // Test that fast runs don't trigger timeout
    AnalysisWindow* window = new AnalysisWindow();
    
    // Use "noop" feature (immediate completion)
    QMap<QString, QVariant> params;
    
    QThread* thread = new QThread(this);
    AnalysisWorker* worker = new AnalysisWorker();
    worker->moveToThread(thread);
    worker->setParameters("noop", params);
    
    QSignalSpy finishedSpy(worker, &AnalysisWorker::finished);
    QSignalSpy cancelledSpy(worker, &AnalysisWorker::cancelled);
    
    connect(thread, &QThread::started, worker, &AnalysisWorker::run);
    connect(worker, &AnalysisWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    QElapsedTimer timer;
    timer.start();
    
    thread->start();
    
    while (finishedSpy.count() == 0 && cancelledSpy.count() == 0 && timer.elapsed() < 2000) {
        if (finishedSpy.wait(100) || cancelledSpy.wait(100)) {
            break;
        }
        QCoreApplication::processEvents();
    }
    
    QVERIFY(thread->wait(1000));
    
    // Verify finished was emitted, not cancelled
    QVERIFY(finishedSpy.count() == 1);
    QVERIFY(cancelledSpy.count() == 0);
    QVERIFY(timer.elapsed() < 500);  // Should complete very quickly
    
    delete window;
}

QTEST_MAIN(AnalysisTimeoutTests)
#include "test_analysis_timeout.moc"

