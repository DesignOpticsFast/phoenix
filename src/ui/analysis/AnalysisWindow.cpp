#include "ui/analysis/AnalysisWindow.hpp"
#include "ui/analysis/IAnalysisView.hpp"
#include "ui/widgets/FeatureParameterPanel.hpp"
#include "features/FeatureRegistry.hpp"
#include "analysis/AnalysisWorker.hpp"
#include "analysis/AnalysisProgress.hpp"
#include "transport/LocalSocketChannel.hpp"
#include "app/LicenseManager.h"
#include "plot/XYPlotViewGraphs.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QProgressBar>
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <QPointF>
#include <vector>
#include <memory>

AnalysisWindow::AnalysisWindow(QWidget* parent)
    : QWidget(parent)
    , m_view(nullptr)
    , m_splitter(new QSplitter(Qt::Horizontal, this))
    , m_parameterPanel(nullptr)
    , m_runButton(nullptr)
    , m_cancelButton(nullptr)
    , m_progressBar(nullptr)
    , m_panelLayout(nullptr)
    , m_workerThread(nullptr)
    , m_worker(nullptr)
    , m_timeoutTimer(nullptr)
    , m_timeoutTimer(nullptr)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_splitter);
    setLayout(layout);
    
    // Register meta-types for signal/slot passing
    qRegisterMetaType<XYSineResult>("XYSineResult");
    qRegisterMetaType<AnalysisProgress>("AnalysisProgress");
}

AnalysisWindow::~AnalysisWindow() = default;

void AnalysisWindow::setView(std::unique_ptr<IAnalysisView> view) {
    // Remove old widget, if any
    if (m_view && m_view->widget()) {
        int index = m_splitter->indexOf(m_view->widget());
        if (index >= 0) {
            m_splitter->widget(index)->setParent(nullptr);
        }
    }

    m_view = std::move(view);

    if (m_view && m_view->widget()) {
        // Insert view as first widget (left side)
        m_splitter->insertWidget(0, m_view->widget());
        
        // Adjust splitter sizes if parameter panel exists
        if (m_parameterPanel) {
            m_splitter->setSizes({700, 300});  // 70% view, 30% params
        }
    }
}

IAnalysisView* AnalysisWindow::view() const {
    return m_view.get();
}

void AnalysisWindow::setFeature(const QString& featureId)
{
    setupParameterPanel(featureId);
}

void AnalysisWindow::setupParameterPanel(const QString& featureId)
{
    const FeatureDescriptor* desc = FeatureRegistry::instance().getFeature(featureId);
    if (!desc) {
        qWarning() << "AnalysisWindow: Feature not found:" << featureId;
        return;
    }
    
    m_currentFeatureId = featureId;
    
    // Remove existing parameter panel if any
    if (m_parameterPanel) {
        // Find the container widget in the splitter
        for (int i = 0; i < m_splitter->count(); ++i) {
            QWidget* widget = m_splitter->widget(i);
            if (widget && widget->layout() && widget->layout()->indexOf(m_parameterPanel) >= 0) {
                m_splitter->widget(i)->setParent(nullptr);
                delete widget;
                break;
            }
        }
        m_parameterPanel = nullptr;
        m_runButton = nullptr;
        m_cancelButton = nullptr;
        m_panelLayout = nullptr;
    }
    
    // Create container widget for parameter panel + Run button
    QWidget* panelContainer = new QWidget(this);
    m_panelLayout = new QVBoxLayout(panelContainer);
    m_panelLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create parameter panel
    m_parameterPanel = new FeatureParameterPanel(*desc, panelContainer);
    m_panelLayout->addWidget(m_parameterPanel);
    
    // Create progress bar (initially hidden)
    m_progressBar = new QProgressBar(panelContainer);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setVisible(false);
    m_panelLayout->addWidget(m_progressBar);
    
    // Create Cancel and Run buttons
    m_cancelButton = new QPushButton(tr("Cancel"), panelContainer);
    m_cancelButton->setMinimumHeight(32);
    m_cancelButton->setVisible(false);
    connect(m_cancelButton, &QPushButton::clicked, this, &AnalysisWindow::onCancelClicked);
    
    m_runButton = new QPushButton(tr("Run"), panelContainer);
    m_runButton->setMinimumHeight(32);
    connect(m_runButton, &QPushButton::clicked, this, &AnalysisWindow::runFeature);
    
    // Add buttons in horizontal layout (Cancel left, Run right)
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_runButton);
    m_panelLayout->addLayout(buttonLayout);
    
    panelContainer->setLayout(m_panelLayout);
    m_splitter->addWidget(panelContainer);
    
    // Set splitter sizes (70% view, 30% params)
    m_splitter->setSizes({700, 300});
    
    // Set minimum widths
    m_splitter->setChildrenCollapsible(false);
    panelContainer->setMinimumWidth(200);
}

void AnalysisWindow::runFeature()
{
    // Prevent double-click spam
    if (m_workerThread && m_workerThread->isRunning()) {
        return;
    }
    
    // Clean up any existing worker
    cleanupWorker();
    
    // Validate parameters (UI thread validation)
    if (!m_parameterPanel || !m_parameterPanel->isValid()) {
        QStringList errors = m_parameterPanel->validationErrors();
        QMessageBox::warning(this, tr("Invalid Parameters"),
            tr("Please correct the following parameter errors:\n\n%1")
            .arg(errors.join("\n")));
        return;
    }
    
    // Get parameters
    QMap<QString, QVariant> params = m_parameterPanel->parameters();
    
    // Create worker thread
    m_workerThread = new QThread(this);
    m_worker = new AnalysisWorker();
    
    // Move worker to thread
    m_worker->moveToThread(m_workerThread);
    
    // Set parameters
    m_worker->setParameters(m_currentFeatureId, params);
    
    // Connect signals (use QueuedConnection for cross-thread safety)
    connect(m_workerThread, &QThread::started, m_worker, &AnalysisWorker::run);
    connect(m_worker, &AnalysisWorker::progressChanged, this, &AnalysisWindow::onProgressChanged, Qt::QueuedConnection);
    connect(m_worker, &AnalysisWorker::finished, this, &AnalysisWindow::onWorkerFinished, Qt::QueuedConnection);
    connect(m_worker, &AnalysisWorker::cancelled, this, &AnalysisWindow::onWorkerCancelled, Qt::QueuedConnection);
    
    // Cleanup connections
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);
    
    // Disable Run button, show Cancel button and progress bar
    if (m_runButton) {
        m_runButton->setEnabled(false);
    }
    if (m_cancelButton) {
        m_cancelButton->setEnabled(true);
        m_cancelButton->setVisible(true);
    }
    if (m_progressBar) {
        m_progressBar->setValue(0);
        m_progressBar->setVisible(true);
    }
    
    // Start thread
    m_workerThread->start();
}

void AnalysisWindow::onCancelClicked()
{
    if (m_worker) {
        m_worker->requestCancel();
        // Disable Cancel button to prevent double-click spam
        if (m_cancelButton) {
            m_cancelButton->setEnabled(false);
        }
    }
}

void AnalysisWindow::onWorkerCancelled()
{
    // Stop timeout timer
    if (m_timeoutTimer && m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }
    
    // Hide progress bar and Cancel button
    if (m_progressBar) {
        m_progressBar->setVisible(false);
    }
    if (m_cancelButton) {
        m_cancelButton->setVisible(false);
        m_cancelButton->setEnabled(true);  // Re-enable for next run
    }
    
    // Re-enable Run button
    if (m_runButton) {
        m_runButton->setEnabled(true);
    }
    
    // Optionally show info message
    // For WP3.5.3, we'll keep it minimal - just update UI state
    
    cleanupWorker();
}

void AnalysisWindow::onProgressChanged(const AnalysisProgress& progress)
{
    if (m_progressBar) {
        m_progressBar->setValue(static_cast<int>(progress.progressPercent));
        m_progressBar->setToolTip(progress.status);
    }
}

void AnalysisWindow::onWorkerFinished(bool success, const QVariant& result, const QString& error)
{
    // Stop timeout timer
    if (m_timeoutTimer && m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }
    
    // Hide progress bar and Cancel button
    if (m_progressBar) {
        m_progressBar->setVisible(false);
    }
    if (m_cancelButton) {
        m_cancelButton->setVisible(false);
        m_cancelButton->setEnabled(true);  // Re-enable for next run
    }
    
    // Re-enable Run button
    if (m_runButton) {
        m_runButton->setEnabled(true);
    }
    
    // Handle error
    if (!success) {
        if (!error.isEmpty()) {
            QMessageBox::warning(this, tr("Computation Failed"), error);
        }
        cleanupWorker();
        return;
    }
    
    // Handle success - update plot
    if (m_currentFeatureId == "xy_sine") {
        XYSineResult xyResult = result.value<XYSineResult>();
        
        // Convert to QPointF vector
        std::vector<QPointF> points;
        points.reserve(xyResult.x.size());
        for (size_t i = 0; i < xyResult.x.size(); ++i) {
            points.emplace_back(xyResult.x[i], xyResult.y[i]);
        }
        
        // Update XYPlotViewGraphs
        XYPlotViewGraphs* xyView = dynamic_cast<XYPlotViewGraphs*>(m_view.get());
        if (xyView) {
            xyView->setData(points);
            qDebug() << "AnalysisWindow::onWorkerFinished: Updated plot with" << points.size() << "points";
        } else {
            qWarning() << "AnalysisWindow::onWorkerFinished: Current view is not XYPlotViewGraphs";
            QMessageBox::warning(this, tr("View Error"),
                tr("Current view does not support XY data display."));
        }
    }
    
    cleanupWorker();
}

void AnalysisWindow::cleanupWorker()
{
    if (m_workerThread) {
        if (m_workerThread->isRunning()) {
            m_workerThread->quit();
            m_workerThread->wait(1000);  // Wait up to 1 second
        }
        if (m_workerThread->isRunning()) {
            qWarning() << "AnalysisWindow::cleanupWorker: Thread did not stop, terminating";
            m_workerThread->terminate();
            m_workerThread->wait(1000);
        }
        m_workerThread->deleteLater();
        m_workerThread = nullptr;
    }
    
    // Worker will be deleted by thread's finished signal
    m_worker = nullptr;
}

