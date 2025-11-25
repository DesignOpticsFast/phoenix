#include "LocalSocketChannel.hpp"

// Include generated proto headers (only when PHX_WITH_TRANSPORT_DEPS=ON)
// CMake will define PHX_WITH_TRANSPORT_DEPS as a compile definition when flag is ON
#ifdef PHX_WITH_TRANSPORT_DEPS
#include "palantir/capabilities.pb.h"
#include "palantir/envelope.pb.h"
#include "EnvelopeHelpers.hpp"
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
    
    // Create CapabilitiesRequest and wrap in envelope
    palantir::CapabilitiesRequest request;
    QString envelopeError;
    auto envelope = phoenix::transport::makeEnvelope(
        palantir::MessageType::CAPABILITIES_REQUEST,
        request,
        {},
        &envelopeError
    );
    
    if (!envelope.has_value()) {
        if (outError) {
            *outError = QString("Failed to create envelope: %1").arg(envelopeError);
        }
        return std::nullopt;
    }
    
    // Serialize envelope
    std::string serialized;
    if (!envelope->SerializeToString(&serialized)) {
        if (outError) {
            *outError = QString("Failed to serialize MessageEnvelope");
        }
        return std::nullopt;
    }
    
    // Create length-prefixed frame (4-byte little-endian length + serialized envelope)
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
    
    // Read envelope bytes (may need multiple reads)
    QByteArray envelopeBytes;
    while (envelopeBytes.size() < static_cast<int>(responseLength)) {
        if (!m_socket->waitForReadyRead(5000)) {
            if (outError) {
                *outError = QString("Timeout reading MessageEnvelope");
            }
            return std::nullopt;
        }
        envelopeBytes += m_socket->read(responseLength - envelopeBytes.size());
    }
    
    if (envelopeBytes.size() != static_cast<int>(responseLength)) {
        if (outError) {
            *outError = QString("Failed to read complete MessageEnvelope");
        }
        return std::nullopt;
    }
    
    // Parse envelope
    palantir::MessageEnvelope responseEnvelope;
    QString parseError;
    if (!phoenix::transport::parseEnvelope(envelopeBytes, responseEnvelope, &parseError)) {
        if (outError) {
            *outError = QString("Failed to parse MessageEnvelope: %1").arg(parseError);
        }
        return std::nullopt;
    }
    
    // Validate envelope
    if (responseEnvelope.version() != 1) {
        if (outError) {
            *outError = QString("Invalid envelope version: %1").arg(responseEnvelope.version());
        }
        return std::nullopt;
    }
    
    if (responseEnvelope.type() != palantir::MessageType::CAPABILITIES_RESPONSE) {
        if (outError) {
            *outError = QString("Unexpected message type: %1").arg(static_cast<int>(responseEnvelope.type()));
        }
        return std::nullopt;
    }
    
    // Parse inner CapabilitiesResponse from payload
    palantir::CapabilitiesResponse response;
    const std::string& payload = responseEnvelope.payload();
    if (!response.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
        if (outError) {
            *outError = QString("Failed to parse CapabilitiesResponse from envelope payload");
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

