#pragma once

#include <QMainWindow>
#include <memory>

class XYPlotViewGraphs;
class QToolBar;
class QAction;
class QWidget;
class FeatureParameterPanel;
class QCloseEvent;
class QThread;
class AnalysisWorker;

class XYAnalysisWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit XYAnalysisWindow(QWidget* parent = nullptr);
    ~XYAnalysisWindow() override;

    void setFeature(const QString& featureId);
    
    // Public access to plot view for setting data
    XYPlotViewGraphs* plotView() const { return m_plotView; }

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onRunClicked();
    void onCancelClicked();
    void onCloseClicked();
    void onWorkerFinished(bool success, const QVariant& result, const QString& error);
    void onWorkerCancelled();

private:
    void setupToolbar();
    void setupParameterPanel(const QString& featureId);
    void cleanupWorker();
    
    XYPlotViewGraphs* m_plotView;
    QToolBar* m_toolbar;
    QAction* m_runAction;
    QAction* m_cancelAction;
    QAction* m_closeAction;
    FeatureParameterPanel* m_parameterPanel;
    QString m_currentFeatureId;
    
    // Worker thread infrastructure
    QThread* m_workerThread;
    AnalysisWorker* m_worker;
};

