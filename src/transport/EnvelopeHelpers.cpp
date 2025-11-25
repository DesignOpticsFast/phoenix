#include "EnvelopeHelpers.hpp"

#ifdef PHX_WITH_TRANSPORT_DEPS

#include <optional>
#include <string>

namespace phoenix::transport {

std::optional<palantir::MessageEnvelope> makeEnvelope(
    palantir::MessageType type,
    const google::protobuf::Message& innerMessage,
    const std::map<std::string, std::string>& metadata,
    QString* outError)
{
    // Serialize inner message
    std::string serializedPayload;
    if (!innerMessage.SerializeToString(&serializedPayload)) {
        if (outError) {
            *outError = "Failed to serialize inner message";
        }
        return std::nullopt;
    }
    
    // Create envelope
    palantir::MessageEnvelope envelope;
    envelope.set_version(PROTOCOL_VERSION);
    envelope.set_type(type);
    envelope.set_payload(serializedPayload);
    
    // Set metadata
    for (const auto& [key, value] : metadata) {
        (*envelope.mutable_metadata())[key] = value;
    }
    
    return envelope;
}

bool parseEnvelope(
    const QByteArray& buffer,
    palantir::MessageEnvelope& outEnvelope,
    QString* outError)
{
    if (buffer.isEmpty()) {
        if (outError) {
            *outError = "Empty buffer";
        }
        return false;
    }
    
    // Parse envelope from buffer
    if (!outEnvelope.ParseFromArray(buffer.data(), buffer.size())) {
        if (outError) {
            *outError = "Failed to parse MessageEnvelope";
        }
        return false;
    }
    
    // Validate version
    if (outEnvelope.version() != PROTOCOL_VERSION) {
        if (outError) {
            *outError = QString("Invalid protocol version: %1 (expected %2)")
                       .arg(outEnvelope.version())
                       .arg(PROTOCOL_VERSION);
        }
        return false;
    }
    
    // Validate type (check if it's in valid enum range)
    // MessageType enum values: 0-11 are defined, 12-255 are reserved
    int typeValue = static_cast<int>(outEnvelope.type());
    if (typeValue < 0 || typeValue > 255) {
        if (outError) {
            *outError = QString("Invalid MessageType value: %1").arg(typeValue);
        }
        return false;
    }
    
    // Check for UNSPECIFIED type (0) - this is reserved and should not be used
    if (outEnvelope.type() == palantir::MessageType::MESSAGE_TYPE_UNSPECIFIED) {
        if (outError) {
            *outError = "MessageType is UNSPECIFIED (invalid)";
        }
        return false;
    }
    
    // Payload is optional (can be empty for some message types)
    // No validation needed here - let the caller validate based on message type
    
    return true;
}

} // namespace phoenix::transport

#endif // PHX_WITH_TRANSPORT_DEPS

