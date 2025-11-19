#include "AnalysisWorker.hpp"
#include "transport/LocalSocketChannel.hpp"
#include "app/LicenseManager.h"
#include <QDebug>

AnalysisWorker::AnalysisWorker(QObject* parent)
    : QObject(parent)
    , m_cancelRequested(false)
    , m_currentClient(nullptr)
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
    
    // If we have an active client and job, send cancel to Bedrock
    if (m_currentClient && !m_currentJobId.isEmpty()) {
        m_currentClient->cancelJob(m_currentJobId);
        qDebug() << "AnalysisWorker::requestCancel: Sent cancel for job" << m_currentJobId;
    }
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
    
    // Handle "xy_sine" feature
    if (m_featureId == "xy_sine") {
        // Check license (in worker thread - this is OK for LicenseManager)
        LicenseManager* mgr = LicenseManager::instance();
        if (mgr->currentState() != LicenseManager::LicenseState::NotConfigured &&
            !mgr->hasFeature("feature_xy_plot")) {
            emit finished(false, QVariant(), 
                tr("XY Sine computation requires a valid license with the 'feature_xy_plot' feature."));
            return;
        }
        
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
    }
    
    // Unknown feature
    emit finished(false, QVariant(),
        tr("Unknown feature: %1").arg(m_featureId));
}

