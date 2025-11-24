#include "RemoteExecutor.hpp"

// Include transport client (only when PHX_WITH_TRANSPORT_DEPS=ON)
#ifdef PHX_WITH_TRANSPORT_DEPS
#include "transport/TransportFactory.hpp"
#include "transport/TransportClient.hpp"
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
    // WP1: Return immediate error - no Bedrock communication yet
    if (onError) {
        onError(QString("Remote execution not yet implemented"));
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

