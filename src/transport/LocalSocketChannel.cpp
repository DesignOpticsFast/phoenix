#include "LocalSocketChannel.hpp"

// Include generated proto headers (only when PHX_WITH_TRANSPORT_DEPS=ON)
// CMake will define PHX_WITH_TRANSPORT_DEPS as a compile definition when flag is ON
#ifdef PHX_WITH_TRANSPORT_DEPS
#include "palantir/capabilities.pb.h"
#endif

#include <QLocalSocket>
#include <memory>
#include <QString>

LocalSocketChannel::LocalSocketChannel()
    : m_socket(std::make_unique<QLocalSocket>())
{
    // WP1: No initialization needed yet
    // Future: Set up socket name, connect signals, etc.
}

LocalSocketChannel::~LocalSocketChannel() = default;

bool LocalSocketChannel::connect()
{
    // WP1: Return true for stub implementation (no real socket connection yet)
    // Future: Connect to Bedrock Palantir server via QLocalSocket
    return true;
}

void LocalSocketChannel::disconnect()
{
    // WP1: No-op - no connection to disconnect
    // Future: Close socket, clean up state
    if (m_socket) {
        m_socket->disconnectFromServer();
    }
}

bool LocalSocketChannel::isConnected() const
{
    // WP1: Return true when connect() has been called (stub implementation)
    // Future: Check actual QLocalSocket connection state
    return true;
}

#ifdef PHX_WITH_TRANSPORT_DEPS
std::optional<palantir::CapabilitiesResponse> LocalSocketChannel::getCapabilities(QString* outError)
{
    // WP1: Return stubbed CapabilitiesResponse (matches Bedrock's CapabilitiesService)
    // Future: Send CapabilitiesRequest over QLocalSocket, receive CapabilitiesResponse
    palantir::CapabilitiesResponse response;
    auto* caps = response.mutable_capabilities();
    
    // Match Bedrock's CapabilitiesService response
    caps->set_server_version("bedrock-0.0.1");
    caps->add_supported_features("xy_sine");
    
    return response;
}
#else
std::optional<int> LocalSocketChannel::getCapabilities(QString* outError)
{
    // Transport deps not available
    if (outError) {
        *outError = "Transport dependencies not enabled";
    }
    return std::nullopt;
}
#endif

