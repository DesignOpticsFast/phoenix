#pragma once

#include "TransportClient.hpp"
#include "analysis/AnalysisResults.hpp"  // For XYSineResult
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMap>
#include <memory>
#include <vector>
#include <functional>

class QLocalSocket;
class QByteArray;

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
    // progressCallback: Optional callback for progress updates (progress_pct, status)
    bool computeXYSine(const QMap<QString, QVariant>& params, 
                       XYSineResult& outResult,
                       std::function<void(double, const QString&)> progressCallback = nullptr);
    
    // Cancel a running job
    // Returns true if cancel message was sent, false on error
    // If jobId is empty, uses the current job ID from the last computeXYSine() call
    bool cancelJob(const QString& jobId = QString());
    
    // Get the current job ID (from last computeXYSine() call)
    QString currentJobId() const { return m_currentJobId; }

private:
    QString socketName_;
    std::unique_ptr<QLocalSocket> socket_;
    QByteArray readBuffer_;
    bool connected_;
    QString m_currentJobId;  // Job ID from last computeXYSine() call
    
    // Helper methods for message framing
    bool sendMessage(const QByteArray& message);
    QByteArray readMessage();
    void parseIncomingData();
};

