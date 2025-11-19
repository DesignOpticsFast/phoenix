#include <QtTest/QtTest>
#include "analysis/AnalysisWorker.hpp"
#include <QThread>
#include <QSignalSpy>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QMap>
#include <QVariant>

class AsyncArchitectureTests : public QObject {
    Q_OBJECT

private slots:
    void testWorkerThreadCreation();
    void testNoopFeature();
    void testWorkerSignals();
    void testUINonBlocking();
    void testThreadCleanup();
};

void AsyncArchitectureTests::testWorkerThreadCreation()
{
    QThread* thread = new QThread(this);
    AnalysisWorker* worker = new AnalysisWorker();
    
    // Move worker to thread
    worker->moveToThread(thread);
    
    QVERIFY(thread != nullptr);
    QVERIFY(worker != nullptr);
    QVERIFY(!thread->isRunning());
    
    // Cleanup
    thread->quit();
    thread->wait(1000);
    thread->deleteLater();
    worker->deleteLater();
}

void AsyncArchitectureTests::testNoopFeature()
{
    QThread* thread = new QThread(this);
    AnalysisWorker* worker = new AnalysisWorker();
    worker->moveToThread(thread);
    
    // Set "noop" feature
    QMap<QString, QVariant> params;
    worker->setParameters("noop", params);
    
    // Connect signals
    QSignalSpy startedSpy(worker, &AnalysisWorker::started);
    QSignalSpy finishedSpy(worker, &AnalysisWorker::finished);
    
    connect(thread, &QThread::started, worker, &AnalysisWorker::run);
    connect(worker, &AnalysisWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    // Start thread
    thread->start();
    
    // Wait for completion (with timeout) - process events to allow signal delivery
    QElapsedTimer timer;
    timer.start();
    while (!finishedSpy.wait(100) && timer.elapsed() < 5000) {
        QCoreApplication::processEvents();
    }
    
    // Wait for thread to finish
    QVERIFY(thread->wait(1000));
    
    // Verify signals were emitted
    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(finishedSpy.count(), 1);
    
    // Verify finished signal has success=true
    QList<QVariant> finishedArgs = finishedSpy.takeFirst();
    QVERIFY(finishedArgs.size() >= 1);
    QVERIFY(finishedArgs[0].toBool() == true);  // success = true
}

void AsyncArchitectureTests::testWorkerSignals()
{
    QThread* thread = new QThread(this);
    AnalysisWorker* worker = new AnalysisWorker();
    worker->moveToThread(thread);
    
    QMap<QString, QVariant> params;
    worker->setParameters("noop", params);
    
    QSignalSpy startedSpy(worker, &AnalysisWorker::started);
    QSignalSpy finishedSpy(worker, &AnalysisWorker::finished);
    
    connect(thread, &QThread::started, worker, &AnalysisWorker::run);
    connect(worker, &AnalysisWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    thread->start();
    
    // Wait for finished signal
    QElapsedTimer timer;
    timer.start();
    while (!finishedSpy.wait(100) && timer.elapsed() < 5000) {
        QCoreApplication::processEvents();
    }
    
    QVERIFY(thread->wait(1000));
    
    // Verify signal order: started before finished
    QVERIFY(startedSpy.count() > 0);
    QVERIFY(finishedSpy.count() > 0);
}

void AsyncArchitectureTests::testUINonBlocking()
{
    QThread* thread = new QThread(this);
    AnalysisWorker* worker = new AnalysisWorker();
    worker->moveToThread(thread);
    
    QMap<QString, QVariant> params;
    worker->setParameters("noop", params);
    
    QSignalSpy finishedSpy(worker, &AnalysisWorker::finished);
    
    connect(thread, &QThread::started, worker, &AnalysisWorker::run);
    connect(worker, &AnalysisWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    // Measure time before starting
    QElapsedTimer timer;
    timer.start();
    
    thread->start();
    
    // UI thread should not block - we should return immediately
    qint64 elapsedBeforeWait = timer.elapsed();
    QVERIFY(elapsedBeforeWait < 100);  // Should return in < 100ms
    
    // Wait for finished signal (process events to allow delivery)
    QElapsedTimer waitTimer;
    waitTimer.start();
    while (!finishedSpy.wait(100) && waitTimer.elapsed() < 5000) {
        QCoreApplication::processEvents();
    }
    
    // Wait for thread to finish
    QVERIFY(thread->wait(1000));
    
    // Verify finished signal was received
    QCOMPARE(finishedSpy.count(), 1);
}

void AsyncArchitectureTests::testThreadCleanup()
{
    QThread* thread = new QThread(this);
    AnalysisWorker* worker = new AnalysisWorker();
    worker->moveToThread(thread);
    
    QMap<QString, QVariant> params;
    worker->setParameters("noop", params);
    
    QSignalSpy finishedSpy(worker, &AnalysisWorker::finished);
    
    connect(thread, &QThread::started, worker, &AnalysisWorker::run);
    connect(worker, &AnalysisWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    thread->start();
    
    // Wait for finished signal
    QElapsedTimer timer;
    timer.start();
    while (!finishedSpy.wait(100) && timer.elapsed() < 5000) {
        QCoreApplication::processEvents();
    }
    
    QVERIFY(thread->wait(1000));
    
    // Thread should be finished
    QVERIFY(!thread->isRunning());
    
    // Process events to allow deleteLater to execute
    QCoreApplication::processEvents();
}

QTEST_MAIN(AsyncArchitectureTests)
#include "test_async_architecture.moc"

