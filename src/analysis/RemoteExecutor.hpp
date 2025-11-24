#pragma once

#include "IAnalysisExecutor.hpp"
#include <memory>

// Include TransportClient header (interface only, no proto deps in header)
// TransportClient uses forward declarations for proto types
#include "transport/TransportClient.hpp"

// Remote analysis executor - uses TransportClient for Bedrock communication
// WP1: Skeleton implementation - returns immediate error
// Future: Will implement actual Bedrock communication, progress, cancellation
class RemoteExecutor : public IAnalysisExecutor {
public:
    RemoteExecutor();
    ~RemoteExecutor() override;

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
    std::unique_ptr<TransportClient> m_transport;
    std::atomic<bool> m_cancelled;
};

