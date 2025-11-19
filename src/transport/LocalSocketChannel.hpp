#pragma once

#include "TransportClient.hpp"
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMap>
#include <memory>
#include <vector>

class QLocalSocket;
class QByteArray;

// Result structure for XY Sine computation
struct XYSineResult {
    std::vector<double> x;
    std::vector<double> y;
};

Q_DECLARE_METATYPE(XYSineResult)

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
    
    // Compute XY Sine feature
    // Returns true on success, false on failure. Logs errors internally.
    bool computeXYSine(const QMap<QString, QVariant>& params, XYSineResult& outResult);

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

