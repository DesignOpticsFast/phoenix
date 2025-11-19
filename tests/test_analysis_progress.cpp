#include <QtTest/QtTest>
#include "analysis/AnalysisWorker.hpp"
#include "analysis/AnalysisProgress.hpp"
#include <QThread>
#include <QSignalSpy>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QMap>
#include <QVariant>

class AnalysisProgressTests : public QObject {
    Q_OBJECT

private slots:
    void testProgressSignalEmitted();
    void testProgressValues();
    void testProgressMetaType();
};

void AnalysisProgressTests::testProgressSignalEmitted()
{
    QThread* thread = new QThread(this);
    AnalysisWorker* worker = new AnalysisWorker();
    worker->moveToThread(thread);
    
    // Set "noop" feature
    QMap<QString, QVariant> params;
    worker->setParameters("noop", params);
    
    // Connect signals
    QSignalSpy progressSpy(worker, &AnalysisWorker::progressChanged);
    QSignalSpy finishedSpy(worker, &AnalysisWorker::finished);
    
    connect(thread, &QThread::started, worker, &AnalysisWorker::run);
    connect(worker, &AnalysisWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    thread->start();
    
    // Wait for finished signal
    QElapsedTimer timer;
    timer.start();
    while (finishedSpy.count() == 0 && timer.elapsed() < 5000) {
        if (finishedSpy.wait(100)) {
            break;
        }
        QCoreApplication::processEvents();
    }
    
    QVERIFY(thread->wait(1000));
    
    // Verify progress signals were emitted
    QVERIFY(progressSpy.count() >= 1);
    QVERIFY(finishedSpy.count() == 1);
}

void AnalysisProgressTests::testProgressValues()
{
    QThread* thread = new QThread(this);
    AnalysisWorker* worker = new AnalysisWorker();
    worker->moveToThread(thread);
    
    QMap<QString, QVariant> params;
    worker->setParameters("noop", params);
    
    QSignalSpy progressSpy(worker, &AnalysisWorker::progressChanged);
    
    connect(thread, &QThread::started, worker, &AnalysisWorker::run);
    connect(worker, &AnalysisWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    thread->start();
    
    // Wait for finished
    QElapsedTimer timer;
    timer.start();
    QSignalSpy finishedSpy(worker, &AnalysisWorker::finished);
    while (finishedSpy.count() == 0 && timer.elapsed() < 5000) {
        if (finishedSpy.wait(100)) {
            break;
        }
        QCoreApplication::processEvents();
    }
    
    QVERIFY(thread->wait(1000));
    
    // Verify progress values are reasonable
    QVERIFY(progressSpy.count() >= 1);
    
    // Check that we have at least 0% and 100%
    bool hasZero = false;
    bool hasHundred = false;
    
    for (int i = 0; i < progressSpy.count(); ++i) {
        QList<QVariant> args = progressSpy[i];
        if (args.size() >= 1) {
            AnalysisProgress progress = args[0].value<AnalysisProgress>();
            if (progress.progressPercent == 0.0) {
                hasZero = true;
            }
            if (progress.progressPercent == 100.0) {
                hasHundred = true;
            }
            // Verify percent is in valid range
            QVERIFY(progress.progressPercent >= 0.0);
            QVERIFY(progress.progressPercent <= 100.0);
            // Verify status is not empty
            QVERIFY(!progress.status.isEmpty());
        }
    }
    
    QVERIFY(hasZero);
    QVERIFY(hasHundred);
}

void AnalysisProgressTests::testProgressMetaType()
{
    // Test that AnalysisProgress can be stored in QVariant
    AnalysisProgress progress(50.0, "Testing...");
    QVariant variant = QVariant::fromValue(progress);
    
    QVERIFY(variant.isValid());
    QVERIFY(variant.canConvert<AnalysisProgress>());
    
    AnalysisProgress retrieved = variant.value<AnalysisProgress>();
    QCOMPARE(retrieved.progressPercent, 50.0);
    QCOMPARE(retrieved.status, QString("Testing..."));
}

QTEST_MAIN(AnalysisProgressTests)
#include "test_analysis_progress.moc"

