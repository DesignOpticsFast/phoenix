#include "ui/analysis/XYAnalysisWindow.hpp"
#include "ui/analysis/AnalysisWindowManager.hpp"
#include "plot/XYPlotViewGraphs.hpp"
#include "ui/widgets/FeatureParameterPanel.hpp"
#include "features/FeatureRegistry.hpp"
#include "analysis/AnalysisWorker.hpp"
#include "analysis/AnalysisResults.hpp"  // For XYSineResult
#include "app/LicenseManager.h"
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSplitter>
#include <QMessageBox>
#include <QCloseEvent>
#include <QThread>
#include <QDebug>

XYAnalysisWindow::XYAnalysisWindow(QWidget* parent)
    : QMainWindow(nullptr)  // Force nullptr parent to create top-level window (macOS Z-order requirement)
    , m_plotView(nullptr)
    , m_toolbar(nullptr)
    , m_runAction(nullptr)
    , m_cancelAction(nullptr)
    , m_closeAction(nullptr)
    , m_parameterPanel(nullptr)
    , m_workerThread(nullptr)
    , m_worker(nullptr)
{
    setWindowTitle(tr("XY Plot Analysis"));
    resize(900, 600);
    
    // Ensure this is a top-level window and always stays above MainWindow.
    // QMainWindow already has Qt::Window by default; we just add the stays-on-top hint.
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    
    // Create XYPlotViewGraphs
    m_plotView = new XYPlotViewGraphs();
    
    // Set plot view as central widget initially (parameter panel will be added later if needed)
    setCentralWidget(m_plotView->widget());
    
    // Setup toolbar
    setupToolbar();
    
    // Set attribute for cleanup
    setAttribute(Qt::WA_DeleteOnClose);
    
    // Register with window manager
    AnalysisWindowManager::instance()->registerWindow(this);
}

XYAnalysisWindow::~XYAnalysisWindow()
{
    // Clean up worker thread if still running
    cleanupWorker();
}

void XYAnalysisWindow::setupToolbar()
{
    m_toolbar = addToolBar(tr("Analysis"));
    m_toolbar->setMovable(false);
    
    // Run action
    m_runAction = m_toolbar->addAction(tr("Run"));
    m_runAction->setToolTip(tr("Run analysis"));
    connect(m_runAction, &QAction::triggered, this, &XYAnalysisWindow::onRunClicked);
    
    // Cancel action (initially hidden)
    m_cancelAction = m_toolbar->addAction(tr("Cancel"));
    m_cancelAction->setToolTip(tr("Cancel running analysis"));
    m_cancelAction->setVisible(false);
    connect(m_cancelAction, &QAction::triggered, this, &XYAnalysisWindow::onCancelClicked);
    
    m_toolbar->addSeparator();
    
    // Close action
    m_closeAction = m_toolbar->addAction(tr("Close"));
    m_closeAction->setToolTip(tr("Close window"));
    connect(m_closeAction, &QAction::triggered, this, &XYAnalysisWindow::onCloseClicked);
}

void XYAnalysisWindow::setFeature(const QString& featureId)
{
    m_currentFeatureId = featureId;
    setupParameterPanel(featureId);
}

void XYAnalysisWindow::setupParameterPanel(const QString& featureId)
{
    const FeatureDescriptor* desc = FeatureRegistry::instance().getFeature(featureId);
    if (!desc) {
        qWarning() << "XYAnalysisWindow: Feature not found:" << featureId;
        return;
    }
    
    // If we already have a parameter panel, clean it up
    if (m_parameterPanel) {
        m_parameterPanel->deleteLater();
        m_parameterPanel = nullptr;
    }
    
    // Get the plot widget (may be in a splitter or directly as central widget)
    QWidget* plotWidget = nullptr;
    QWidget* central = centralWidget();
    if (QSplitter* splitter = qobject_cast<QSplitter*>(central)) {
        // Extract plot widget from splitter
        plotWidget = splitter->widget(0);  // Plot is always first widget
        plotWidget->setParent(nullptr);
        splitter->deleteLater();
    } else {
        // Plot widget is directly the central widget
        plotWidget = central;
        plotWidget->setParent(nullptr);
    }
    
    // Create new splitter with plot view and parameter panel
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(plotWidget);
    
    // Create parameter panel with feature descriptor
    m_parameterPanel = new FeatureParameterPanel(*desc, splitter);
    splitter->addWidget(m_parameterPanel);
    
    // Set sizes (70% plot, 30% params)
    splitter->setSizes({700, 300});
    splitter->setChildrenCollapsible(false);
    m_parameterPanel->setMinimumWidth(200);
    
    // Replace central widget with splitter
    setCentralWidget(splitter);
}

void XYAnalysisWindow::onRunClicked()
{
    if (!m_parameterPanel) {
        QMessageBox::information(this, tr("No Parameters"),
            tr("No parameter panel is configured. This is a display-only view."));
        return;
    }
    
    // Prevent double-click spam
    if (m_workerThread && m_workerThread->isRunning()) {
        return;
    }
    
    // Validate parameters
    if (!m_parameterPanel->isValid()) {
        QStringList errors = m_parameterPanel->validationErrors();
        QMessageBox::warning(this, tr("Invalid Parameters"),
            tr("Please correct the following parameter errors:\n\n%1")
            .arg(errors.join("\n")));
        return;
    }
    
    // Get parameters
    QMap<QString, QVariant> params = m_parameterPanel->parameters();
    
    // Validate Number of Samples parameter
    if (params.contains("num_samples")) {
        bool ok;
        int numSamples = params.value("num_samples").toInt(&ok);
        if (!ok || numSamples <= 0) {
            QMessageBox::warning(this, tr("Invalid Parameters"),
                tr("Number of Samples must be a positive integer."));
            return;
        }
    }
    
    // Clean up any existing worker
    cleanupWorker();
    
    // Create worker thread
    m_workerThread = new QThread(this);
    m_worker = new AnalysisWorker();
    
    // Move worker to thread
    m_worker->moveToThread(m_workerThread);
    
    // Set parameters
    m_worker->setParameters(m_currentFeatureId, params);
    
    // Connect signals (use QueuedConnection for cross-thread safety)
    connect(m_workerThread, &QThread::started, m_worker, &AnalysisWorker::run);
    connect(m_worker, &AnalysisWorker::finished, this, &XYAnalysisWindow::onWorkerFinished, Qt::QueuedConnection);
    connect(m_worker, &AnalysisWorker::cancelled, this, &XYAnalysisWindow::onWorkerCancelled, Qt::QueuedConnection);
    
    // Cleanup connections
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);
    
    // Disable Run button, show Cancel button
    if (m_runAction) {
        m_runAction->setEnabled(false);
    }
    if (m_cancelAction) {
        m_cancelAction->setEnabled(true);
        m_cancelAction->setVisible(true);
    }
    
    // Start thread
    m_workerThread->start();
}

void XYAnalysisWindow::onCancelClicked()
{
    if (m_worker) {
        m_worker->requestCancel();
    }
}

void XYAnalysisWindow::onCloseClicked()
{
    close();
}


void XYAnalysisWindow::onWorkerFinished(bool success, const QVariant& result, const QString& error)
{
    // Re-enable Run button, hide Cancel button
    if (m_runAction) {
        m_runAction->setEnabled(true);
    }
    if (m_cancelAction) {
        m_cancelAction->setVisible(false);
        m_cancelAction->setEnabled(true);  // Re-enable for next run
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
        if (m_plotView) {
            m_plotView->setData(points);
            qDebug() << "XYAnalysisWindow::onWorkerFinished: Updated plot with" << points.size() << "points";
        } else {
            qWarning() << "XYAnalysisWindow::onWorkerFinished: Plot view is null";
        }
    }
    
    cleanupWorker();
}

void XYAnalysisWindow::onWorkerCancelled()
{
    // Re-enable Run button, hide Cancel button
    if (m_runAction) {
        m_runAction->setEnabled(true);
    }
    if (m_cancelAction) {
        m_cancelAction->setVisible(false);
        m_cancelAction->setEnabled(true);
    }
    
    cleanupWorker();
}

void XYAnalysisWindow::cleanupWorker()
{
    if (m_workerThread) {
        if (m_workerThread->isRunning()) {
            m_workerThread->quit();
            m_workerThread->wait(1000);  // Wait up to 1 second
        }
        if (m_workerThread->isRunning()) {
            qWarning() << "XYAnalysisWindow::cleanupWorker: Thread did not stop, terminating";
            m_workerThread->terminate();
            m_workerThread->wait(1000);
        }
        m_workerThread = nullptr;
    }
    m_worker = nullptr;  // Will be deleted by deleteLater() connection
}

void XYAnalysisWindow::closeEvent(QCloseEvent* event)
{
    // Cancel any running analysis before closing
    if (m_worker) {
        m_worker->requestCancel();
    }
    
    // Clean up worker thread
    cleanupWorker();
    
    // Unregister from window manager before closing
    AnalysisWindowManager::instance()->unregisterWindow(this);
    
    // Call base class implementation
    QMainWindow::closeEvent(event);
}
