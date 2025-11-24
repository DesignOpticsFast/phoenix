#pragma once

#include <optional>
#include <QString>

// Forward declaration - actual proto types included in implementation
#ifdef PHX_WITH_TRANSPORT_DEPS
namespace palantir {
    class CapabilitiesResponse;
}
#endif

// Transport client interface for Phoenix ↔ Bedrock communication
// WP1: Minimal interface for Capabilities query
// Future: Will expand with compute requests, progress, cancellation
class TransportClient {
public:
    virtual ~TransportClient() = default;

    // Connection management
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    // Capabilities query (WP1 minimal implementation)
    // Returns CapabilitiesResponse on success, std::nullopt on failure
    // If outError is provided, error message is written to it (QString for UI consistency)
#ifdef PHX_WITH_TRANSPORT_DEPS
    virtual std::optional<palantir::CapabilitiesResponse>
        getCapabilities(QString* outError = nullptr) = 0;
#else
    // When transport deps are OFF, return nullopt
    virtual std::optional<int> getCapabilities(QString* outError = nullptr) {
        if (outError) {
            *outError = "Transport dependencies not enabled";
        }
        return std::nullopt;
    }
#endif
};

