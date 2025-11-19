#pragma once

#include "TransportClient.hpp"
#include <QString>
#include <QStringList>
#include <memory>

class QLocalSocket;
class QByteArray;

class LocalSocketChannel : public TransportClient {
public:
    explicit LocalSocketChannel(const QString& socketName = QString());
    ~LocalSocketChannel() override;

    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    QString backendName() const override;
    
    // Request server capabilities (round-trip test for WP2.B)
    // Returns true on success, false on failure. Logs errors internally.
    bool requestCapabilities(QStringList& features);

private:
    QString socketName_;
    std::unique_ptr<QLocalSocket> socket_;
    QByteArray readBuffer_;
    bool connected_;
    
    // Helper methods for message framing
    bool sendMessage(const QByteArray& message);
    QByteArray readMessage();
    void parseIncomingData();
};

