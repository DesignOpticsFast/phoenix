#pragma once

#include <QWidget>
#include <memory>

class IAnalysisView;
class QSplitter;
class FeatureParameterPanel;
class QPushButton;
class QVBoxLayout;
class QProgressBar;
class QThread;
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
    void onWorkerFinished(bool success, const QVariant& result, const QString& error);
    void onProgressChanged(const AnalysisProgress& progress);

private:
    void setupParameterPanel(const QString& featureId);
    void cleanupWorker();
    
    std::unique_ptr<IAnalysisView> m_view;
    QSplitter* m_splitter;
    FeatureParameterPanel* m_parameterPanel;
    QPushButton* m_runButton;
    QProgressBar* m_progressBar;
    QVBoxLayout* m_panelLayout;
    QString m_currentFeatureId;
    
    // Worker thread infrastructure
    QThread* m_workerThread;
    AnalysisWorker* m_worker;
};

