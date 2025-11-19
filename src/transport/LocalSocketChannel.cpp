#include "LocalSocketChannel.hpp"

#include <QLocalSocket>
#include <QProcessEnvironment>
#include <QDebug>
#include <QTimer>
#include <QEventLoop>
#include "palantir.pb.h"

#include <memory>
#include <cstring>

using namespace palantir;

LocalSocketChannel::LocalSocketChannel(const QString& socketName)
    : socketName_(socketName.isEmpty() ? QStringLiteral("palantir_bedrock") : socketName)
    , socket_(nullptr)
    , connected_(false)
{
}

LocalSocketChannel::~LocalSocketChannel() {
    disconnect();
}

bool LocalSocketChannel::connect() {
    if (connected_) {
        return true;
    }
    
    // Read socket name from environment if not provided
    if (socketName_.isEmpty()) {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        socketName_ = env.value("PHOENIX_LOCALSOCKET_NAME", "palantir_bedrock");
    }
    
    try {
        socket_ = std::make_unique<QLocalSocket>();
        
        // Connect signals
        QObject::connect(socket_.get(), &QLocalSocket::connected, [this]() {
            connected_ = true;
            qDebug() << "LocalSocketChannel: Connected to" << socketName_;
        });
        
        QObject::connect(socket_.get(), &QLocalSocket::disconnected, [this]() {
            connected_ = false;
            qDebug() << "LocalSocketChannel: Disconnected";
        });
        
        QObject::connect(socket_.get(), &QLocalSocket::readyRead, [this]() {
            parseIncomingData();
        });
        
        QObject::connect(socket_.get(), QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::errorOccurred),
                        [this](QLocalSocket::LocalSocketError error) {
            qWarning() << "LocalSocketChannel: Socket error:" << error << socket_->errorString();
        });
        
        // Attempt connection
        socket_->connectToServer(socketName_);
        
        // Wait for connection (with timeout)
        if (!socket_->waitForConnected(3000)) {
            qWarning() << "LocalSocketChannel: Connection timeout to" << socketName_;
            socket_.reset();
            connected_ = false;
            return false;
        }
        
        connected_ = (socket_->state() == QLocalSocket::ConnectedState);
        return connected_;
    } catch (const std::exception& e) {
        qWarning() << "LocalSocketChannel: Exception during connect:" << e.what();
        socket_.reset();
        connected_ = false;
        return false;
    }
}

void LocalSocketChannel::disconnect() {
    if (!connected_ && !socket_) {
        return;
    }
    
    if (socket_) {
        socket_->disconnectFromServer();
        if (socket_->state() != QLocalSocket::UnconnectedState) {
            socket_->waitForDisconnected(1000);
        }
        socket_.reset();
    }
    
    readBuffer_.clear();
    connected_ = false;
    qDebug() << "LocalSocketChannel: Disconnected";
}

bool LocalSocketChannel::isConnected() const {
    return connected_ && socket_ && socket_->state() == QLocalSocket::ConnectedState;
}

QString LocalSocketChannel::backendName() const {
    return QStringLiteral("LocalSocket");
}

bool LocalSocketChannel::sendMessage(const QByteArray& message) {
    if (!isConnected()) {
        qWarning() << "LocalSocketChannel::sendMessage: Not connected";
        return false;
    }
    
    // Create length-prefixed message (4-byte little-endian length + payload)
    QByteArray data;
    uint32_t length = static_cast<uint32_t>(message.size());
    
    // Write length (little-endian)
    data.append(reinterpret_cast<const char*>(&length), 4);
    data.append(message);
    
    // Send data
    qint64 written = socket_->write(data);
    if (written != data.size()) {
        qWarning() << "LocalSocketChannel::sendMessage: Failed to send complete message";
        return false;
    }
    
    if (!socket_->waitForBytesWritten(3000)) {
        qWarning() << "LocalSocketChannel::sendMessage: Write timeout";
        return false;
    }
    
    return true;
}

QByteArray LocalSocketChannel::readMessage() {
    if (readBuffer_.size() < 4) {
        return QByteArray();
    }
    
    // Read length (little-endian)
    uint32_t length;
    memcpy(&length, readBuffer_.data(), 4);
    
    if (readBuffer_.size() < 4 + length) {
        // Not enough data yet
        return QByteArray();
    }
    
    // Extract message
    QByteArray message = readBuffer_.mid(4, length);
    readBuffer_.remove(0, 4 + length);
    
    return message;
}

void LocalSocketChannel::parseIncomingData() {
    if (!socket_) {
        return;
    }
    
    // Append new data to buffer
    readBuffer_.append(socket_->readAll());
    
    // Process complete messages
    while (readBuffer_.size() >= 4) {
        QByteArray message = readMessage();
        if (message.isEmpty()) {
            break;  // Need more data
        }
        // For now, we handle messages synchronously in requestCapabilities()
        // In a full implementation, we'd emit signals or use callbacks
    }
}

bool LocalSocketChannel::requestCapabilities(QStringList& features) {
    if (!isConnected()) {
        qWarning() << "LocalSocketChannel::requestCapabilities: Not connected";
        return false;
    }
    
    try {
        // Create CapabilitiesRequest message
        CapabilitiesRequest request;
        
        // Serialize message
        std::string serialized;
        if (!request.SerializeToString(&serialized)) {
            qWarning() << "LocalSocketChannel::requestCapabilities: Failed to serialize request";
            return false;
        }
        
        QByteArray requestData(serialized.data(), static_cast<int>(serialized.size()));
        
        // Clear read buffer before sending
        readBuffer_.clear();
        
        // Send message
        if (!sendMessage(requestData)) {
            return false;
        }
        
        // Wait for response (with timeout)
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        timeoutTimer.setInterval(5000);  // 5 second timeout
        
        QEventLoop loop;
        QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
        QObject::connect(socket_.get(), &QLocalSocket::readyRead, &loop, &QEventLoop::quit);
        
        timeoutTimer.start();
        loop.exec();
        timeoutTimer.stop();
        
        // Check if we got a complete message
        parseIncomingData();
        
        if (readBuffer_.size() < 4) {
            qWarning() << "LocalSocketChannel::requestCapabilities: No response received";
            return false;
        }
        
        // Read response message
        QByteArray responseData = readMessage();
        if (responseData.isEmpty()) {
            qWarning() << "LocalSocketChannel::requestCapabilities: Failed to read response";
            return false;
        }
        
        // Parse Capabilities response
        Capabilities caps;
        if (!caps.ParseFromArray(responseData.data(), responseData.size())) {
            qWarning() << "LocalSocketChannel::requestCapabilities: Failed to parse response";
            return false;
        }
        
        // Extract features
        features.clear();
        for (int i = 0; i < caps.supported_features_size(); ++i) {
            features.append(QString::fromStdString(caps.supported_features(i)));
        }
        
        qDebug() << "LocalSocketChannel::requestCapabilities: Received" << features.size() << "features";
        return true;
    } catch (const std::exception& e) {
        qWarning() << "LocalSocketChannel::requestCapabilities: Exception:" << e.what();
        return false;
    }
}

