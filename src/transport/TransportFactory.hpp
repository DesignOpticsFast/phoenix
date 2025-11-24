#pragma once

#include "TransportClient.hpp"
#include <memory>

// Transport backend selection
enum class TransportBackend {
    LocalSocket,  // QLocalSocket-based IPC (future)
    Auto          // Auto-select (currently same as LocalSocket)
};

// Factory for creating transport clients
// WP1: Simple factory, no environment variable detection yet
class TransportFactory {
public:
    // Create a transport client for the specified backend
    static std::unique_ptr<TransportClient> makeTransportClient(TransportBackend backend);
};

