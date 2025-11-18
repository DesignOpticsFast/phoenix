#include "GrpcUdsChannel.hpp"

GrpcUdsChannel::GrpcUdsChannel(const QString& socketPath)
    : socketPath_(socketPath.isEmpty() ? QStringLiteral("palantir_bedrock") : socketPath)
    , connected_(false)
{
}

bool GrpcUdsChannel::connect() {
    // Stub: always returns false
    return false;
}

void GrpcUdsChannel::disconnect() {
    // Stub: no-op
    connected_ = false;
}

bool GrpcUdsChannel::isConnected() const {
    return connected_;
}

QString GrpcUdsChannel::backendName() const {
    return QStringLiteral("gRPC (UDS) stub");
}

