#include "RemoteExecutor.hpp"

// Include transport client (only when PHX_WITH_TRANSPORT_DEPS=ON)
#ifdef PHX_WITH_TRANSPORT_DEPS
#include "transport/TransportFactory.hpp"
#include "transport/TransportClient.hpp"
// Proto header is in generated directory, included via CMake include paths
#include "palantir/capabilities.pb.h"
#endif

#include <atomic>
#include <QDebug>

RemoteExecutor::RemoteExecutor()
    : m_cancelled(false)
#ifdef PHX_WITH_TRANSPORT_DEPS
    , m_transport(TransportFactory::makeTransportClient(TransportBackend::Auto))
#else
    , m_transport(nullptr)
#endif
{
}

RemoteExecutor::~RemoteExecutor() = default;

void RemoteExecutor::execute(
    const QString& featureId,
    const QMap<QString, QVariant>& params,
    ProgressCallback onProgress,
    ResultCallback onResult,
    ErrorCallback onError)
{
    // Reset cancellation flag
    m_cancelled.store(false);

#ifdef PHX_WITH_TRANSPORT_DEPS
    if (!m_transport) {
        if (onError) {
            onError(QString("Transport client not available"));
        }
        return;
    }
    
    // Connect to remote service
    if (!m_transport->connect()) {
        QString errorMsg = "Unable to connect to remote analysis service";
        if (onError) {
            onError(errorMsg);
        }
        return;
    }
    
    // Check for cancellation
    if (m_cancelled.load()) {
        if (onError) {
            onError(QString("Computation cancelled"));
        }
        return;
    }
    
    // Fetch capabilities (WP1: stub implementation, no real IPC yet)
    QString errorMsg;
    auto capabilities = m_transport->getCapabilities(&errorMsg);
    
    if (!capabilities.has_value()) {
        if (onError) {
            onError(errorMsg.isEmpty() ? QString("Failed to fetch capabilities") : errorMsg);
        }
        return;
    }
    
    // WP1: For now, just log that capabilities were fetched successfully
    // Future: Use capabilities to determine available features, then execute remote computation
    qDebug() << "Capabilities fetched: server_version=" 
             << QString::fromStdString(capabilities->capabilities().server_version())
             << "features=" << capabilities->capabilities().supported_features_size();
    
    // For this chunk, return a message indicating capabilities were fetched
    // (In future chunks, we'll actually execute remote computation)
    if (onError) {
        onError(QString("Remote execution stubbed (capabilities fetched successfully)"));
    }
#else
    // Transport deps not available
    if (onError) {
        onError(QString("Transport dependencies not enabled (PHX_WITH_TRANSPORT_DEPS=OFF)"));
    }
#endif
}

void RemoteExecutor::cancel()
{
    // WP1: Simple flag-based cancellation
    // Future: Cancel ongoing Bedrock requests
    m_cancelled.store(true);
}

