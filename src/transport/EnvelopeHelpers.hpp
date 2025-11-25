#pragma once

#ifdef PHX_WITH_TRANSPORT_DEPS
#include "palantir/envelope.pb.h"
#include "palantir/error.pb.h"
#include <google/protobuf/message.h>
#include <QByteArray>
#include <QString>
#include <map>
#include <string>
#include <optional>

namespace phoenix::transport {

// Constants
static constexpr uint32_t PROTOCOL_VERSION = 1;

/**
 * Create a MessageEnvelope from an inner message.
 * 
 * @param type Message type enum
 * @param innerMessage Inner protobuf message to wrap
 * @param metadata Optional metadata map (can be empty)
 * @param outError Optional error string output
 * @return MessageEnvelope on success, empty optional on failure
 */
std::optional<palantir::MessageEnvelope> makeEnvelope(
    palantir::MessageType type,
    const google::protobuf::Message& innerMessage,
    const std::map<std::string, std::string>& metadata = {},
    QString* outError = nullptr);

/**
 * Parse a MessageEnvelope from a buffer.
 * 
 * @param buffer Serialized MessageEnvelope bytes
 * @param outEnvelope Output envelope (populated on success)
 * @param outError Optional error string output
 * @return true on success, false on failure
 */
bool parseEnvelope(
    const QByteArray& buffer,
    palantir::MessageEnvelope& outEnvelope,
    QString* outError = nullptr);

} // namespace phoenix::transport

#endif // PHX_WITH_TRANSPORT_DEPS

