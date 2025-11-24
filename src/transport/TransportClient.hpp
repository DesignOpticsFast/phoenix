#pragma once

#include <optional>
#include <string>

// Forward declaration - actual proto types included in implementation
namespace palantir {
    class CapabilitiesResponse;
}

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
    // If outError is provided, error message is written to it
    virtual std::optional<palantir::CapabilitiesResponse>
        getCapabilities(std::string* outError = nullptr) = 0;
};

