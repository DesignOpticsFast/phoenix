#pragma once

#include "TransportClient.hpp"
#include <QString>

class GrpcUdsChannel : public TransportClient {
public:
    explicit GrpcUdsChannel(const QString& socketPath = QString());
    ~GrpcUdsChannel() override = default;

    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    QString backendName() const override;

private:
    QString socketPath_;
    bool connected_;
};

