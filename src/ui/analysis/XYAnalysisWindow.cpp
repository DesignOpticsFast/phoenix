#include "ui/analysis/XYAnalysisWindow.hpp"
#include "ui/analysis/AnalysisWindowManager.hpp"
#include "plot/XYPlotViewGraphs.hpp"
#include "ui/widgets/FeatureParameterPanel.hpp"
#include "features/FeatureRegistry.hpp"
#include "analysis/AnalysisWorker.hpp"
#include "analysis/demo/XYSineDemo.hpp"
// TODO(Phase 3+): Re-enable license checks when LicenseManager is available
// #include "app/LicenseManager.h"
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSplitter>
#include <QMessageBox>
#include <QCloseEvent>
#include <QShowEvent>
#include <QThread>
#include <QPointF>
#include <QDebug>
#include <QLayout>
#include <QLayoutItem>

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
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        qInfo() << "[XYWIN] setupParameterPanel() called with featureId:" << featureId;
    }
#endif
    const FeatureDescriptor* desc = FeatureRegistry::instance().getFeature(featureId);
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        qInfo() << "[XYWIN] FeatureRegistry lookup result:" 
                << (desc ? "FOUND" : "NOT FOUND")
                << (desc ? QString("(id: %1, params: %2)").arg(desc->id()).arg(desc->params().size()) : "");
    }
#endif
    if (!desc) {
        qWarning() << "XYAnalysisWindow: Feature not found:" << featureId;
        return;
    }
    
    // If we already have a parameter panel, clean it up
    if (m_parameterPanel) {
        m_parameterPanel->setParent(nullptr);
        m_parameterPanel->deleteLater();
        m_parameterPanel = nullptr;
    }
    
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        qInfo() << "[XYWIN] Current centralWidget:" 
                << (centralWidget() ? centralWidget()->metaObject()->className() : "nullptr");
    }
#endif
    
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
    
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        qInfo() << "[XYWIN] Plot widget extracted - visible:" << (plotWidget ? plotWidget->isVisible() : false)
                << "size:" << (plotWidget ? plotWidget->size() : QSize());
    }
#endif
    
    // Create new splitter with plot view and parameter panel
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(plotWidget);
    
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        qInfo() << "[XYWIN] Splitter created - orientation: Horizontal"
                << "widgetCount:" << splitter->count();
        qInfo() << "[XYWIN] Plot widget added to splitter at index 0"
                << "plotWidget visible:" << (plotWidget ? plotWidget->isVisible() : false)
                << "plotWidget size:" << (plotWidget ? plotWidget->size() : QSize());
    }
#endif
    
    // Create parameter panel with feature descriptor
    m_parameterPanel = new FeatureParameterPanel(*desc, splitter);
    splitter->addWidget(m_parameterPanel);
    
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        qInfo() << "[XYWIN] FeatureParameterPanel created - pointer:" << (void*)m_parameterPanel
                << "panel visible:" << (m_parameterPanel ? m_parameterPanel->isVisible() : false)
                << "panel size:" << (m_parameterPanel ? m_parameterPanel->size() : QSize())
                << "panel minSize:" << (m_parameterPanel ? m_parameterPanel->minimumSize() : QSize());
        qInfo() << "[XYWIN] Panel added to splitter at index" << splitter->count() - 1
                << "splitter widgetCount:" << splitter->count()
                << "panel visible:" << (m_parameterPanel ? m_parameterPanel->isVisible() : false);
    }
#endif
    
    // Set sizes will be applied in showEvent() after window is shown
    // (splitter needs final size before setSizes() can work correctly)
    splitter->setChildrenCollapsible(false);
    m_parameterPanel->setMinimumWidth(200);
    
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        qInfo() << "[XYWIN] Splitter sizes set:" << splitter->sizes()
                << "splitter size:" << splitter->size()
                << "splitter visible:" << splitter->isVisible();
    }
#endif
    
    // Replace central widget with splitter
    setCentralWidget(splitter);
    
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        qInfo() << "[XYWIN] Splitter set as centralWidget"
                << "centralWidget visible:" << (centralWidget() ? centralWidget()->isVisible() : false)
                << "centralWidget size:" << (centralWidget() ? centralWidget()->size() : QSize());
    }
#endif
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
    
    // Validate Number of Samples parameter (handle both "samples" and "num_samples")
    QString samplesKey = params.contains("samples") ? "samples" : "num_samples";
    if (params.contains(samplesKey)) {
        bool ok;
        int numSamples = params.value(samplesKey).toInt(&ok);
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

void XYAnalysisWindow::showEvent(QShowEvent* event)
{
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        qInfo() << "[XYWIN] showEvent() - window visible:" << isVisible()
                << "window size:" << size()
                << "parameterPanel exists:" << (m_parameterPanel != nullptr)
                << "parameterPanel visible:" << (m_parameterPanel ? m_parameterPanel->isVisible() : false);
        
        if (m_parameterPanel) {
            qInfo() << "[XYWIN] Panel state in showEvent -"
                    << "size:" << m_parameterPanel->size()
                    << "minSize:" << m_parameterPanel->minimumSize()
                    << "parent:" << (m_parameterPanel->parent() ? m_parameterPanel->parent()->metaObject()->className() : "nullptr")
                    << "geometry:" << m_parameterPanel->geometry();
        }
    }
#endif
    
    QMainWindow::showEvent(event);
    
    // Set splitter sizes after window is shown (splitter now has final size)
    // This ensures correct sizing when the splitter has its actual dimensions
    if (QSplitter* splitter = qobject_cast<QSplitter*>(centralWidget())) {
        if (splitter->count() == 2) {
            splitter->setSizes({700, 300});
#ifndef NDEBUG
            if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
                qInfo() << "[XYWIN] Applied splitter sizes in showEvent():" << splitter->sizes();
            }
#endif
        }
    }
    
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        qInfo() << "[XYWIN] Dumping widget tree in showEvent():";
        dumpWidgetTree();
    }
#endif
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

#ifndef NDEBUG
void XYAnalysisWindow::dumpWidgetTree(QWidget* widget, int depth) const
{
    if (!qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        return;
    }
    
    if (!widget) {
        widget = const_cast<XYAnalysisWindow*>(this);
    }
    
    QString indent = QString("  ").repeated(depth);
    QString className = widget->metaObject()->className();
    QString objectName = widget->objectName().isEmpty() ? "<unnamed>" : widget->objectName();
    QRect geometry = widget->geometry();
    bool visible = widget->isVisible();
    bool hidden = widget->isHidden();
    QSize size = widget->size();
    QSize minSize = widget->minimumSize();
    QSize maxSize = widget->maximumSize();
    QWidget* parent = widget->parentWidget();
    
    qInfo() << QString("[TREE%1] %2%3 (class: %4)")
               .arg(depth)
               .arg(indent)
               .arg(objectName)
               .arg(className)
            << "visible:" << visible
            << "hidden:" << hidden
            << "geometry:" << geometry
            << "size:" << size
            << "minSize:" << minSize
            << "maxSize:" << maxSize
            << "parent:" << (parent ? parent->metaObject()->className() : "nullptr");
    
    // Special handling for QSplitter
    if (QSplitter* splitter = qobject_cast<QSplitter*>(widget)) {
        QList<int> sizes = splitter->sizes();
        QString sizesStr = "[";
        for (int i = 0; i < sizes.size(); ++i) {
            if (i > 0) sizesStr += ", ";
            sizesStr += QString::number(sizes[i]);
        }
        sizesStr += "]";
        qInfo() << QString("[TREE%1] %2  SPLITTER: count=%3 sizes=%4 orientation=%5")
                   .arg(depth)
                   .arg(indent)
                   .arg(splitter->count())
                   .arg(sizesStr)
                   .arg(splitter->orientation() == Qt::Horizontal ? "Horizontal" : "Vertical");
    }
    
    // Recurse into children
    for (QObject* child : widget->children()) {
        if (QWidget* childWidget = qobject_cast<QWidget*>(child)) {
            dumpWidgetTree(childWidget, depth + 1);
        }
    }
    
    // Also check layout children
    if (QLayout* layout = widget->layout()) {
        for (int i = 0; i < layout->count(); ++i) {
            QLayoutItem* item = layout->itemAt(i);
            if (item && item->widget()) {
                dumpWidgetTree(item->widget(), depth + 1);
            }
        }
    }
}
#endif
