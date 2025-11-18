#pragma once

#include "TransportClient.hpp"
#include <QString>

class LocalSocketChannel : public TransportClient {
public:
    explicit LocalSocketChannel(const QString& socketPath = QString());
    ~LocalSocketChannel() override = default;

    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    QString backendName() const override;

private:
    QString socketPath_;
    bool connected_;
};

