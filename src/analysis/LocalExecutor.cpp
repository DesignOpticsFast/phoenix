#include "LocalExecutor.hpp"
#include "analysis/demo/XYSineDemo.hpp"
#include <atomic>
#include <QDebug>

LocalExecutor::LocalExecutor()
    : m_cancelled(false)
{
}

void LocalExecutor::execute(
    const QString& featureId,
    const QMap<QString, QVariant>& params,
    ProgressCallback onProgress,
    ResultCallback onResult,
    ErrorCallback onError)
{
    // Reset cancellation flag
    m_cancelled.store(false);

    // Handle "noop" feature for tests
    if (featureId == "noop") {
        // Return immediately with dummy success
        if (onProgress) {
            onProgress(1.0);
        }
        // No result for noop - executor should handle this
        return;
    }

    // Handle "xy_sine" feature
    if (featureId == "xy_sine") {
        // Check for cancellation
        if (m_cancelled.load()) {
            if (onError) {
                onError(QString("Computation cancelled"));
            }
            return;
        }

        // Report progress start
        if (onProgress) {
            onProgress(0.0);
        }

        // Compute XY Sine locally using XYSineDemo
        XYSineResult result;
        if (!XYSineDemo::compute(params, result)) {
            if (onError) {
                onError(QString("XY Sine computation failed.\n\n"
                               "Please check the parameters and try again."));
            }
            return;
        }

        // Check for cancellation after compute
        if (m_cancelled.load()) {
            if (onError) {
                onError(QString("Computation cancelled"));
            }
            return;
        }

        // Validate result
        if (result.x.size() != result.y.size()) {
            if (onError) {
                onError(QString("Computation produced invalid data (mismatched array sizes)."));
            }
            return;
        }

        // Report progress complete
        if (onProgress) {
            onProgress(1.0);
        }

        // Emit success with result
        if (onResult) {
            onResult(result);
        }
        return;
    }

    // Unknown feature
    if (onError) {
        onError(QString("Unknown feature: %1").arg(featureId));
    }
}

void LocalExecutor::cancel()
{
    // WP1: Simple flag-based cancellation
    // Future: May need to interrupt XYSineDemo if it supports cancellation
    m_cancelled.store(true);
}

