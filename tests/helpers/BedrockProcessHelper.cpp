#include "BedrockProcessHelper.hpp"
#include <QDir>
#include <QFileInfo>
#include <QDebug>

BedrockProcessHelper::BedrockProcessHelper()
    : process_(nullptr)
{
}

BedrockProcessHelper::~BedrockProcessHelper()
{
    stop();
    if (process_) {
        delete process_;
        process_ = nullptr;
    }
}

QString BedrockProcessHelper::findBedrockBinary()
{
    QString bedrockDir = "/home/ec2-user/workspace/bedrock";
    
    // Try build first (standard build directory), then fallback to other build dirs
    QStringList candidates = {
        bedrockDir + "/build/bedrock_server",
        bedrockDir + "/build/src/palantir/bedrock_server",
        bedrockDir + "/build-chunk4c/src/palantir/bedrock_server",
        bedrockDir + "/build-default/src/palantir/bedrock_server",
        bedrockDir + "/build-ccache-test/src/palantir/bedrock_server"
    };
    
    for (const QString& candidate : candidates) {
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
    }
    
    return QString();
}

bool BedrockProcessHelper::start(const QString& socketName)
{
    socketName_ = socketName;
    
    // Find Bedrock binary
    binaryPath_ = findBedrockBinary();
    if (binaryPath_.isEmpty()) {
        qWarning() << "Bedrock binary not found in any build directory";
        return false;
    }
    
    // Create log directory
    QDir().mkpath("chunk4c/logs");
    logFile_ = "chunk4c/logs/bedrock_runtime.log";
    
    // Start process
    process_ = new QProcess();
    process_->setProgram(binaryPath_);
    process_->setArguments({"--socket", socketName});
    
    // Redirect output to log file
    process_->setStandardOutputFile(logFile_);
    process_->setStandardErrorFile(logFile_);
    
    qDebug() << "Starting Bedrock:" << binaryPath_ << "with socket:" << socketName;
    
    process_->start();
    
    if (!process_->waitForStarted(5000)) {
        qWarning() << "Bedrock failed to start";
        return false;
    }
    
    // Wait for socket to be available (poll QLocalSocket::connectToServer)
    QLocalSocket testSocket;
    for (int i = 0; i < 50; ++i) {  // 5 second timeout
        testSocket.connectToServer(socketName);
        if (testSocket.waitForConnected(100)) {
            testSocket.disconnectFromServer();
            qDebug() << "Bedrock socket available after" << (i * 100) << "ms";
            return true;
        }
        QThread::msleep(100);
    }
    
    qWarning() << "Bedrock socket not available after 5 seconds";
    return false;
}

void BedrockProcessHelper::stop()
{
    if (process_ && process_->state() != QProcess::NotRunning) {
        qDebug() << "Stopping Bedrock server";
        process_->terminate();
        if (!process_->waitForFinished(3000)) {
            qDebug() << "Bedrock did not terminate gracefully, killing";
            process_->kill();
            process_->waitForFinished(1000);
        }
    }
}

bool BedrockProcessHelper::isRunning() const
{
    return process_ && process_->state() != QProcess::NotRunning;
}

