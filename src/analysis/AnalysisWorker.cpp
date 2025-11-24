#include "AnalysisWorker.hpp"
#include "analysis/demo/XYSineDemo.hpp"
#include "analysis/LocalExecutor.hpp"
#include "analysis/RemoteExecutor.hpp"
// TODO(Phase 3+): Re-enable license checks when LicenseManager is available
// #include "app/LicenseManager.h"
#include <QDebug>

AnalysisWorker::AnalysisWorker(QObject* parent)
    : QObject(parent)
    , m_cancelRequested(false)
    , m_runMode(AnalysisRunMode::LocalOnly)
    , m_localExecutor(std::make_unique<LocalExecutor>())
    , m_remoteExecutor(std::make_unique<RemoteExecutor>())
{
    // Register XYSineResult meta-type for signal/slot passing
    qRegisterMetaType<XYSineResult>("XYSineResult");
    qRegisterMetaType<XYSineResult>("XYSineResult&");
}

AnalysisWorker::~AnalysisWorker() = default;

void AnalysisWorker::setParameters(const QString& featureId, const QMap<QString, QVariant>& params)
{
    m_featureId = featureId;
    m_params = params;
}

void AnalysisWorker::setRunMode(AnalysisRunMode mode)
{
    m_runMode = mode;
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
    
    // WP1: Use executor pattern if enabled, otherwise fall back to legacy path
    if (m_runMode == AnalysisRunMode::LocalOnly || m_runMode == AnalysisRunMode::RemoteOnly) {
        executeWithExecutor();
    } else {
        // Fallback to legacy executeCompute (should not happen with current enum)
        executeCompute();
    }
}

void AnalysisWorker::requestCancel()
{
    m_cancelRequested.store(true);
    // Cancel executors
    if (m_localExecutor) {
        m_localExecutor->cancel();
    }
    if (m_remoteExecutor) {
        m_remoteExecutor->cancel();
    }
    // Actual cancellation logic will be implemented in WP3.5.2
}

void AnalysisWorker::executeWithExecutor()
{
    // Select executor based on run mode
    IAnalysisExecutor* executor = nullptr;
    if (m_runMode == AnalysisRunMode::LocalOnly) {
        executor = m_localExecutor.get();
    } else if (m_runMode == AnalysisRunMode::RemoteOnly) {
        executor = m_remoteExecutor.get();
    }
    
    if (!executor) {
        emit finished(false, QVariant(), QString("No executor available"));
        return;
    }
    
    // Execute with callbacks
    executor->execute(
        m_featureId,
        m_params,
        // Progress callback
        [this](double progress) {
            // WP1: No progress reporting yet
            // Future: emit progress signal
            Q_UNUSED(progress);
        },
        // Result callback
        [this](const XYSineResult& result) {
            emit finished(true, QVariant::fromValue(result), QString());
        },
        // Error callback
        [this](const QString& error) {
            emit finished(false, QVariant(), error);
        }
    );
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
        // TODO(Phase 3+): Re-enable license checks when LicenseManager is available
        /*
        LicenseManager* mgr = LicenseManager::instance();
        if (mgr->currentState() != LicenseManager::LicenseState::NotConfigured &&
            !mgr->hasFeature("feature_xy_plot")) {
            emit finished(false, QVariant(), 
                tr("XY Sine computation requires a valid license with the 'feature_xy_plot' feature."));
            return;
        }
        */
        
        // Check for cancel before compute
        if (m_cancelRequested.load()) {
            emit cancelled();
            emit finished(false, QVariant(), QString());
            return;
        }
        
        // Compute XY Sine locally using XYSineDemo
        XYSineResult result;
        if (!XYSineDemo::compute(m_params, result)) {
            emit finished(false, QVariant(),
                tr("XY Sine computation failed.\n\n"
                   "Please check the parameters and try again."));
            return;
        }
        
        // Check for cancel after compute
        if (m_cancelRequested.load()) {
            emit cancelled();
            emit finished(false, QVariant(), QString());
            return;
        }
        
        // Validate result
        if (result.x.size() != result.y.size()) {
            emit finished(false, QVariant(),
                tr("Computation produced invalid data (mismatched array sizes)."));
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

