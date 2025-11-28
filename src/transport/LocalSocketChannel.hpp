#pragma once

#include "TransportClient.hpp"
#include <QLocalSocket>
#include <memory>

#ifdef PHX_WITH_TRANSPORT_DEPS
#include "palantir/xysine.pb.h"
#include "palantir/error.pb.h"
#endif

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
#ifdef PHX_WITH_TRANSPORT_DEPS
    std::optional<palantir::CapabilitiesResponse>
        getCapabilities(QString* outError = nullptr) override;
    
    // XY Sine RPC (Sprint 4.5)
    std::optional<palantir::XYSineResponse>
        sendXYSineRequest(const palantir::XYSineRequest& request, QString* outError = nullptr);
#else
    std::optional<int> getCapabilities(QString* outError = nullptr) override;
#endif

private:
    std::unique_ptr<QLocalSocket> m_socket;
    // TODO(WP2+): Add protocol framing, message dispatch, state machine
    
    // Constants
#ifdef PHX_WITH_TRANSPORT_DEPS
    static constexpr uint32_t MAX_MESSAGE_SIZE = 10 * 1024 * 1024; // 10MB - matches Bedrock limit
#endif
    
    // Error mapping helper (normalized error semantics)
#ifdef PHX_WITH_TRANSPORT_DEPS
    static QString mapErrorResponse(const palantir::ErrorResponse& errorResponse);
#endif
};

