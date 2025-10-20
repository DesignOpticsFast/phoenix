#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QToolBar>
#include <QAction>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QTextEdit>
#include <QProgressBar>
#include <QTimer>
#include <memory>

#include "../palantir/PalantirClient.hpp"
#include "../palantir/palantir.pb.h"

class QtGraphsPlotView;

class XYWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit XYWindow(QWidget *parent = nullptr);
    ~XYWindow() override;

    void setPalantirClient(PalantirClient* client);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // Toolbar actions
    void onComputeSettings();
    void onDisplaySettings();
    void onUpdate();
    void onCancel();
    
    // Palantir client slots
    void onJobStarted(const QString& jobId);
    void onJobProgress(const QString& jobId, double progressPct, const QString& status);
    void onJobCompleted(const QString& jobId, const palantir::ResultMeta& meta);
    void onJobFailed(const QString& jobId, const QString& error);
    void onJobCancelled(const QString& jobId);
    
    // UI updates
    void updateStatus();
    void updateProgress();

private:
    void setupUI();
    void setupToolbar();
    void setupTabs();
    void setupComputeSettings();
    void setupDisplaySettings();
    void updatePlot();
    void updateDataTable();
    void updateDebugLog();
    
    // XY Sine computation
    void computeXYSine();
    void startXYSineJob();
    void processXYSineResult(const palantir::ResultMeta& meta);
    
    // UI Components
    QToolBar* toolbar_;
    QTabWidget* tabWidget_;
    
    // Tabs
    QWidget* graphTab_;
    QWidget* dataTab_;
    QWidget* debugTab_;
    
    // Graph tab
    QtGraphsPlotView* plotView_;
    
    // Data tab
    QTableWidget* dataTable_;
    
    // Debug tab
    QTextEdit* debugLog_;
    
    // Toolbar actions
    QAction* computeSettingsAction_;
    QAction* displaySettingsAction_;
    QAction* updateAction_;
    QAction* cancelAction_;
    
    // Status
    QLabel* statusLabel_;
    QProgressBar* progressBar_;
    
    // Compute settings
    QDoubleSpinBox* amplitudeSpinBox_;
    QDoubleSpinBox* frequencySpinBox_;
    QDoubleSpinBox* phaseSpinBox_;
    QSpinBox* cyclesSpinBox_;
    QSpinBox* nSamplesSpinBox_;
    
    // Display settings
    QLineEdit* titleEdit_;
    QLineEdit* xLabelEdit_;
    QLineEdit* yLabelEdit_;
    
    // Data
    std::vector<double> xValues_;
    std::vector<double> yValues_;
    QString currentJobId_;
    bool jobRunning_;
    
    // Palantir client
    PalantirClient* palantirClient_;
    
    // Update timer
    QTimer updateTimer_;
};




