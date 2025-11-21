#pragma once

#include "TransportClient.hpp"
#include <QString>
#include <memory>

// Forward declarations for gRPC types
namespace grpc {
    class Channel;
}

// Note: EchoService::Stub is a nested type, so we can't forward declare it.
// The implementation file will include the full headers.

// TODO: Currently implements TCP-based gRPC channel (localhost:50051).
// UDS support will be added in a later WP2 chunk when Bedrock adds UDS server support.
class GrpcUdsChannel : public TransportClient {
public:
    explicit GrpcUdsChannel(const QString& endpoint = QString());
    ~GrpcUdsChannel() override;

    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    QString backendName() const override;
    
    TransportError lastError() const override { return m_lastError; }
    QString lastErrorString() const override;
    
    // Echo RPC method (for WP2.A sanity test)
    // Returns true on success, false on failure. Logs errors internally.
    bool echo(const QString& message, QString& reply);

private:
    QString endpoint_;
    // Use void* to avoid including headers in the header file
    // Implementation will cast to the actual type
    std::shared_ptr<void> channel_;
    std::unique_ptr<void, void(*)(void*)> stub_;
    bool connected_;
    TransportError m_lastError;
};

