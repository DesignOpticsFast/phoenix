#pragma once

#include "TransportClient.hpp"
#include <memory>
#include <QString>

enum class TransportBackend {
    Grpc,
    LocalSocket,
    Auto
};

// Factory functions for creating transport clients

// Create a transport client with explicit backend selection
std::unique_ptr<TransportClient> makeTransportClient(TransportBackend backend);

// Create a transport client based on PHOENIX_TRANSPORT environment variable
// Values: "grpc", "localsocket", "auto" (default)
// If unset or "auto", returns gRPC transport (fallback logic handled at call site)
std::unique_ptr<TransportClient> makeTransportClientFromEnv();

// Get list of available backend names
QStringList availableTransportBackends();

