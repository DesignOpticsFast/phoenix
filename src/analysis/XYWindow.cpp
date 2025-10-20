#include "XYWindow.hpp"
#include "../plot/QtGraphsPlotView.hpp"

#include <QMainWindow>
#include <QTabWidget>
#include <QToolBar>
#include <QAction>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QStatusBar>
#include <QDateTime>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QTextEdit>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>
#include <cmath>

XYWindow::XYWindow(QWidget *parent)
    : QMainWindow(parent)
    , toolbar_(nullptr)
    , tabWidget_(nullptr)
    , graphTab_(nullptr)
    , dataTab_(nullptr)
    , debugTab_(nullptr)
    , plotView_(nullptr)
    , dataTable_(nullptr)
    , debugLog_(nullptr)
    , computeSettingsAction_(nullptr)
    , displaySettingsAction_(nullptr)
    , updateAction_(nullptr)
    , cancelAction_(nullptr)
    , statusLabel_(nullptr)
    , progressBar_(nullptr)
    , amplitudeSpinBox_(nullptr)
    , frequencySpinBox_(nullptr)
    , phaseSpinBox_(nullptr)
    , cyclesSpinBox_(nullptr)
    , nSamplesSpinBox_(nullptr)
    , titleEdit_(nullptr)
    , xLabelEdit_(nullptr)
    , yLabelEdit_(nullptr)
    , currentJobId_()
    , jobRunning_(false)
    , palantirClient_(nullptr)
{
    setupUI();
    setupToolbar();
    setupTabs();
    setupComputeSettings();
    setupDisplaySettings();
    
    // Setup update timer
    updateTimer_.setInterval(100); // 10 Hz updates
    connect(&updateTimer_, &QTimer::timeout, this, &XYWindow::updateStatus);
}

XYWindow::~XYWindow() = default;

void XYWindow::setPalantirClient(PalantirClient* client)
{
    palantirClient_ = client;
    
    if (client) {
        connect(client, &PalantirClient::jobStarted, this, &XYWindow::onJobStarted);
        connect(client, &PalantirClient::jobProgress, this, &XYWindow::onJobProgress);
        connect(client, &PalantirClient::jobCompleted, this, &XYWindow::onJobCompleted);
        connect(client, &PalantirClient::jobFailed, this, &XYWindow::onJobFailed);
        connect(client, &PalantirClient::jobCancelled, this, &XYWindow::onJobCancelled);
    }
}

void XYWindow::setupUI()
{
    setWindowTitle("XY Sine Analysis");
    setMinimumSize(800, 600);
    resize(1000, 700);
    
    // Central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Status bar
    statusLabel_ = new QLabel("Ready", this);
    progressBar_ = new QProgressBar(this);
    progressBar_->setVisible(false);
    
    statusBar()->addWidget(statusLabel_);
    statusBar()->addPermanentWidget(progressBar_);
}

void XYWindow::setupToolbar()
{
    toolbar_ = addToolBar("Analysis Tools");
    
    // Compute Settings action
    computeSettingsAction_ = toolbar_->addAction("⚙️", this, &XYWindow::onComputeSettings);
    computeSettingsAction_->setToolTip("Compute Settings");
    
    // Display Settings action
    displaySettingsAction_ = toolbar_->addAction("🎨", this, &XYWindow::onDisplaySettings);
    displaySettingsAction_->setToolTip("Display Settings");
    
    toolbar_->addSeparator();
    
    // Update action
    updateAction_ = toolbar_->addAction("▶️", this, &XYWindow::onUpdate);
    updateAction_->setToolTip("Update Analysis");
    
    // Cancel action
    cancelAction_ = toolbar_->addAction("⏹️", this, &XYWindow::onCancel);
    cancelAction_->setToolTip("Cancel Analysis");
    cancelAction_->setEnabled(false);
}

void XYWindow::setupTabs()
{
    tabWidget_ = new QTabWidget(this);
    centralWidget()->layout()->addWidget(tabWidget_);
    
    // Graph tab
    graphTab_ = new QWidget();
    QVBoxLayout* graphLayout = new QVBoxLayout(graphTab_);
    plotView_ = new QtGraphsPlotView(graphTab_);
    graphLayout->addWidget(plotView_);
    tabWidget_->addTab(graphTab_, "Graph");
    
    // Data tab
    dataTab_ = new QWidget();
    QVBoxLayout* dataLayout = new QVBoxLayout(dataTab_);
    dataTable_ = new QTableWidget(dataTab_);
    dataTable_->setColumnCount(2);
    dataTable_->setHorizontalHeaderLabels({"X", "Y"});
    dataTable_->horizontalHeader()->setStretchLastSection(true);
    dataLayout->addWidget(dataTable_);
    tabWidget_->addTab(dataTab_, "Data");
    
    // Debug tab
    debugTab_ = new QWidget();
    QVBoxLayout* debugLayout = new QVBoxLayout(debugTab_);
    debugLog_ = new QTextEdit(debugTab_);
    debugLog_->setReadOnly(true);
    debugLog_->setMaximumHeight(200);
    debugLayout->addWidget(debugLog_);
    tabWidget_->addTab(debugTab_, "Debug");
}

void XYWindow::setupComputeSettings()
{
    // Create compute settings dialog
    QDialog* computeDialog = new QDialog(this);
    computeDialog->setWindowTitle("Compute Settings");
    computeDialog->setModal(true);
    
    QVBoxLayout* layout = new QVBoxLayout(computeDialog);
    
    // Amplitude
    QFormLayout* formLayout = new QFormLayout();
    amplitudeSpinBox_ = new QDoubleSpinBox();
    amplitudeSpinBox_->setRange(0.1, 100.0);
    amplitudeSpinBox_->setValue(1.0);
    amplitudeSpinBox_->setDecimals(2);
    formLayout->addRow("Amplitude:", amplitudeSpinBox_);
    
    // Frequency
    frequencySpinBox_ = new QDoubleSpinBox();
    frequencySpinBox_->setRange(0.1, 10.0);
    frequencySpinBox_->setValue(1.0);
    frequencySpinBox_->setDecimals(2);
    formLayout->addRow("Frequency:", frequencySpinBox_);
    
    // Phase
    phaseSpinBox_ = new QDoubleSpinBox();
    phaseSpinBox_->setRange(0.0, 2.0 * M_PI);
    phaseSpinBox_->setValue(0.0);
    phaseSpinBox_->setDecimals(3);
    formLayout->addRow("Phase:", phaseSpinBox_);
    
    // Cycles
    cyclesSpinBox_ = new QSpinBox();
    cyclesSpinBox_->setRange(1, 100);
    cyclesSpinBox_->setValue(1);
    formLayout->addRow("Cycles:", cyclesSpinBox_);
    
    // N Samples
    nSamplesSpinBox_ = new QSpinBox();
    nSamplesSpinBox_->setRange(100, 10000);
    nSamplesSpinBox_->setValue(500);
    formLayout->addRow("N Samples:", nSamplesSpinBox_);
    
    layout->addLayout(formLayout);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK");
    QPushButton* cancelButton = new QPushButton("Cancel");
    
    connect(okButton, &QPushButton::clicked, computeDialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, computeDialog, &QDialog::reject);
    
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    // Store dialog for later use
    computeDialog->hide();
}

void XYWindow::setupDisplaySettings()
{
    // Create display settings dialog
    QDialog* displayDialog = new QDialog(this);
    displayDialog->setWindowTitle("Display Settings");
    displayDialog->setModal(true);
    
    QVBoxLayout* layout = new QVBoxLayout(displayDialog);
    
    QFormLayout* formLayout = new QFormLayout();
    
    // Title
    titleEdit_ = new QLineEdit("XY Sine Wave");
    formLayout->addRow("Title:", titleEdit_);
    
    // X Label
    xLabelEdit_ = new QLineEdit("X");
    formLayout->addRow("X Label:", xLabelEdit_);
    
    // Y Label
    yLabelEdit_ = new QLineEdit("Y");
    formLayout->addRow("Y Label:", yLabelEdit_);
    
    layout->addLayout(formLayout);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK");
    QPushButton* cancelButton = new QPushButton("Cancel");
    
    connect(okButton, &QPushButton::clicked, displayDialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, displayDialog, &QDialog::reject);
    
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    // Store dialog for later use
    displayDialog->hide();
}

void XYWindow::onComputeSettings()
{
    // Show compute settings dialog
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Compute Settings");
    dialog->setModal(true);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    QFormLayout* formLayout = new QFormLayout();
    
    // Amplitude
    QDoubleSpinBox* amplitudeBox = new QDoubleSpinBox();
    amplitudeBox->setRange(0.1, 100.0);
    amplitudeBox->setValue(amplitudeSpinBox_->value());
    amplitudeBox->setDecimals(2);
    formLayout->addRow("Amplitude:", amplitudeBox);
    
    // Frequency
    QDoubleSpinBox* frequencyBox = new QDoubleSpinBox();
    frequencyBox->setRange(0.1, 10.0);
    frequencyBox->setValue(frequencySpinBox_->value());
    frequencyBox->setDecimals(2);
    formLayout->addRow("Frequency:", frequencyBox);
    
    // Phase
    QDoubleSpinBox* phaseBox = new QDoubleSpinBox();
    phaseBox->setRange(0.0, 2.0 * M_PI);
    phaseBox->setValue(phaseSpinBox_->value());
    phaseBox->setDecimals(3);
    formLayout->addRow("Phase:", phaseBox);
    
    // Cycles
    QSpinBox* cyclesBox = new QSpinBox();
    cyclesBox->setRange(1, 100);
    cyclesBox->setValue(cyclesSpinBox_->value());
    formLayout->addRow("Cycles:", cyclesBox);
    
    // N Samples
    QSpinBox* nSamplesBox = new QSpinBox();
    nSamplesBox->setRange(100, 10000);
    nSamplesBox->setValue(nSamplesSpinBox_->value());
    formLayout->addRow("N Samples:", nSamplesBox);
    
    layout->addLayout(formLayout);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK");
    QPushButton* cancelButton = new QPushButton("Cancel");
    
    connect(okButton, &QPushButton::clicked, dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);
    
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    if (dialog->exec() == QDialog::Accepted) {
        amplitudeSpinBox_->setValue(amplitudeBox->value());
        frequencySpinBox_->setValue(frequencyBox->value());
        phaseSpinBox_->setValue(phaseBox->value());
        cyclesSpinBox_->setValue(cyclesBox->value());
        nSamplesSpinBox_->setValue(nSamplesBox->value());
    }
}

void XYWindow::onDisplaySettings()
{
    // Show display settings dialog
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Display Settings");
    dialog->setModal(true);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    QFormLayout* formLayout = new QFormLayout();
    
    // Title
    QLineEdit* titleBox = new QLineEdit(titleEdit_->text());
    formLayout->addRow("Title:", titleBox);
    
    // X Label
    QLineEdit* xLabelBox = new QLineEdit(xLabelEdit_->text());
    formLayout->addRow("X Label:", xLabelBox);
    
    // Y Label
    QLineEdit* yLabelBox = new QLineEdit(yLabelEdit_->text());
    formLayout->addRow("Y Label:", yLabelBox);
    
    layout->addLayout(formLayout);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK");
    QPushButton* cancelButton = new QPushButton("Cancel");
    
    connect(okButton, &QPushButton::clicked, dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);
    
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    if (dialog->exec() == QDialog::Accepted) {
        titleEdit_->setText(titleBox->text());
        xLabelEdit_->setText(xLabelBox->text());
        yLabelEdit_->setText(yLabelBox->text());
        updatePlot();
    }
}

void XYWindow::onUpdate()
{
    if (jobRunning_) {
        return;
    }
    
    startXYSineJob();
}

void XYWindow::onCancel()
{
    if (!jobRunning_ || currentJobId_.isEmpty()) {
        return;
    }
    
    if (palantirClient_) {
        palantirClient_->cancelJob(currentJobId_);
    }
}

void XYWindow::onJobStarted(const QString& jobId)
{
    if (jobId == currentJobId_) {
        jobRunning_ = true;
        updateAction_->setEnabled(false);
        cancelAction_->setEnabled(true);
        progressBar_->setVisible(true);
        progressBar_->setValue(0);
        statusLabel_->setText("Job started: " + jobId);
    }
}

void XYWindow::onJobProgress(const QString& jobId, double progressPct, const QString& status)
{
    if (jobId == currentJobId_) {
        progressBar_->setValue(static_cast<int>(progressPct));
        statusLabel_->setText(QString("Progress: %1% - %2").arg(progressPct, 0, 'f', 1).arg(status));
    }
}

void XYWindow::onJobCompleted(const QString& jobId, const palantir::ResultMeta& meta)
{
    if (jobId == currentJobId_) {
        processXYSineResult(meta);
        jobRunning_ = false;
        updateAction_->setEnabled(true);
        cancelAction_->setEnabled(false);
        progressBar_->setVisible(false);
        statusLabel_->setText("Job completed: " + jobId);
    }
}

void XYWindow::onJobFailed(const QString& jobId, const QString& error)
{
    if (jobId == currentJobId_) {
        jobRunning_ = false;
        updateAction_->setEnabled(true);
        cancelAction_->setEnabled(false);
        progressBar_->setVisible(false);
        statusLabel_->setText("Job failed: " + error);
        
        QMessageBox::critical(this, "Job Failed", "Analysis failed: " + error);
    }
}

void XYWindow::onJobCancelled(const QString& jobId)
{
    if (jobId == currentJobId_) {
        jobRunning_ = false;
        updateAction_->setEnabled(true);
        cancelAction_->setEnabled(false);
        progressBar_->setVisible(false);
        statusLabel_->setText("Job cancelled: " + jobId);
    }
}

void XYWindow::updateStatus()
{
    // Update status display
    if (jobRunning_) {
        statusLabel_->setText("Running analysis...");
    } else {
        statusLabel_->setText("Ready");
    }
}

void XYWindow::updateProgress()
{
    // Update progress bar
    if (jobRunning_) {
        progressBar_->setVisible(true);
    } else {
        progressBar_->setVisible(false);
    }
}

void XYWindow::updatePlot()
{
    if (plotView_ && !xValues_.empty()) {
        plotView_->setData(xValues_, yValues_);
        plotView_->setTitle(titleEdit_->text());
        plotView_->setXLabel(xLabelEdit_->text());
        plotView_->setYLabel(yLabelEdit_->text());
    }
}

void XYWindow::updateDataTable()
{
    if (dataTable_ && !xValues_.empty()) {
        dataTable_->setRowCount(xValues_.size());
        
        for (size_t i = 0; i < xValues_.size(); ++i) {
            dataTable_->setItem(i, 0, new QTableWidgetItem(QString::number(xValues_[i])));
            dataTable_->setItem(i, 1, new QTableWidgetItem(QString::number(yValues_[i])));
        }
    }
}

void XYWindow::updateDebugLog()
{
    if (debugLog_) {
        debugLog_->append(QString("Analysis completed at %1").arg(QDateTime::currentDateTime().toString()));
        debugLog_->append(QString("Parameters: A=%1, f=%2, φ=%3, cycles=%4, n=%5")
                         .arg(amplitudeSpinBox_->value())
                         .arg(frequencySpinBox_->value())
                         .arg(phaseSpinBox_->value())
                         .arg(cyclesSpinBox_->value())
                         .arg(nSamplesSpinBox_->value()));
        debugLog_->append(QString("Data points: %1").arg(xValues_.size()));
    }
}

void XYWindow::computeXYSine()
{
    // Local computation for immediate results
    double amplitude = amplitudeSpinBox_->value();
    double frequency = frequencySpinBox_->value();
    double phase = phaseSpinBox_->value();
    int cycles = cyclesSpinBox_->value();
    int nSamples = nSamplesSpinBox_->value();
    
    xValues_.clear();
    yValues_.clear();
    
    double period = 2.0 * M_PI / frequency;
    double totalLength = cycles * period;
    double step = totalLength / nSamples;
    
    for (int i = 0; i < nSamples; ++i) {
        double x = i * step;
        double y = amplitude * std::sin(frequency * x + phase);
        
        xValues_.push_back(x);
        yValues_.push_back(y);
    }
    
    updatePlot();
    updateDataTable();
    updateDebugLog();
}

void XYWindow::startXYSineJob()
{
    if (!palantirClient_ || !palantirClient_->isConnected()) {
        // Fallback to local computation
        computeXYSine();
        return;
    }
    
    // Create compute spec
    palantir::ComputeSpec spec;
    spec.set_feature_id("xy_sine");
    spec.mutable_params()->insert({"amplitude", QString::number(amplitudeSpinBox_->value()).toStdString()});
    spec.mutable_params()->insert({"frequency", QString::number(frequencySpinBox_->value()).toStdString()});
    spec.mutable_params()->insert({"phase", QString::number(phaseSpinBox_->value()).toStdString()});
    spec.mutable_params()->insert({"cycles", QString::number(cyclesSpinBox_->value()).toStdString()});
    spec.mutable_params()->insert({"n_samples", QString::number(nSamplesSpinBox_->value()).toStdString()});
    
    // Start job
    currentJobId_ = palantirClient_->startJob(spec);
    if (currentJobId_.isEmpty()) {
        QMessageBox::warning(this, "Job Start Failed", "Failed to start analysis job");
        return;
    }
}

void XYWindow::processXYSineResult(const palantir::ResultMeta& meta)
{
    // Process the result metadata
    // TODO: Parse actual result data from meta
    // For now, use local computation as fallback
    computeXYSine();
}

void XYWindow::closeEvent(QCloseEvent *event)
{
    if (jobRunning_) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Cancel Analysis",
            "Analysis is running. Do you want to cancel and close?",
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            onCancel();
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

#include "XYWindow.moc"




