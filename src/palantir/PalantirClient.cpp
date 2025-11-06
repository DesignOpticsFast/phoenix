#include "PalantirClient.hpp"
#include "PalantirLogging.h"

#include <QLocalSocket>
#include <QTimer>
#include <QCoreApplication>

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

    // TODO: Implement framing in Chunk 4
    // For now, just log
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
    // Stub for now (Chunk 4 will implement full protocol parsing)
    qCDebug(phxPalantirProto) << "readyRead:" << socket_->bytesAvailable() << "bytes";
    // Accumulate receiveBuffer_ for future parsing
    receiveBuffer_ += socket_->readAll();
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
