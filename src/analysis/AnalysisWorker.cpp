#include "AnalysisWorker.hpp"
#include "analysis/demo/XYSineDemo.hpp"
// TODO(Phase 3+): Re-enable license checks when LicenseManager is available
// #include "app/LicenseManager.h"
#include <QDebug>

AnalysisWorker::AnalysisWorker(QObject* parent)
    : QObject(parent)
    , m_cancelRequested(false)
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

