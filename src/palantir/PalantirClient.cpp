#include "PalantirClient.hpp"
#include "PalantirLogging.h"

#include <QLocalSocket>
#include <QTimer>
#include <QCoreApplication>
#include <QDataStream>

PalantirClient::PalantirClient(QObject *parent)
    : QObject(parent)
{
    socket_.reset(new QLocalSocket(this));

    // Connect socket signals
    connect(socket_.get(), &QLocalSocket::connected, this, &PalantirClient::onSocketConnected);
    connect(socket_.get(), &QLocalSocket::errorOccurred, this, &PalantirClient::onSocketError);
    connect(socket_.get(), &QLocalSocket::stateChanged, this, &PalantirClient::onSocketStateChanged);
    connect(socket_.get(), &QLocalSocket::readyRead, this, &PalantirClient::onSocketReadyRead);

    // Setup backoff timer
    backoffTimer_.setSingleShot(true);
    connect(&backoffTimer_, &QTimer::timeout, this, &PalantirClient::onBackoffTimeout);

    // Resolve socket name from environment or use default
    socketName_ = qEnvironmentVariable("PALANTIR_SOCKET", "palantir_bedrock");
}

PalantirClient::~PalantirClient()
{
    disconnectAsync();
}

bool PalantirClient::connectAsync()
{
    if (state_ == ConnState::Connected ||
        state_ == ConnState::Connecting ||
        state_ == ConnState::PermanentFail) {
        return false;
    }

    socket_->connectToServer(socketName_);
    setState(ConnState::Connecting);
    qCDebug(phxPalantirConn) << "Initiated connection to" << socketName_;
    return true;
}

void PalantirClient::disconnectAsync()
{
    socket_->abort();
    resetBackoff();
    setState(ConnState::Idle);
    emit disconnected();
    qCDebug(phxPalantirConn) << "Disconnected from server";
}

ConnState PalantirClient::connectionState() const noexcept
{
    return state_;
}

void PalantirClient::sendRequest(quint16 type, const QByteArray& payload)
{
    if (state_ != ConnState::Connected) {
        return;  // Silent no-op if not connected
    }

    // TODO: Implement send framing (header + payload) for Chunk 5+ feature work
    qCDebug(phxPalantirProto) << "sendRequest type:" << type << "payload:" << payload.size() << "bytes";
}

void PalantirClient::onSocketConnected()
{
    setState(ConnState::Connected);
    resetBackoff();
    emit connected();
    qCDebug(phxPalantirConn) << "Socket connected";
}

void PalantirClient::onSocketError(QLocalSocket::LocalSocketError error)
{
    QString errorMsg = socket_->errorString();
    if (errorMsg.isEmpty()) {
        errorMsg = "Socket error: " + QString::number(static_cast<int>(error));
    }

    if (state_ == ConnState::Connecting || state_ == ConnState::Connected) {
        startBackoff(errorMsg);
    } else if (state_ == ConnState::ErrorBackoff) {
        // Log but don't restart (timer already running)
        qCDebug(phxPalantirConn) << "Socket error during backoff:" << errorMsg;
    }
}

void PalantirClient::onSocketStateChanged(QLocalSocket::LocalSocketState state)
{
    if (state == QLocalSocket::UnconnectedState) {
        if (state_ == ConnState::Connecting || state_ == ConnState::Connected) {
            startBackoff("Socket disconnected");
        }
    }
}

void PalantirClient::onSocketReadyRead()
{
    // Accumulate incoming data
    receiveBuffer_.append(socket_->readAll());

    // Parse complete frames
    while (true) {
        // Check if we have enough data for header
        if (receiveBuffer_.size() < kHeaderSize) {
            break;  // Need more data
        }

        // Read header from first kHeaderSize bytes
        QByteArray header = receiveBuffer_.left(kHeaderSize);
        QDataStream s(&header, QIODevice::ReadOnly);
        s.setByteOrder(QDataStream::BigEndian);

        quint32 magic;
        quint16 version;
        quint16 type;
        quint32 length;

        s >> magic >> version >> type >> length;

        // Validate magic
        if (magic != kMagic) {
            handleProtocolError(QString("bad magic 0x%1 (expected 0x%2)")
                               .arg(magic, 8, 16, QChar('0'))
                               .arg(kMagic, 8, 16, QChar('0')));
            return;
        }

        // Validate version
        if (version < kVersionMin || version > kVersionMax) {
            qCWarning(phxPalantirProto) << "Version mismatch: got v" << version
                                        << "supported v" << kVersionMin << ".." << kVersionMax;
            handleProtocolError(QString("version mismatch: v%1 not in %2..%3")
                               .arg(version).arg(kVersionMin).arg(kVersionMax));
            return;
        }

        // Validate length
        if (length > kMaxMessageSize) {
            qCWarning(phxPalantirProto) << "Payload too large:" << length << "bytes (max" << kMaxMessageSize << ")";
            handleProtocolError(QString("payload too large: %1 (max %2)").arg(length).arg(kMaxMessageSize));
            return;
        }

        // Check if we have complete frame (header + payload)
        const int frameSize = kHeaderSize + static_cast<int>(length);
        if (receiveBuffer_.size() < frameSize) {
            break;  // Need more data for payload
        }

        // Extract payload
        QByteArray payload = receiveBuffer_.mid(kHeaderSize, static_cast<int>(length));

        // Remove consumed frame from buffer
        receiveBuffer_.remove(0, frameSize);

        // Emit message
        qCDebug(phxPalantirProto) << "Frame type=" << type << "len=" << length;
        emit messageReceived(type, payload);

        // Try dispatcher handler
        auto it = handlers_.find(type);
        if (it != handlers_.end()) {
            it.value()(payload);
        } else {
            qCDebug(phxPalantirProto) << "No handler for type" << type
                                     << "(payload" << payload.size() << "bytes)";
        }

        // Continue loop to handle multiple frames
    }
}

void PalantirClient::onBackoffTimeout()
{
    qCDebug(phxPalantirConn) << "Backoff timeout, retrying connection";
    connectAsync();
}

void PalantirClient::setState(ConnState s)
{
    if (state_ != s) {
        state_ = s;
        emit connectionStateChanged(state_);
        qCDebug(phxPalantirConn) << "State changed to:" << static_cast<int>(state_);
    }
}

void PalantirClient::startBackoff(const QString& why)
{
    if (backoffAttempt_ >= kMaxBackoffAttempts) {
        setState(ConnState::PermanentFail);
        emit permanentDisconnect(why);
        qCDebug(phxPalantirConn) << "Permanent disconnect after" << kMaxBackoffAttempts << "attempts:" << why;
        return;
    }

    setState(ConnState::ErrorBackoff);
    const int delayMs = qMin(16000, 1000 << backoffAttempt_);  // 1s, 2s, 4s, 8s, 16s
    backoffTimer_.start(delayMs);
    emit connectionError(why);
    ++backoffAttempt_;
    qCDebug(phxPalantirConn) << "Starting backoff attempt" << backoffAttempt_
                             << "delay" << delayMs << "ms:" << why;
}

void PalantirClient::resetBackoff()
{
    backoffAttempt_ = 0;
    backoffTimer_.stop();
}

void PalantirClient::registerHandler(quint16 type, std::function<void(const QByteArray&)> handler)
{
    handlers_.insert(type, std::move(handler));
}

void PalantirClient::unregisterHandler(quint16 type)
{
    handlers_.remove(type);
}

void PalantirClient::handleProtocolError(const QString& reason)
{
    qCWarning(phxPalantirProto) << "Protocol error, aborting socket:" << reason;
    socket_->abort();
    receiveBuffer_.clear();
    startBackoff("Protocol error: " + reason);
}
