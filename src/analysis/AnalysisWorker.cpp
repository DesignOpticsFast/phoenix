#include "AnalysisWorker.hpp"
#include "transport/LocalSocketChannel.hpp"
#include "app/LicenseManager.h"
#include <QDebug>

AnalysisWorker::AnalysisWorker(QObject* parent)
    : QObject(parent)
    , m_cancelRequested(false)
{
    // Register XYSineResult meta-type for signal/slot passing
    qRegisterMetaType<XYSineResult>("XYSineResult");
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
    // Actual cancellation logic will be implemented in WP3.5.2
}

void AnalysisWorker::executeCompute()
{
    // Handle "noop" feature for tests
    if (m_featureId == "noop") {
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
        
        // Connect
        if (!client->connect()) {
            emit finished(false, QVariant(),
                tr("Failed to connect to Bedrock server.\n\n"
                   "Please ensure Bedrock is running and accessible via LocalSocket."));
            return;
        }
        
        // Check for cancel
        if (m_cancelRequested.load()) {
            client->disconnect();
            emit cancelled();
            emit finished(false, QVariant(), QString());
            return;
        }
        
        // Compute XY Sine
        XYSineResult result;
        if (!client->computeXYSine(m_params, result)) {
            client->disconnect();
            emit finished(false, QVariant(),
                tr("XY Sine computation failed.\n\n"
                   "Please check the server logs for details."));
            return;
        }
        
        client->disconnect();
        
        // Validate result
        if (result.x.size() != result.y.size()) {
            emit finished(false, QVariant(),
                tr("Received invalid data from server (mismatched array sizes)."));
            return;
        }
        
        // Emit success with result
        emit finished(true, QVariant::fromValue(result), QString());
        return;
    }
    
    // Unknown feature
    emit finished(false, QVariant(),
        tr("Unknown feature: %1").arg(m_featureId));
}

