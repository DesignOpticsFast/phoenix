#pragma once

#include <QMainWindow>
#include <memory>

class XYPlotViewGraphs;
class QToolBar;
class QAction;
class QWidget;
class FeatureParameterPanel;

class XYAnalysisWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit XYAnalysisWindow(QWidget* parent = nullptr);
    ~XYAnalysisWindow() override;

    void setFeature(const QString& featureId);
    
    // Public access to plot view for setting data
    XYPlotViewGraphs* plotView() const { return m_plotView; }

private slots:
    void onRunClicked();
    void onCancelClicked();
    void onCloseClicked();

private:
    void setupToolbar();
    void setupParameterPanel(const QString& featureId);
    
    XYPlotViewGraphs* m_plotView;
    QToolBar* m_toolbar;
    QAction* m_runAction;
    QAction* m_cancelAction;
    QAction* m_closeAction;
    FeatureParameterPanel* m_parameterPanel;
    QString m_currentFeatureId;
};

