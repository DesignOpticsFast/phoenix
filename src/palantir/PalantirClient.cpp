// NOTE (Sprint 4): This implementation is valid only when PHX_WITH_PALANTIR=1.
// Existing blocking calls are tolerated for now; they will be guarded or moved
// behind the feature flag and replaced with stubs in the next chunk.

#include "PalantirClient.hpp"

#include <QLocalSocket>
#include <QTimer>
#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QDateTime>
#include <QRandomGenerator>

PalantirClient::PalantirClient(QObject *parent)
    : QObject(parent)
    , socket_(std::make_unique<QLocalSocket>(this))
    , connected_(false)
    , reconnectAttempts_(0)
    , capabilitiesReceived_(false)
{
    // Connect socket signals
    connect(socket_.get(), &QLocalSocket::connected, this, &PalantirClient::onSocketConnected);
    connect(socket_.get(), &QLocalSocket::disconnected, this, &PalantirClient::onSocketDisconnected);
    connect(socket_.get(), &QLocalSocket::errorOccurred,
            this, &PalantirClient::onSocketError);
    connect(socket_.get(), &QLocalSocket::readyRead, this, &PalantirClient::onSocketReadyRead);
    
    // Setup reconnect timer
    reconnectTimer_.setSingleShot(true);
    connect(&reconnectTimer_, &QTimer::timeout, this, &PalantirClient::onReconnectTimer);
}

PalantirClient::~PalantirClient()
{
    disconnectFromServer();
}

bool PalantirClient::connectToServer()
{
    if (connected_) {
        return true;
    }
    
    // Start Bedrock server if not running
    QString socketName = "palantir_bedrock_" + QString::number(QCoreApplication::applicationPid());
    
    // Try to connect to existing server
    socket_->connectToServer(socketName);
    
    if (socket_->waitForConnected(1000)) {
        connected_ = true;
        emit connected();
        requestCapabilities();
        return true;
    }
    
    // If connection failed, start Bedrock server
    qDebug() << "Starting Bedrock server...";
    QProcess* bedrockProcess = new QProcess(this);
    bedrockProcess->start("bedrock_server", QStringList() << "--socket" << socketName);
    
    if (!bedrockProcess->waitForStarted(5000)) {
        emit errorOccurred("Failed to start Bedrock server");
        return false;
    }
    
    // Wait for server to be ready
    QThread::msleep(1000);
    
    // Try to connect again
    socket_->connectToServer(socketName);
    if (socket_->waitForConnected(5000)) {
        connected_ = true;
        emit connected();
        requestCapabilities();
        return true;
    }
    
    emit errorOccurred("Failed to connect to Bedrock server");
    return false;
}

void PalantirClient::disconnectFromServer()
{
    if (connected_) {
        socket_->disconnectFromServer();
        connected_ = false;
        emit disconnected();
    }
    stopReconnectTimer();
}

bool PalantirClient::isConnected() const
{
    return connected_;
}

QString PalantirClient::startJob(const palantir::ComputeSpec& spec)
{
    if (!connected_) {
        emit errorOccurred("Not connected to server");
        return QString();
    }
    
    // Generate job ID
    QString jobId = QString("job_%1_%2").arg(QDateTime::currentMSecsSinceEpoch()).arg(QRandomGenerator::global()->generate());
    
    // Create StartJob message
    palantir::StartJob startJob;
    startJob.mutable_job_id()->set_id(jobId.toStdString());
    startJob.mutable_spec()->CopyFrom(spec);
    
    // Send message
    sendMessage(startJob);
    
    // Track job
    activeJobs_[jobId] = spec;
    
    return jobId;
}

void PalantirClient::cancelJob(const QString& jobId)
{
    if (!connected_) {
        return;
    }
    
    palantir::Cancel cancel;
    cancel.mutable_job_id()->set_id(jobId.toStdString());
    
    sendMessage(cancel);
}

void PalantirClient::requestCapabilities()
{
    if (!connected_) {
        return;
    }
    
    palantir::CapabilitiesRequest request;
    sendMessage(request);
}

void PalantirClient::onSocketConnected()
{
    connected_ = true;
    reconnectAttempts_ = 0;
    stopReconnectTimer();
    emit connected();
    requestCapabilities();
}

void PalantirClient::onSocketDisconnected()
{
    connected_ = false;
    emit disconnected();
    startReconnectTimer();
}

void PalantirClient::onSocketError(QLocalSocket::LocalSocketError error)
{
    QString errorMsg;
    switch (error) {
        case QLocalSocket::ServerNotFoundError:
            errorMsg = "Server not found";
            break;
        case QLocalSocket::ConnectionRefusedError:
            errorMsg = "Connection refused";
            break;
        case QLocalSocket::PeerClosedError:
            errorMsg = "Server closed connection";
            break;
        default:
            errorMsg = "Socket error: " + QString::number(error);
            break;
    }
    
    emit errorOccurred(errorMsg);
    startReconnectTimer();
}

void PalantirClient::onSocketReadyRead()
{
    parseIncomingData();
}

void PalantirClient::onReconnectTimer()
{
    attemptReconnect();
}

void PalantirClient::handleStartReply(const palantir::StartReply& reply)
{
    QString jobId = QString::fromStdString(reply.job_id().id());
    QString status = QString::fromStdString(reply.status());
    
    if (status == "OK") {
        emit jobStarted(jobId);
    } else {
        QString error = QString::fromStdString(reply.error_message());
        emit jobFailed(jobId, error);
        activeJobs_.erase(jobId);
    }
}

void PalantirClient::handleProgress(const palantir::Progress& progress)
{
    QString jobId = QString::fromStdString(progress.job_id().id());
    QString status = QString::fromStdString(progress.status());
    
    emit jobProgress(jobId, progress.progress_pct(), status);
}

void PalantirClient::handleResultMeta(const palantir::ResultMeta& meta)
{
    QString jobId = QString::fromStdString(meta.job_id().id());
    QString status = QString::fromStdString(meta.status());
    
    if (status == "SUCCEEDED") {
        emit jobCompleted(jobId, meta);
    } else if (status == "FAILED") {
        QString error = QString::fromStdString(meta.error_message());
        emit jobFailed(jobId, error);
    } else if (status == "CANCELLED") {
        emit jobCancelled(jobId);
    }
    
    activeJobs_.erase(jobId);
}

void PalantirClient::handleDataChunk(const palantir::DataChunk& chunk)
{
    QString jobId = QString::fromStdString(chunk.job_id().id());
    
    // Store data chunk
    jobDataBuffers_[jobId] += QByteArray(chunk.data().data(), chunk.data().size());
    
    // If this is the last chunk, emit completion
    if (chunk.chunk_index() == chunk.total_chunks() - 1) {
        // TODO: Process complete data and emit result
        jobDataBuffers_.erase(jobId);
    }
}

void PalantirClient::handleCapabilities(const palantir::Capabilities& caps)
{
    serverCapabilities_ = caps;
    capabilitiesReceived_ = true;
    emit capabilitiesReceived(caps);
}

void PalantirClient::handlePong(const palantir::Pong& pong)
{
    // Handle ping response
    Q_UNUSED(pong);
}

void PalantirClient::sendMessage(const google::protobuf::Message& message)
{
    if (!connected_) {
        return;
    }
    
    // Serialize message
    std::string serialized;
    if (!message.SerializeToString(&serialized)) {
        emit errorOccurred("Failed to serialize message");
        return;
    }
    
    // Create length-prefixed message
    QByteArray data;
    uint32_t length = static_cast<uint32_t>(serialized.size());
    
    // Write length (little-endian)
    data.append(reinterpret_cast<const char*>(&length), 4);
    data.append(serialized.data(), serialized.size());
    
    // Send data
    qint64 written = socket_->write(data);
    if (written != data.size()) {
        emit errorOccurred("Failed to send complete message");
    }
}

void PalantirClient::parseIncomingData()
{
    receiveBuffer_ += socket_->readAll();
    
    while (receiveBuffer_.size() >= 4) {
        // Read message length
        uint32_t length;
        memcpy(&length, receiveBuffer_.data(), 4);
        
        if (receiveBuffer_.size() < 4 + length) {
            // Need more data
            return;
        }
        
        // Extract message data
        QByteArray messageData = receiveBuffer_.mid(4, length);
        receiveBuffer_.remove(0, 4 + length);
        
        // Parse message type and dispatch
        // TODO: Implement message type parsing and dispatch
        // For now, just log the received data
        qDebug() << "Received message of length:" << length;
    }
}

QByteArray PalantirClient::readMessage()
{
    if (receiveBuffer_.size() < 4) {
        return QByteArray();
    }
    
    uint32_t length;
    memcpy(&length, receiveBuffer_.data(), 4);
    
    if (receiveBuffer_.size() < 4 + length) {
        return QByteArray();
    }
    
    QByteArray message = receiveBuffer_.mid(4, length);
    receiveBuffer_.remove(0, 4 + length);
    
    return message;
}

void PalantirClient::startReconnectTimer()
{
    if (reconnectAttempts_ < MAX_RECONNECT_ATTEMPTS) {
        reconnectTimer_.start(RECONNECT_INTERVAL_MS);
    }
}

void PalantirClient::stopReconnectTimer()
{
    reconnectTimer_.stop();
}

void PalantirClient::attemptReconnect()
{
    if (reconnectAttempts_ >= MAX_RECONNECT_ATTEMPTS) {
        emit errorOccurred("Maximum reconnection attempts reached");
        return;
    }
    
    reconnectAttempts_++;
    connectToServer();
}

#include "PalantirClient.moc"
