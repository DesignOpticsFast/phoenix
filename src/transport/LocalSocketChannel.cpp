#include "LocalSocketChannel.hpp"

LocalSocketChannel::LocalSocketChannel(const QString& socketPath)
    : socketPath_(socketPath.isEmpty() ? QStringLiteral("palantir_bedrock") : socketPath)
    , connected_(false)
{
}

bool LocalSocketChannel::connect() {
    // Stub: always returns false
    return false;
}

void LocalSocketChannel::disconnect() {
    // Stub: no-op
    connected_ = false;
}

bool LocalSocketChannel::isConnected() const {
    return connected_;
}

QString LocalSocketChannel::backendName() const {
    return QStringLiteral("LocalSocket stub");
}

