#include "LocalSocketChannel.hpp"

// Include generated proto headers (only when PHX_WITH_TRANSPORT_DEPS=ON)
// CMake will define PHX_WITH_TRANSPORT_DEPS as a compile definition when flag is ON
#ifdef PHX_WITH_TRANSPORT_DEPS
#include "palantir/capabilities.pb.h"
#endif

#include <QLocalSocket>
#include <memory>
#include <QString>
#include <QByteArray>
#include <cstring>

#ifdef PHX_WITH_TRANSPORT_DEPS
// Helper function to get socket path from environment or use default
static QString getSocketPath()
{
    QString envPath = qEnvironmentVariable("PALANTIR_SOCKET_PATH");
    if (!envPath.isEmpty()) {
        return envPath;
    }
    return QStringLiteral("palantir_bedrock");
}
#endif

LocalSocketChannel::LocalSocketChannel()
    : m_socket(std::make_unique<QLocalSocket>())
{
    // Socket will be connected on demand
}

LocalSocketChannel::~LocalSocketChannel() = default;

bool LocalSocketChannel::connect()
{
#ifdef PHX_WITH_TRANSPORT_DEPS
    if (!m_socket) {
        m_socket = std::make_unique<QLocalSocket>();
    }
    
    // If already connected, return true
    if (m_socket->state() == QLocalSocket::ConnectedState) {
        return true;
    }
    
    // Connect to server
    QString socketPath = getSocketPath();
    m_socket->connectToServer(socketPath);
    
    // Wait for connection (5 second timeout)
    if (!m_socket->waitForConnected(5000)) {
        return false;
    }
    
    return true;
#else
    // Transport deps not available
    return false;
#endif
}

void LocalSocketChannel::disconnect()
{
#ifdef PHX_WITH_TRANSPORT_DEPS
    if (m_socket) {
        m_socket->disconnectFromServer();
        if (m_socket->state() != QLocalSocket::UnconnectedState) {
            m_socket->waitForDisconnected(1000);
        }
    }
#endif
}

bool LocalSocketChannel::isConnected() const
{
#ifdef PHX_WITH_TRANSPORT_DEPS
    return m_socket && m_socket->state() == QLocalSocket::ConnectedState;
#else
    return false;
#endif
}

#ifdef PHX_WITH_TRANSPORT_DEPS
std::optional<palantir::CapabilitiesResponse> LocalSocketChannel::getCapabilities(QString* outError)
{
    // Ensure connected
    if (!isConnected() && !connect()) {
        if (outError) {
            *outError = QString("Failed to connect to Bedrock server");
        }
        return std::nullopt;
    }
    
    // Serialize CapabilitiesRequest
    palantir::CapabilitiesRequest request;
    std::string serialized;
    if (!request.SerializeToString(&serialized)) {
        if (outError) {
            *outError = QString("Failed to serialize CapabilitiesRequest");
        }
        return std::nullopt;
    }
    
    // Create length-prefixed frame (4-byte little-endian length + message)
    uint32_t length = static_cast<uint32_t>(serialized.size());
    QByteArray data;
    data.append(reinterpret_cast<const char*>(&length), 4);
    data.append(serialized.data(), static_cast<int>(serialized.size()));
    
    // Send request
    qint64 written = m_socket->write(data);
    if (written != data.size()) {
        if (outError) {
            *outError = QString("Failed to send CapabilitiesRequest");
        }
        return std::nullopt;
    }
    
    // Wait for data to be written
    if (!m_socket->waitForBytesWritten(5000)) {
        if (outError) {
            *outError = QString("Timeout writing CapabilitiesRequest");
        }
        return std::nullopt;
    }
    
    // Wait for response
    if (!m_socket->waitForReadyRead(5000)) {
        if (outError) {
            *outError = QString("Timeout waiting for CapabilitiesResponse");
        }
        return std::nullopt;
    }
    
    // Read length prefix (4 bytes)
    QByteArray lengthBytes = m_socket->read(4);
    if (lengthBytes.size() != 4) {
        if (outError) {
            *outError = QString("Failed to read length prefix");
        }
        return std::nullopt;
    }
    
    uint32_t responseLength;
    std::memcpy(&responseLength, lengthBytes.data(), 4);
    
    // Read message bytes (may need multiple reads)
    QByteArray messageBytes;
    while (messageBytes.size() < static_cast<int>(responseLength)) {
        if (!m_socket->waitForReadyRead(5000)) {
            if (outError) {
                *outError = QString("Timeout reading CapabilitiesResponse");
            }
            return std::nullopt;
        }
        messageBytes += m_socket->read(responseLength - messageBytes.size());
    }
    
    if (messageBytes.size() != static_cast<int>(responseLength)) {
        if (outError) {
            *outError = QString("Failed to read complete CapabilitiesResponse");
        }
        return std::nullopt;
    }
    
    // Parse response
    palantir::CapabilitiesResponse response;
    if (!response.ParseFromArray(messageBytes.data(), messageBytes.size())) {
        if (outError) {
            *outError = QString("Failed to parse CapabilitiesResponse");
        }
        return std::nullopt;
    }
    
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

