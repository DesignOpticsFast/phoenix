#pragma once

#include <QMap>
#include <QVariant>
#include <QString>
#include <functional>

// Forward declaration
struct XYSineResult;

// Analysis executor interface (Strategy pattern)
// WP1: Minimal interface for local and remote execution paths
// Future: Will expand with progress reporting, cancellation, etc.
class IAnalysisExecutor {
public:
    using ProgressCallback = std::function<void(double)>;  // progress 0.0-1.0
    using ResultCallback = std::function<void(const XYSineResult&)>;
    using ErrorCallback = std::function<void(const QString&)>;

    virtual ~IAnalysisExecutor() = default;

    // Execute analysis with given parameters
    // Parameters:
    //   - featureId: Feature identifier (e.g., "xy_sine")
    //   - params: Feature parameters (QMap<QString, QVariant>)
    //   - onProgress: Progress callback (optional, may be nullptr)
    //   - onResult: Success callback (required)
    //   - onError: Error callback (required)
    virtual void execute(
        const QString& featureId,
        const QMap<QString, QVariant>& params,
        ProgressCallback onProgress,
        ResultCallback onResult,
        ErrorCallback onError
    ) = 0;

    // Cancel ongoing execution
    // WP1: Placeholder - actual cancellation in later sprints
    virtual void cancel() = 0;
};

