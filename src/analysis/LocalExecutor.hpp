#pragma once

#include "IAnalysisExecutor.hpp"

// Local analysis executor - wraps XYSineDemo for local-only compute
// WP1: Simple wrapper around existing local compute path
// Does NOT modify XYSineDemo or existing worker logic
class LocalExecutor : public IAnalysisExecutor {
public:
    LocalExecutor();
    ~LocalExecutor() override = default;

    // IAnalysisExecutor interface
    void execute(
        const QString& featureId,
        const QMap<QString, QVariant>& params,
        ProgressCallback onProgress,
        ResultCallback onResult,
        ErrorCallback onError
    ) override;

    void cancel() override;

private:
    std::atomic<bool> m_cancelled;
};

