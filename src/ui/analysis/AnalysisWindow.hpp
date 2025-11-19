#pragma once

#include <QWidget>
#include <memory>
#include "analysis/AnalysisProgress.hpp"

class IAnalysisView;
class QSplitter;
class FeatureParameterPanel;
class QPushButton;
class QVBoxLayout;
class QProgressBar;
class QThread;
class QTimer;
class AnalysisWorker;

class AnalysisWindow : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisWindow(QWidget* parent = nullptr);
    ~AnalysisWindow() override;

    void setView(std::unique_ptr<IAnalysisView> view);
    IAnalysisView* view() const;
    
    // Set feature and create parameter panel
    void setFeature(const QString& featureId);

private slots:
    void runFeature();
    void onCancelClicked();
    void onTimeout();
    void onWorkerFinished(bool success, const QVariant& result, const QString& error);
    void onWorkerCancelled();
    void onProgressChanged(const AnalysisProgress& progress);

private:
    void setupParameterPanel(const QString& featureId);
    void cleanupWorker();
    
    std::unique_ptr<IAnalysisView> m_view;
    QSplitter* m_splitter;
    FeatureParameterPanel* m_parameterPanel;
    QPushButton* m_runButton;
    QPushButton* m_cancelButton;
    QProgressBar* m_progressBar;
    QVBoxLayout* m_panelLayout;
    QString m_currentFeatureId;
    
    // Worker thread infrastructure
    QThread* m_workerThread;
    AnalysisWorker* m_worker;
    
    // Timeout handling
    QTimer* m_timeoutTimer;
    static constexpr int DEFAULT_ANALYSIS_TIMEOUT_SEC = 30;
};

