#pragma once

#include <QString>

class TransportClient {
public:
    virtual ~TransportClient() = default;

    // Connection management
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    // Backend identification
    virtual QString backendName() const = 0;

    // Placeholders for future request/response API
    // virtual Response sendRequest(const Request& req) = 0;
};

