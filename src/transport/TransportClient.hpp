#pragma once

#include <QString>

// Transport error codes
enum class TransportError {
    NoError = 0,
    ConnectionFailed,      // Cannot connect to server socket
    ConnectionTimeout,     // Connection attempt timed out
    ProtocolError,         // Protocol/message parsing error
    NetworkError,         // Socket/network I/O error
    
    // Server-side errors
    InvalidArgument,      // INVALID_ARGUMENT - bad parameters
    ResourceExhausted,   // RESOURCE_EXHAUSTED - server at capacity
    Unimplemented,       // UNIMPLEMENTED - feature not supported
    PermissionDenied,    // PERMISSION_DENIED - license issue
    Cancelled,           // CANCELLED - job was cancelled
    ServerError          // Generic server error (fallback)
};

// Helper function to get user-friendly error message
inline QString transportErrorString(TransportError error) {
    switch (error) {
        case TransportError::NoError:
            return QString();
        case TransportError::ConnectionFailed:
            return QStringLiteral("Failed to connect to Bedrock server. Please ensure Bedrock is running.");
        case TransportError::ConnectionTimeout:
            return QStringLiteral("Connection to Bedrock server timed out.");
        case TransportError::ProtocolError:
            return QStringLiteral("Protocol error communicating with Bedrock server.");
        case TransportError::NetworkError:
            return QStringLiteral("Network I/O error occurred.");
        case TransportError::InvalidArgument:
            return QStringLiteral("Invalid parameters provided.");
        case TransportError::ResourceExhausted:
            return QStringLiteral("Server at capacity.");
        case TransportError::Unimplemented:
            return QStringLiteral("Feature not supported.");
        case TransportError::PermissionDenied:
            return QStringLiteral("License required.");
        case TransportError::Cancelled:
            return QStringLiteral("Operation cancelled.");
        case TransportError::ServerError:
            return QStringLiteral("Bedrock server returned an error.");
    }
    return QStringLiteral("Unknown transport error.");
}

class TransportClient {
public:
    virtual ~TransportClient() = default;

    // Connection management
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    // Backend identification
    virtual QString backendName() const = 0;
    
    // Get last error (if any)
    virtual TransportError lastError() const = 0;
    virtual QString lastErrorString() const = 0;

    // Placeholders for future request/response API
    // virtual Response sendRequest(const Request& req) = 0;
};

