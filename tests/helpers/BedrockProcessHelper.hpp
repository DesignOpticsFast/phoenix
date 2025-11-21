#pragma once

#include <QProcess>
#include <QString>
#include <QLocalSocket>
#include <QThread>

// Helper class to manage Bedrock server process for integration tests
class BedrockProcessHelper {
public:
    BedrockProcessHelper();
    ~BedrockProcessHelper();
    
    // Start Bedrock server with given socket name
    // Returns true if server started and socket is available
    bool start(const QString& socketName = "palantir_bedrock");
    
    // Stop Bedrock server (graceful shutdown, then kill if needed)
    void stop();
    
    // Check if Bedrock process is running
    bool isRunning() const;
    
    // Get the log file path
    QString logFile() const { return logFile_; }
    
    // Get the binary path used
    QString binaryPath() const { return binaryPath_; }

private:
    // Find Bedrock binary in various build directories
    QString findBedrockBinary();
    
    QProcess* process_;
    QString logFile_;
    QString binaryPath_;
    QString socketName_;
};

