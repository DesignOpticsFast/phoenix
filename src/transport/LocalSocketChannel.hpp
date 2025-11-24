#pragma once

#include "TransportClient.hpp"
#include <QLocalSocket>
#include <memory>

// Local socket transport channel (skeleton for WP1)
// WP1: Minimal implementation - no actual IPC yet
// Future: Will implement Palantir IPC protocol over QLocalSocket
class LocalSocketChannel : public TransportClient {
public:
    LocalSocketChannel();
    ~LocalSocketChannel() override;

    // TransportClient interface
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    std::optional<palantir::CapabilitiesResponse>
        getCapabilities(std::string* outError = nullptr) override;

private:
    std::unique_ptr<QLocalSocket> m_socket;
    // TODO(WP2+): Add protocol framing, message dispatch, state machine
};

