#include "LocalSocketChannel.hpp"

// Include generated proto headers (only when PHX_WITH_TRANSPORT_DEPS=ON)
// CMake will define PHX_WITH_TRANSPORT_DEPS as a compile definition when flag is ON
#ifdef PHX_WITH_TRANSPORT_DEPS
#include "palantir/capabilities.pb.h"
#endif

#include <QLocalSocket>
#include <memory>

LocalSocketChannel::LocalSocketChannel()
    : m_socket(std::make_unique<QLocalSocket>())
{
    // WP1: No initialization needed yet
    // Future: Set up socket name, connect signals, etc.
}

LocalSocketChannel::~LocalSocketChannel() = default;

bool LocalSocketChannel::connect()
{
    // WP1: Always return false - no actual connection yet
    // Future: Connect to Bedrock Palantir server via QLocalSocket
    return false;
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
    // WP1: Always return false - no connection capability yet
    return false;
}

std::optional<palantir::CapabilitiesResponse> LocalSocketChannel::getCapabilities(std::string* outError)
{
    // WP1: Return nullopt - no IPC implementation yet
    // Future: Send CapabilitiesRequest, receive CapabilitiesResponse
    if (outError) {
        *outError = "LocalSocketChannel: Remote execution not yet implemented";
    }
#ifdef PHX_WITH_TRANSPORT_DEPS
    // Return empty response when proto is available
    return std::nullopt;
#else
    // Proto not available
    return std::nullopt;
#endif
}

