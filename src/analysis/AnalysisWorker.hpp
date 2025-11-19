#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QMap>
#include <atomic>
#include "AnalysisProgress.hpp"

class AnalysisWorker : public QObject {
    Q_OBJECT

public:
    explicit AnalysisWorker(QObject* parent = nullptr);
    ~AnalysisWorker() override;

    void setParameters(const QString& featureId, const QMap<QString, QVariant>& params);

public slots:
    void run();  // Executes compute in worker thread
    void requestCancel();  // Placeholder for WP3.5.3

signals:
    void started();
    void progressChanged(const AnalysisProgress& progress);
    void finished(bool success, const QVariant& result, const QString& error);
    void cancelled();  // Placeholder for WP3.5.3

private:
    void executeCompute();
    
    QString m_featureId;
    QMap<QString, QVariant> m_params;
    std::atomic<bool> m_cancelRequested;
    
    // For cancel support
    LocalSocketChannel* m_currentClient;
    QString m_currentJobId;
};

