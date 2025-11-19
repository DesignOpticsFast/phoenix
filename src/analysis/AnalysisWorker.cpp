#include "AnalysisWorker.hpp"
#include "analysis/AnalysisResults.hpp"  // For XYSineResult
#include "analysis/demo/XYSineDemo.hpp"
#include "app/LicenseManager.h"
#include "analysis/AnalysisProgress.hpp"
#ifdef PHX_WITH_TRANSPORT_DEPS
#include "transport/LocalSocketChannel.hpp"
#endif
#include <QDebug>
#include <QThread>
#include <QProcessEnvironment>
#include <QVariant>
#include <QMap>
#include <QString>
#include <atomic>
#include <memory>

AnalysisWorker::AnalysisWorker(QObject* parent)
    : QObject(parent)
    , m_cancelRequested(false)
#ifdef PHX_WITH_TRANSPORT_DEPS
    , m_currentClient(nullptr)
#endif
{
    // Register meta-types for signal/slot passing
    qRegisterMetaType<XYSineResult>("XYSineResult");
    qRegisterMetaType<AnalysisProgress>("AnalysisProgress");
}

AnalysisWorker::~AnalysisWorker() = default;

void AnalysisWorker::setParameters(const QString& featureId, const QMap<QString, QVariant>& params)
{
    m_featureId = featureId;
    m_params = params;
}

void AnalysisWorker::run()
{
    emit started();
    
    // Check for cancel (placeholder for WP3.5.2)
    if (m_cancelRequested.load()) {
        emit cancelled();
        emit finished(false, QVariant(), QString());
        return;
    }
    
    executeCompute();
}

void AnalysisWorker::requestCancel()
{
    m_cancelRequested.store(true);
    
#ifdef PHX_WITH_TRANSPORT_DEPS
    // If we have an active client and job, send cancel to Bedrock
    if (m_currentClient && !m_currentJobId.isEmpty()) {
        m_currentClient->cancelJob(m_currentJobId);
        qDebug() << "AnalysisWorker::requestCancel: Sent cancel for job" << m_currentJobId;
    }
#endif
}

// Check if demo mode is enabled (PHOENIX_DEMO_MODE=1)
static bool isDemoModeEnabled()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    return env.value("PHOENIX_DEMO_MODE") == "1";
}

void AnalysisWorker::executeCompute()
{
    // Handle "noop" feature for tests
    if (m_featureId == "noop") {
        // Emit progress updates
        emit progressChanged(AnalysisProgress(0.0, tr("Starting...")));
        emit progressChanged(AnalysisProgress(100.0, tr("Done")));
        // Return immediately with dummy success
        emit finished(true, QVariant(), QString());
        return;
    }
    
    // Handle "noop_sleepy" feature for timeout tests
    if (m_featureId == "noop_sleepy") {
        // Sleepy version for timeout tests - sleeps for a configurable duration
        emit progressChanged(AnalysisProgress(0.0, tr("Sleeping...")));
        
        // Check for cancel before sleep
        if (m_cancelRequested.load()) {
            emit cancelled();
            return;
        }
        
        // Sleep for test duration (default 1000ms, can be overridden via params)
        int sleepMs = 1000;
        if (m_params.contains("sleep_ms")) {
            bool ok;
            int val = m_params.value("sleep_ms").toInt(&ok);
            if (ok && val > 0) {
                sleepMs = val;
            }
        }
        
        QThread::msleep(sleepMs);
        
        // Check for cancel after sleep
        if (m_cancelRequested.load()) {
            emit cancelled();
            return;
        }
        
        emit progressChanged(AnalysisProgress(100.0, tr("Done")));
        emit finished(true, QVariant(), QString());
        return;
    }
    
    // Handle "xy_sine" feature
    if (m_featureId == "xy_sine") {
        // Check license (in worker thread - this is OK for LicenseManager)
        // License check applies even in demo mode - licensing is core, not optional
        LicenseManager* mgr = LicenseManager::instance();
        if (mgr->currentState() != LicenseManager::LicenseState::NotConfigured &&
            !mgr->hasFeature("feature_xy_plot")) {
            emit finished(false, QVariant(), 
                tr("XY Sine computation requires a valid license with the 'feature_xy_plot' feature."));
            return;
        }
        
        // TODO(Sprint 4.4): Demo mode is temporary for Mac testing - remove once permanent solution exists
        // Check if demo mode is enabled (PHOENIX_DEMO_MODE=1)
        if (isDemoModeEnabled()) {
            // Local compute path (demo mode)
            emit progressChanged(AnalysisProgress(0.0, tr("Computing locally...")));
            
            // Check for cancel before compute
            if (m_cancelRequested.load()) {
                emit cancelled();
                return;
            }
            
            // Compute locally using demo helper
            XYSineResult result;
            if (!XYSineDemo::compute(m_params, result)) {
                emit finished(false, QVariant(),
                    tr("Local XY Sine computation failed."));
                return;
            }
            
            // Check for cancel after compute
            if (m_cancelRequested.load()) {
                emit cancelled();
                return;
            }
            
            // Emit progress
            emit progressChanged(AnalysisProgress(50.0, tr("Processing...")));
            
            // Small delay to show progress (local compute is very fast)
            QThread::msleep(10);
            
            // Check for cancel before emitting finished
            if (m_cancelRequested.load()) {
                emit cancelled();
                return;
            }
            
            emit progressChanged(AnalysisProgress(100.0, tr("Done")));
            
            // Emit success with result
            emit finished(true, QVariant::fromValue(result), QString());
            return;
        }
        
        // Original Bedrock path (unchanged when demo mode is off)
#ifdef PHX_WITH_TRANSPORT_DEPS
        // Create LocalSocketChannel
        auto client = std::make_unique<LocalSocketChannel>();
        m_currentClient = client.get();
        m_currentJobId.clear();
        
        // Emit initial progress
        emit progressChanged(AnalysisProgress(0.0, tr("Connecting...")));
        
        // Connect
        if (!client->connect()) {
            m_currentClient = nullptr;
            emit finished(false, QVariant(),
                tr("Failed to connect to Bedrock server.\n\n"
                   "Please ensure Bedrock is running and accessible via LocalSocket."));
            return;
        }
        
        // Check for cancel
        if (m_cancelRequested.load()) {
            client->disconnect();
            m_currentClient = nullptr;
            emit cancelled();
            return;
        }
        
        // Emit progress before compute
        emit progressChanged(AnalysisProgress(25.0, tr("Computing...")));
        
        // Set up progress callback (check for CANCELLED status)
        auto progressCallback = [this](double percent, const QString& status) {
            // Check if Bedrock sent CANCELLED status
            if (status == "CANCELLED") {
                m_cancelRequested.store(true);
                emit cancelled();
                return;
            }
            
            AnalysisProgress progress(percent, status);
            emit progressChanged(progress);
        };
        
        // Compute XY Sine
        XYSineResult result;
        if (!client->computeXYSine(m_params, result, progressCallback)) {
            // Store job ID before disconnect (for potential cancel)
            m_currentJobId = client->currentJobId();
            
            client->disconnect();
            m_currentClient = nullptr;
            
            // Check if cancelled during compute
            if (m_cancelRequested.load()) {
                emit cancelled();
                return;
            }
            
            emit finished(false, QVariant(),
                tr("XY Sine computation failed.\n\n"
                   "Please check the server logs for details."));
            return;
        }
        
        // Store job ID for potential cancel
        m_currentJobId = client->currentJobId();
        
        // Check for cancel after compute
        if (m_cancelRequested.load()) {
            client->disconnect();
            m_currentClient = nullptr;
            emit cancelled();
            return;
        }
        
        client->disconnect();
        m_currentClient = nullptr;
        
        // Validate result
        if (result.x.size() != result.y.size()) {
            emit finished(false, QVariant(),
                tr("Received invalid data from server (mismatched array sizes)."));
            return;
        }
        
        // Emit final progress
        emit progressChanged(AnalysisProgress(100.0, tr("Done")));
        
        // Emit success with result
        emit finished(true, QVariant::fromValue(result), QString());
        return;
#else
        // Transport unavailable - fail gracefully
        emit finished(false, QVariant(),
            tr("Transport is not available in this build.\n\n"
               "Please enable demo mode (PHOENIX_DEMO_MODE=1) to use local computation."));
        return;
#endif
    }
    
    // Unknown feature
    emit finished(false, QVariant(),
        tr("Unknown feature: %1").arg(m_featureId));
}

