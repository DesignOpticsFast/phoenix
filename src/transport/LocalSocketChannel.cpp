#include "LocalSocketChannel.hpp"

#include <QLocalSocket>
#include <QProcessEnvironment>
#include <QLoggingCategory>
#include <QTimer>
#include <QEventLoop>
#include <QUuid>
#include "palantir.pb.h"

#include <memory>
#include <cstring>

using namespace palantir;

// Qt logging category for transport layer
Q_LOGGING_CATEGORY(phoenixTransport, "phoenix.transport")

LocalSocketChannel::LocalSocketChannel(const QString& socketName)
    : socketName_(socketName.isEmpty() ? QStringLiteral("palantir_bedrock") : socketName)
    , socket_(nullptr)
    , connected_(false)
    , m_lastError(TransportError::NoError)
    , readyReadConnection_()
{
}

LocalSocketChannel::~LocalSocketChannel() {
    disconnect();
}

bool LocalSocketChannel::connect() {
    if (connected_) {
        return true;
    }
    
    m_lastError = TransportError::NoError;
    
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
            qCDebug(phoenixTransport) << "Connected to" << socketName_;
            m_lastError = TransportError::NoError;
        });
        
        QObject::connect(socket_.get(), &QLocalSocket::disconnected, [this]() {
            connected_ = false;
            qCDebug(phoenixTransport) << "Disconnected";
        });
        
        // Store connection for later disconnection during compute
        readyReadConnection_ = QObject::connect(socket_.get(), &QLocalSocket::readyRead, [this]() {
            parseIncomingData();
        });
        
        QObject::connect(socket_.get(), QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::errorOccurred),
                        [this](QLocalSocket::LocalSocketError error) {
            setError(TransportError::NetworkError, QStringLiteral("Socket error: %1").arg(socket_->errorString()));
        });
        
        // Attempt connection
        socket_->connectToServer(socketName_);
        
        // Wait for connection (with timeout)
        if (!socket_->waitForConnected(3000)) {
            if (socket_->error() == QLocalSocket::ConnectionRefusedError) {
                setError(TransportError::ConnectionFailed, QStringLiteral("Connection refused to %1").arg(socketName_));
            } else {
                setError(TransportError::ConnectionTimeout, QStringLiteral("Connection timeout to %1").arg(socketName_));
            }
            socket_.reset();
            connected_ = false;
            return false;
        }
        
        connected_ = (socket_->state() == QLocalSocket::ConnectedState);
        if (connected_) {
            m_lastError = TransportError::NoError;
        }
        return connected_;
    } catch (const std::exception& e) {
        setError(TransportError::NetworkError, QStringLiteral("Exception during connect: %1").arg(e.what()));
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
    m_lastError = TransportError::NoError;
    qCDebug(phoenixTransport) << "Disconnected";
}

bool LocalSocketChannel::isConnected() const {
    return connected_ && socket_ && socket_->state() == QLocalSocket::ConnectedState;
}

QString LocalSocketChannel::backendName() const {
    return QStringLiteral("LocalSocket");
}

QString LocalSocketChannel::lastErrorString() const {
    QString baseMessage = transportErrorString(m_lastError);
    if (!m_lastErrorContext.isEmpty()) {
        // Return the detailed context message (from Bedrock) if available
        return m_lastErrorContext;
    }
    return baseMessage;
}

void LocalSocketChannel::setError(TransportError error, const QString& context) {
    m_lastError = error;
    m_lastErrorContext = context;  // Store detailed error message
    if (error != TransportError::NoError) {
        QString message = transportErrorString(error);
        if (!context.isEmpty()) {
            qCWarning(phoenixTransport) << context << "-" << message;
        } else {
            qCWarning(phoenixTransport) << message;
        }
    } else {
        m_lastErrorContext.clear();
    }
}

void LocalSocketChannel::pingHealthCheck() {
    if (!isConnected()) {
        qCDebug(phoenixTransport) << "Health check skipped: not connected";
        return;
    }
    
    try {
        // Create Ping message
        Ping ping;
        
        // Serialize
        std::string serialized;
        if (!ping.SerializeToString(&serialized)) {
            qCDebug(phoenixTransport) << "Health check: Failed to serialize Ping";
            return;
        }
        
        QByteArray pingData(serialized.data(), static_cast<int>(serialized.size()));
        
        // Send Ping (non-blocking, don't wait for response)
        if (sendMessage(pingData)) {
            qCDebug(phoenixTransport) << "Health check: Ping sent";
            // Note: Pong response will be handled asynchronously by parseIncomingData()
            // We don't wait for it - this is logging-only diagnostic
        } else {
            qCDebug(phoenixTransport) << "Health check: Failed to send Ping";
        }
    } catch (const std::exception& e) {
        qCDebug(phoenixTransport) << "Health check: Exception:" << e.what();
    }
}

bool LocalSocketChannel::sendMessage(const QByteArray& message) {
    if (!isConnected()) {
        setError(TransportError::ConnectionFailed, QStringLiteral("sendMessage: Not connected"));
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
        setError(TransportError::NetworkError, QStringLiteral("sendMessage: Failed to send complete message"));
        return false;
    }
    
    if (!socket_->waitForBytesWritten(3000)) {
        setError(TransportError::NetworkError, QStringLiteral("sendMessage: Write timeout"));
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
    
    // Process complete messages - only handle Pong, leave others in buffer for synchronous handlers
    while (readBuffer_.size() >= 4) {
        // Peek at message length without removing it
        uint32_t length;
        memcpy(&length, readBuffer_.data(), 4);
        
        if (readBuffer_.size() < 4 + length) {
            break;  // Need more data
        }
        
        // Extract message for parsing (but don't remove from buffer yet)
        QByteArray message = readBuffer_.mid(4, length);
        
        // Try to parse as Pong (for health check ping)
        Pong pong;
        if (pong.ParseFromArray(message.data(), message.size())) {
            qCDebug(phoenixTransport) << "Health check: Received Pong (timestamp:" << pong.timestamp_ms() << "ms)";
            // Remove Pong from buffer since we handled it
            readBuffer_.remove(0, 4 + length);
            continue;  // Handled, continue to next message
        }
        
        // Other messages (StartReply, ResultMeta, DataChunk, etc.) are handled synchronously
        // in requestCapabilities()/computeXYSine() - leave them in the buffer
        break;  // Stop processing, let synchronous handlers deal with remaining messages
    }
}

void LocalSocketChannel::reconnectReadyRead() {
    if (!readyReadConnection_) {
        readyReadConnection_ = QObject::connect(socket_.get(), &QLocalSocket::readyRead, [this]() {
            parseIncomingData();
        });
        qCDebug(phoenixTransport) << "Reconnected readyRead handler";
    }
}

bool LocalSocketChannel::requestCapabilities(QStringList& features) {
    if (!isConnected()) {
        setError(TransportError::ConnectionFailed, QStringLiteral("requestCapabilities: Not connected"));
        return false;
    }
    
    m_lastError = TransportError::NoError;
    
    try {
        // Create CapabilitiesRequest message
        CapabilitiesRequest request;
        
        // Serialize message
        std::string serialized;
        if (!request.SerializeToString(&serialized)) {
            setError(TransportError::ProtocolError, QStringLiteral("requestCapabilities: Failed to serialize request"));
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
            setError(TransportError::ConnectionTimeout, QStringLiteral("requestCapabilities: No response received"));
            return false;
        }
        
        // Read response message
        QByteArray responseData = readMessage();
        if (responseData.isEmpty()) {
            setError(TransportError::NetworkError, QStringLiteral("requestCapabilities: Failed to read response"));
            return false;
        }
        
        // Parse Capabilities response
        Capabilities caps;
        if (!caps.ParseFromArray(responseData.data(), responseData.size())) {
            setError(TransportError::ProtocolError, QStringLiteral("requestCapabilities: Failed to parse response"));
            return false;
        }
        
        // Extract features
        features.clear();
        for (int i = 0; i < caps.supported_features_size(); ++i) {
            features.append(QString::fromStdString(caps.supported_features(i)));
        }
        
        qCDebug(phoenixTransport) << "requestCapabilities: Received" << features.size() << "features";
        m_lastError = TransportError::NoError;
        return true;
    } catch (const std::exception& e) {
        setError(TransportError::NetworkError, QStringLiteral("requestCapabilities: Exception: %1").arg(e.what()));
        return false;
    }
}

bool LocalSocketChannel::computeXYSine(const QMap<QString, QVariant>& params, 
                                       XYSineResult& outResult,
                                       std::function<void(double, const QString&)> progressCallback)
{
    if (!isConnected()) {
        setError(TransportError::ConnectionFailed, QStringLiteral("computeXYSine: Not connected"));
        return false;
    }
    
    m_lastError = TransportError::NoError;
    
    try {
        // Generate unique job ID
        QString jobId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        
        // Store job ID for cancel access
        m_currentJobId = jobId;
        
        // Build ComputeSpec
        ComputeSpec spec;
        spec.set_feature_id("xy_sine");
        
        // Convert QVariant params to string map
        for (auto it = params.begin(); it != params.end(); ++it) {
            QString key = it.key();
            QVariant value = it.value();
            
            // Convert QVariant to string
            QString valueStr;
            if (value.userType() == QMetaType::Double) {
                valueStr = QString::number(value.toDouble());
            } else if (value.userType() == QMetaType::Int) {
                valueStr = QString::number(value.toInt());
            } else {
                valueStr = value.toString();
            }
            
            (*spec.mutable_params())[key.toStdString()] = valueStr.toStdString();
        }
        
        // Build StartJob
        StartJob startJob;
        startJob.mutable_job_id()->set_id(jobId.toStdString());
        *startJob.mutable_spec() = spec;
        
        // Serialize StartJob
        std::string serialized;
        if (!startJob.SerializeToString(&serialized)) {
            setError(TransportError::ProtocolError, QStringLiteral("computeXYSine: Failed to serialize StartJob"));
            return false;
        }
        
        QByteArray requestData(serialized.data(), static_cast<int>(serialized.size()));
        
        // CRITICAL FIX: Completely drain socket buffer before sending StartJob
        // This prevents old Pongs or partial frames from corrupting message alignment
        while (socket_->bytesAvailable() > 0) {
            socket_->read(socket_->bytesAvailable());
        }
        readBuffer_.clear();
        qCDebug(phoenixTransport) << "computeXYSine: Cleared socket buffer, bytesAvailable:" << socket_->bytesAvailable();
        
        // CRITICAL FIX: Disconnect async readyRead handler to prevent it from consuming StartReply
        // We'll use only blocking reads during computeXYSine()
        if (readyReadConnection_) {
            QObject::disconnect(readyReadConnection_);
            readyReadConnection_ = QMetaObject::Connection();
            qCDebug(phoenixTransport) << "computeXYSine: Disconnected async readyRead handler";
        }
        
        // Send StartJob
        if (!sendMessage(requestData)) {
            // Reconnect readyRead on error
            reconnectReadyRead();
            setError(TransportError::NetworkError, QStringLiteral("computeXYSine: Failed to send StartJob"));
            return false;
        }
        
        // Wait for StartReply (with timeout) - blocking read only
        qCDebug(phoenixTransport) << "computeXYSine: Waiting for StartReply (blocking read)";
        if (!socket_->waitForReadyRead(5000)) {
            reconnectReadyRead();
            setError(TransportError::ConnectionTimeout, QStringLiteral("computeXYSine: No StartReply received"));
            return false;
        }
        
        // Read StartReply using strict framed reads (not readAll)
        // Phase 1: Read exactly 4 bytes for message length
        if (socket_->bytesAvailable() < 4) {
            if (!socket_->waitForReadyRead(1000)) {
                reconnectReadyRead();
                setError(TransportError::ConnectionTimeout, QStringLiteral("computeXYSine: Incomplete StartReply header"));
                return false;
            }
        }
        
        qint32 msgLen;
        if (socket_->read(reinterpret_cast<char*>(&msgLen), 4) != 4) {
            reconnectReadyRead();
            setError(TransportError::NetworkError, QStringLiteral("computeXYSine: Failed to read StartReply length"));
            return false;
        }
        
        // Phase 2: Read exactly N bytes for message payload
        if (socket_->bytesAvailable() < msgLen) {
            if (!socket_->waitForReadyRead(1000)) {
                reconnectReadyRead();
                setError(TransportError::ConnectionTimeout, QStringLiteral("computeXYSine: Incomplete StartReply payload"));
                return false;
            }
        }
        
        QByteArray replyData = socket_->read(msgLen);
        if (replyData.size() != msgLen) {
            reconnectReadyRead();
            setError(TransportError::NetworkError, QStringLiteral("computeXYSine: Failed to read complete StartReply"));
            return false;
        }
        
        qCDebug(phoenixTransport) << "computeXYSine: Read StartReply, length:" << msgLen << "bytes";
        
        StartReply startReply;
        if (!startReply.ParseFromArray(replyData.data(), replyData.size())) {
            setError(TransportError::ProtocolError, QStringLiteral("computeXYSine: Failed to parse StartReply"));
            return false;
        }
        
        // Check status and map to specific error types
        QString status = QString::fromStdString(startReply.status());
        qCDebug(phoenixTransport) << "computeXYSine: Parsed StartReply, status:" << status;
        if (status != "OK") {
            QString errorMsg = QString::fromStdString(startReply.error_message());
            qCDebug(phoenixTransport) << "computeXYSine: StartReply error:" << errorMsg;
            
            // Map protocol status to specific TransportError codes
            TransportError errorCode;
            if (status == "INVALID_ARGUMENT") {
                errorCode = TransportError::InvalidArgument;
            } else if (status == "UNIMPLEMENTED") {
                errorCode = TransportError::Unimplemented;
            } else if (status == "RESOURCE_EXHAUSTED") {
                errorCode = TransportError::ResourceExhausted;
            } else if (status == "PERMISSION_DENIED") {
                errorCode = TransportError::PermissionDenied;
            } else {
                errorCode = TransportError::ServerError;  // Fallback
            }
            
            reconnectReadyRead();
            setError(errorCode, errorMsg);
            return false;
        }
        
        // Phase 2: Read messages in correct Bedrock order: Progress → ResultMeta → DataChunks[]
        enum class ReadPhase {
            WaitingForMeta,      // Waiting for ResultMeta (comes after Progress, before chunks)
            ReadingChunks,       // Reading DataChunks
            Done                  // All chunks received
        };
        ReadPhase currentPhase = ReadPhase::WaitingForMeta;
        
        QByteArray combinedData;
        ResultMeta resultMeta;
        bool resultMetaReceived = false;
        int expectedChunks = 0;
        int receivedChunks = 0;
        int64_t totalBytesReceived = 0;
        std::map<int, QByteArray> chunkMap;
        
        qCDebug(phoenixTransport) << "computeXYSine: Starting message loop, phase: WaitingForMeta";
        
        // Read messages until we have ResultMeta and all DataChunks
        while (currentPhase != ReadPhase::Done) {
            // Wait for next message
            if (!socket_->waitForReadyRead(10000)) {
                // Check if socket was closed by remote
                if (socket_->state() != QLocalSocket::ConnectedState) {
                    qCDebug(phoenixTransport) << "computeXYSine: Socket closed by remote, state:" << socket_->state() << "bytesAvailable:" << socket_->bytesAvailable();
                    // Try to read any remaining data from the socket buffer before giving up
                    if (socket_->bytesAvailable() > 0) {
                        qCDebug(phoenixTransport) << "computeXYSine: Reading" << socket_->bytesAvailable() << "remaining bytes from closed socket";
                        // Continue reading from buffer - will be handled in the read loop below
                    }
                    // If we've received ResultMeta, check if we have enough data
                    if (resultMetaReceived) {
                        int64_t bytesTotal = resultMeta.bytes_total();
                        // Calculate total bytes received so far
                        int64_t calculatedBytes = 0;
                        for (const auto& pair : chunkMap) {
                            calculatedBytes += pair.second.size();
                        }
                        // If we have all expected chunks OR enough bytes_total, this is OK
                        bool hasAllChunks = (receivedChunks >= expectedChunks && expectedChunks > 0);
                        bool hasEnoughBytes = (bytesTotal > 0 && calculatedBytes >= bytesTotal);
                        if (hasAllChunks || hasEnoughBytes) {
                            qCDebug(phoenixTransport) << "computeXYSine: Socket closed but have enough data (chunks:" << receivedChunks << "/" << expectedChunks << "bytes:" << calculatedBytes << "/" << bytesTotal << "), continuing";
                            currentPhase = ReadPhase::Done;
                            break;  // Exit loop gracefully
                        }
                        // If socket has bytes available, continue reading
                        if (socket_->bytesAvailable() > 0) {
                            qCDebug(phoenixTransport) << "computeXYSine: Socket closed but has" << socket_->bytesAvailable() << "bytes available, continuing to read";
                            // Continue to read loop below
                        } else {
                            reconnectReadyRead();
                            setError(TransportError::NetworkError, QStringLiteral("computeXYSine: Connection closed by Bedrock before all data received (chunks:%1/%2 bytes:%3/%4)")
                                     .arg(receivedChunks).arg(expectedChunks).arg(calculatedBytes).arg(bytesTotal));
                            return false;
                        }
                    } else {
                        reconnectReadyRead();
                        setError(TransportError::NetworkError, QStringLiteral("computeXYSine: Connection closed by Bedrock before ResultMeta received"));
                        return false;
                    }
                } else {
                    reconnectReadyRead();
                    setError(TransportError::ConnectionTimeout, QStringLiteral("computeXYSine: Timeout waiting for message"));
                    return false;
                }
            }
            
            // Strict framed read: Read exactly 4 bytes for message length
            if (socket_->bytesAvailable() < 4) {
                if (!socket_->waitForReadyRead(1000)) {
                    reconnectReadyRead();
                    setError(TransportError::ConnectionTimeout, QStringLiteral("computeXYSine: Incomplete message header"));
                    return false;
                }
            }
            
            qint32 msgLen;
            qint64 bytesRead = socket_->read(reinterpret_cast<char*>(&msgLen), 4);
            if (bytesRead != 4) {
                qCDebug(phoenixTransport) << "computeXYSine: Failed to read message length, bytesRead:" << bytesRead << "socket state:" << socket_->state();
                reconnectReadyRead();
                setError(TransportError::NetworkError, QStringLiteral("computeXYSine: Failed to read message length"));
                return false;
            }
            
            qCDebug(phoenixTransport) << "computeXYSine: Read message length:" << msgLen << "bytes, phase:" << (currentPhase == ReadPhase::WaitingForMeta ? "WaitingForMeta" : "ReadingChunks");
            
            // Read exactly N bytes for message payload
            if (socket_->bytesAvailable() < msgLen) {
                if (!socket_->waitForReadyRead(1000)) {
                    reconnectReadyRead();
                    setError(TransportError::ConnectionTimeout, QStringLiteral("computeXYSine: Incomplete message payload"));
                    return false;
                }
            }
            
            QByteArray messageData = socket_->read(msgLen);
            if (messageData.size() != msgLen) {
                reconnectReadyRead();
                setError(TransportError::NetworkError, QStringLiteral("computeXYSine: Failed to read complete message"));
                return false;
            }
            
            // Phase: WaitingForMeta - Look for ResultMeta FIRST (Bedrock sends it before chunks)
            if (currentPhase == ReadPhase::WaitingForMeta) {
                // CRITICAL: Distinguish Progress from ResultMeta
                // Both have job_id at field 1, but:
                // Progress: field 2 = progress_pct (double), field 3 = status (string)
                // ResultMeta: field 2 = status (string), field 3 = dtype (string)
                // 
                // Strategy: Parse as both and check which one makes more sense
                Progress progressMsg;
                ResultMeta testMeta;
                bool parsedAsProgress = progressMsg.ParseFromArray(messageData.data(), messageData.size());
                bool parsedAsResultMeta = testMeta.ParseFromArray(messageData.data(), messageData.size());
                
                if (parsedAsProgress) {
                    QString progressJobId = QString::fromStdString(progressMsg.job_id().id());
                    if (progressJobId == jobId) {
                        double progressPct = progressMsg.progress_pct();
                        QString progressStatus = QString::fromStdString(progressMsg.status());
                        
                        // Check if this is actually Progress by verifying it's NOT ResultMeta
                        // ResultMeta has: shape[], compute_elapsed_ms, bytes_total (fields 4, 5, 6)
                        // Progress doesn't have these fields
                        bool hasResultMetaFields = parsedAsResultMeta && 
                            (testMeta.shape_size() > 0 || 
                             testMeta.compute_elapsed_ms() != 0 ||
                             testMeta.bytes_total() != 0);
                        
                        // Also check: Progress typically has progress_pct in range [0, 100]
                        // And Progress messages are typically smaller (~59-65 bytes) vs ResultMeta (~65-70 bytes)
                        bool looksLikeProgress = (progressPct >= 0.0 && progressPct <= 100.0) && 
                                                (messageData.size() < 70) && 
                                                !hasResultMetaFields;
                        
                        if (looksLikeProgress && progressCallback) {
                            progressCallback(progressPct, progressStatus);
                            qCDebug(phoenixTransport) << "computeXYSine: Received Progress:" << progressPct << "% status:" << progressStatus;
                            continue;  // Continue to next message
                        }
                    }
                }
                
                // Try to parse as ResultMeta (check by shape/compute_elapsed_ms/bytes_total fields)
                // ResultMeta: job_id, status, dtype, shape[], compute_elapsed_ms, bytes_total
                if (parsedAsResultMeta) {
                    QString testJobId = QString::fromStdString(testMeta.job_id().id());
                    if (testJobId == jobId) {
                        // Check if it's ResultMeta by looking for ResultMeta-specific fields
                        // Fields 4, 5, 6: shape[], compute_elapsed_ms, bytes_total
                        bool hasShape = testMeta.shape_size() > 0;
                        bool hasComputeElapsed = testMeta.compute_elapsed_ms() != 0;
                        bool hasBytesTotal = testMeta.bytes_total() != 0;
                        QString dtype = QString::fromStdString(testMeta.dtype());
                        
                        // ResultMeta must have at least one of: shape, compute_elapsed_ms, bytes_total
                        // OR dtype must be a known dtype value (not a progress status string)
                        bool knownDtype = (dtype == "f64" || dtype == "f32" || dtype == "i64" || dtype == "i32");
                        if (hasShape || hasComputeElapsed || hasBytesTotal || knownDtype) {
                            // This is definitely ResultMeta
                            resultMeta = testMeta;
                            resultMetaReceived = true;
                            QString testStatus = QString::fromStdString(resultMeta.status());
                            qCDebug(phoenixTransport) << "computeXYSine: Received ResultMeta, status:" << testStatus << "dtype:" << dtype << "shape:" << (hasShape ? QString::number(resultMeta.shape(0)) : "none") << "bytes_total:" << resultMeta.bytes_total();
                            
                            currentPhase = ReadPhase::ReadingChunks;
                            qCDebug(phoenixTransport) << "computeXYSine: Phase transition: WaitingForMeta -> ReadingChunks";
                            continue;  // Continue to read chunks
                        }
                    }
                }
                
                // Unknown message type in WaitingForMeta phase
                qCDebug(phoenixTransport) << "computeXYSine: Unknown message type in WaitingForMeta phase, length:" << msgLen;
                continue;
            }
            
            // Phase: ReadingChunks - Read DataChunks until we have all data
            if (currentPhase == ReadPhase::ReadingChunks) {
                // Try to parse as Progress (optional, may come between chunks)
                // Progress has progress_pct field (double) - check if non-zero or status is set
                if (progressCallback) {
                    Progress progressMsg;
                    if (progressMsg.ParseFromArray(messageData.data(), messageData.size())) {
                        QString progressJobId = QString::fromStdString(progressMsg.job_id().id());
                        QString progressStatus = QString::fromStdString(progressMsg.status());
                        if (progressJobId == jobId && (progressMsg.progress_pct() != 0.0 || !progressStatus.isEmpty())) {
                            progressCallback(progressMsg.progress_pct(), progressStatus);
                            qCDebug(phoenixTransport) << "computeXYSine: Received Progress:" << progressMsg.progress_pct() << "%";
                            continue;  // Continue to next message
                        }
                    }
                }
                
                // Try to parse as DataChunk
                // DataChunk has chunk_index field (int32) - check if non-zero or has data
                DataChunk chunk;
                if (chunk.ParseFromArray(messageData.data(), messageData.size())) {
                    QString chunkJobId = QString::fromStdString(chunk.job_id().id());
                    if (chunkJobId == jobId) {
                        // Check if it's actually DataChunk by looking for chunk_index or data
                        bool hasChunkIndex = chunk.chunk_index() >= 0;  // proto3: default is 0, but valid indices are >= 0
                        bool hasData = chunk.data().size() > 0;
                        if (hasChunkIndex || hasData) {
                            // Verify it's actually DataChunk (has chunk_index or data)
                            chunkMap[chunk.chunk_index()] = QByteArray(chunk.data().data(), chunk.data().size());
                            expectedChunks = chunk.total_chunks();
                            receivedChunks++;
                            qCDebug(phoenixTransport) << "computeXYSine: Received DataChunk" << chunk.chunk_index() << "/" << expectedChunks;
                            
                            if (progressCallback && expectedChunks > 0) {
                                double chunkProgress = (receivedChunks * 100.0) / expectedChunks;
                                QString status = QString("Receiving data... (%1/%2)").arg(receivedChunks).arg(expectedChunks);
                                progressCallback(chunkProgress, status);
                            }
                            
                            // Check if we have all chunks
                            if (receivedChunks >= expectedChunks && expectedChunks > 0) {
                                currentPhase = ReadPhase::Done;
                                qCDebug(phoenixTransport) << "computeXYSine: Phase transition: ReadingChunks -> Done (all chunks received)";
                                break;  // Exit loop
                            }
                            continue;  // Continue to next message
                        }
                    }
                }
                
                // Unknown message type in ReadingChunks phase
                qCDebug(phoenixTransport) << "computeXYSine: Unknown message type in ReadingChunks phase, length:" << msgLen;
                // If we've received ResultMeta and have some chunks, but not all expected, continue waiting
                // Otherwise, this might be an error
                if (resultMetaReceived && receivedChunks > 0 && expectedChunks == 0) {
                    // No expected chunks info yet, keep reading
                    continue;
                }
            }
        }
        
        // Verify we received ResultMeta
        if (!resultMetaReceived) {
            reconnectReadyRead();
            setError(TransportError::ProtocolError, QStringLiteral("computeXYSine: ResultMeta not received"));
            return false;
        }
        
        // Now process ResultMeta (already parsed in the loop above)
        QString resultStatus = QString::fromStdString(resultMeta.status());
        qCDebug(phoenixTransport) << "computeXYSine: ResultMeta status:" << resultStatus << "length:" << resultStatus.length();
        if (resultStatus != "SUCCEEDED") {
            TransportError errorCode;
            if (resultStatus == "CANCELLED") {
                errorCode = TransportError::Cancelled;
            } else if (resultStatus == "FAILED") {
                errorCode = TransportError::ServerError;
            } else {
                errorCode = TransportError::ServerError;  // Fallback
            }
            
            QString errorMsg = QStringLiteral("computeXYSine: Computation %1").arg(resultStatus.isEmpty() ? "<empty>" : resultStatus);
            qCDebug(phoenixTransport) << "computeXYSine: ResultMeta status not SUCCEEDED, error:" << errorMsg;
            reconnectReadyRead();
            setError(errorCode, errorMsg);
            return false;
        }
        
        // Combine chunks in order
        // If we have fewer chunks than expected but have enough bytes_total, that's OK (socket may have closed early)
        int64_t bytesTotal = resultMeta.bytes_total();
        // totalBytesReceived was already calculated during chunk reception
        // Recalculate to be sure
        int64_t recalculatedBytes = 0;
        for (int i = 0; i < expectedChunks; ++i) {
            if (chunkMap.find(i) == chunkMap.end()) {
                // Missing chunk - check if we have enough bytes anyway
                if (bytesTotal > 0 && recalculatedBytes >= bytesTotal) {
                    qCDebug(phoenixTransport) << "computeXYSine: Missing chunk" << i << "but have enough bytes (" << recalculatedBytes << ">=" << bytesTotal << "), continuing";
                    break;
                }
                reconnectReadyRead();
                setError(TransportError::ProtocolError, QStringLiteral("computeXYSine: Missing chunk %1").arg(i));
                return false;
            }
            combinedData.append(chunkMap[i]);
            recalculatedBytes += chunkMap[i].size();
        }
        
        qCDebug(phoenixTransport) << "computeXYSine: Combined" << expectedChunks << "chunks, total bytes:" << combinedData.size() << "expected:" << bytesTotal;
        
        // Parse combined data into x[] and y[] arrays
        // Bedrock sends: x[] (doubles) followed by y[] (doubles)
        int totalBytes = combinedData.size();
        int expectedSamples = resultMeta.shape(0);
        int expectedBytes = expectedSamples * sizeof(double) * 2;  // x + y
        
        if (totalBytes < expectedBytes) {
            setError(TransportError::ProtocolError, QStringLiteral("computeXYSine: Insufficient data bytes - expected %1, got %2")
                     .arg(expectedBytes).arg(totalBytes));
            return false;
        }
        
        // Extract x array
        const double* xData = reinterpret_cast<const double*>(combinedData.data());
        outResult.x.clear();
        outResult.x.reserve(expectedSamples);
        for (int i = 0; i < expectedSamples; ++i) {
            outResult.x.push_back(xData[i]);
        }
        
        // Extract y array (starts after x array)
        const double* yData = reinterpret_cast<const double*>(combinedData.data() + expectedSamples * sizeof(double));
        outResult.y.clear();
        outResult.y.reserve(expectedSamples);
        for (int i = 0; i < expectedSamples; ++i) {
            outResult.y.push_back(yData[i]);
        }
        
        qCDebug(phoenixTransport) << "computeXYSine: Successfully computed" 
                 << outResult.x.size() << "samples";
        
        // Reconnect readyRead handler now that compute is complete
        reconnectReadyRead();
        qCDebug(phoenixTransport) << "computeXYSine: Successfully completed, reconnected readyRead handler";
        
        m_lastError = TransportError::NoError;
        return true;
        
    } catch (const std::exception& e) {
        reconnectReadyRead();
        setError(TransportError::NetworkError, QStringLiteral("computeXYSine: Exception: %1").arg(e.what()));
        return false;
    }
}

bool LocalSocketChannel::cancelJob(const QString& jobId)
{
    if (!isConnected()) {
        setError(TransportError::ConnectionFailed, QStringLiteral("cancelJob: Not connected"));
        return false;
    }
    
    // Use provided jobId or fall back to current job ID
    QString targetJobId = jobId.isEmpty() ? m_currentJobId : jobId;
    
    if (targetJobId.isEmpty()) {
        setError(TransportError::ProtocolError, QStringLiteral("cancelJob: No job ID available"));
        return false;
    }
    
    m_lastError = TransportError::NoError;
    
    try {
        // Build Cancel message
        Cancel cancelMsg;
        cancelMsg.mutable_job_id()->set_id(targetJobId.toStdString());
        
        // Serialize
        std::string serialized;
        if (!cancelMsg.SerializeToString(&serialized)) {
            setError(TransportError::ProtocolError, QStringLiteral("cancelJob: Failed to serialize Cancel"));
            return false;
        }
        
        QByteArray cancelData(serialized.data(), static_cast<int>(serialized.size()));
        
        // Send Cancel message
        if (!sendMessage(cancelData)) {
            setError(TransportError::NetworkError, QStringLiteral("cancelJob: Failed to send Cancel"));
            return false;
        }
        
        qCDebug(phoenixTransport) << "cancelJob: Sent cancel for job" << targetJobId;
        m_lastError = TransportError::NoError;
        return true;
    } catch (const std::exception& e) {
        setError(TransportError::NetworkError, QStringLiteral("cancelJob: Exception: %1").arg(e.what()));
        return false;
    }
}


