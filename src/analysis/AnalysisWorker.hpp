#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QMap>
#include <atomic>
#include <memory>

// Forward declaration
class IAnalysisExecutor;

// Analysis run mode (Strategy pattern selection)
enum class AnalysisRunMode {
    LocalOnly,   // Use LocalExecutor (existing local compute path)
    RemoteOnly   // Use RemoteExecutor (future Bedrock communication)
};

class AnalysisWorker : public QObject {
    Q_OBJECT

public:
    explicit AnalysisWorker(QObject* parent = nullptr);
    ~AnalysisWorker() override;

    void setParameters(const QString& featureId, const QMap<QString, QVariant>& params);
    
    // Set execution mode (WP1: Strategy pattern integration)
    void setRunMode(AnalysisRunMode mode);
    AnalysisRunMode runMode() const { return m_runMode; }

public slots:
    void run();  // Executes compute in worker thread
    void requestCancel();  // Placeholder for WP3.5.2

signals:
    void started();
    void finished(bool success, const QVariant& result, const QString& error);
    void cancelled();  // Placeholder for WP3.5.2

private:
    void executeCompute();
    void executeWithExecutor();  // New: Uses executor pattern
    
    QString m_featureId;
    QMap<QString, QVariant> m_params;
    std::atomic<bool> m_cancelRequested;
    
    // WP1: Strategy pattern - executor selection
    AnalysisRunMode m_runMode;
    std::unique_ptr<IAnalysisExecutor> m_localExecutor;
    std::unique_ptr<IAnalysisExecutor> m_remoteExecutor;
};

