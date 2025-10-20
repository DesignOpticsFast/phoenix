#pragma once

#include <QLocalSocket>
#include <QTimer>
#include <QObject>
#include <QByteArray>
#include <QString>
#include <memory>
#include <functional>
#include <map>
#include <atomic>

#include "palantir.pb.h"

class PalantirClient : public QObject
{
    Q_OBJECT

public:
    explicit PalantirClient(QObject *parent = nullptr);
    ~PalantirClient() override;

    // Connection management
    bool connectToServer();
    void disconnectFromServer();
    bool isConnected() const;

    // Job management
    QString startJob(const palantir::ComputeSpec& spec);
    void cancelJob(const QString& jobId);
    void requestCapabilities();

    // Signals for async operations
signals:
    void connected();
    void disconnected();
    void jobStarted(const QString& jobId);
    void jobProgress(const QString& jobId, double progressPct, const QString& status);
    void jobCompleted(const QString& jobId, const palantir::ResultMeta& meta);
    void jobFailed(const QString& jobId, const QString& error);
    void jobCancelled(const QString& jobId);
    void capabilitiesReceived(const palantir::Capabilities& caps);
    void errorOccurred(const QString& error);

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QLocalSocket::LocalSocketError error);
    void onSocketReadyRead();
    void onReconnectTimer();

private:
    // Message handling
    void handleStartReply(const palantir::StartReply& reply);
    void handleProgress(const palantir::Progress& progress);
    void handleResultMeta(const palantir::ResultMeta& meta);
    void handleDataChunk(const palantir::DataChunk& chunk);
    void handleCapabilities(const palantir::Capabilities& caps);
    void handlePong(const palantir::Pong& pong);

    // Protocol helpers
    void sendMessage(const google::protobuf::Message& message);
    void parseIncomingData();
    QByteArray readMessage();
    
    // Connection management
    void startReconnectTimer();
    void stopReconnectTimer();
    void attemptReconnect();

    // Socket and protocol
    std::unique_ptr<QLocalSocket> socket_;
    QTimer reconnectTimer_;
    QByteArray receiveBuffer_;
    
    // Connection state
    std::atomic<bool> connected_;
    int reconnectAttempts_;
    static const int MAX_RECONNECT_ATTEMPTS = 5;
    static const int RECONNECT_INTERVAL_MS = 1000;
    
    // Job tracking
    std::map<QString, palantir::ComputeSpec> activeJobs_;
    std::map<QString, QByteArray> jobDataBuffers_;
    
    // Server capabilities
    palantir::Capabilities serverCapabilities_;
    bool capabilitiesReceived_;
};




