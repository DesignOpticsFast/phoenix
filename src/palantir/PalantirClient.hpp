#pragma once

#include <QLocalSocket>
#include <QTimer>
#include <QObject>
#include <QByteArray>
#include <QString>
#include <memory>

class PalantirClient : public QObject
{
    Q_OBJECT

public:
    enum class ConnState : quint8 { Idle, Connecting, Connected, ErrorBackoff, PermanentFail };
    Q_ENUM(ConnState)

    explicit PalantirClient(QObject *parent = nullptr);
    ~PalantirClient() override;

    // Connection management
    [[nodiscard]] bool connectAsync();     // returns false if already Connecting/Connected/PermanentFail
    void disconnectAsync();
    [[nodiscard]] ConnState connectionState() const noexcept;

    // Protocol
    void sendRequest(quint16 type, const QByteArray& payload);  // no-op if not Connected

    // Signals for async operations
signals:
    void connected();
    void disconnected();
    void connectionStateChanged(PalantirClient::ConnState);
    void connectionError(QString message);
    void permanentDisconnect(QString message);
    void messageReceived(quint16 type, QByteArray payload);

private slots:
    void onSocketConnected();
    void onSocketError(QLocalSocket::LocalSocketError);
    void onSocketStateChanged(QLocalSocket::LocalSocketState);
    void onSocketReadyRead();
    void onBackoffTimeout();

private:
    void setState(ConnState s);
    void startBackoff(const QString& why);
    void resetBackoff();

    std::unique_ptr<QLocalSocket> socket_;
    QTimer backoffTimer_{};
    ConnState state_{ConnState::Idle};
    int backoffAttempt_{0};
    QString socketName_;

    static constexpr int kMaxBackoffAttempts = 5;

    // Protocol buffer (for Chunk 4)
    QByteArray receiveBuffer_;
};
