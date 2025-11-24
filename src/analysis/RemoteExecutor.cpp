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
    
    // Fetch capabilities from remote server
    QString errorMsg;
    auto capabilities = m_transport->getCapabilities(&errorMsg);
    
    if (!capabilities.has_value()) {
        if (onError) {
            onError(errorMsg.isEmpty() ? QString("Failed to fetch capabilities") : errorMsg);
        }
        return;
    }
    
    // Log capabilities for debugging
    qDebug() << "Capabilities fetched: server_version=" 
             << QString::fromStdString(capabilities->capabilities().server_version())
             << "features=" << capabilities->capabilities().supported_features_size();
    
    // Check if requested feature is supported
    QString requestedFeature = featureId;
    bool featureSupported = false;
    for (int i = 0; i < capabilities->capabilities().supported_features_size(); ++i) {
        if (QString::fromStdString(capabilities->capabilities().supported_features(i)) == requestedFeature) {
            featureSupported = true;
            break;
        }
    }
    
    if (!featureSupported) {
        if (onError) {
            onError(QString("Feature '%1' not supported by server").arg(requestedFeature));
        }
        return;
    }
    
    // WP1: Capabilities verified, but remote computation not yet implemented
    // Future: Execute actual remote computation based on featureId and params
    if (onError) {
        onError(QString("Remote execution for '%1' not yet implemented (capabilities verified)").arg(requestedFeature));
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

