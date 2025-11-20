#include "ui/analysis/XYAnalysisWindow.hpp"
#include "ui/analysis/AnalysisWindowManager.hpp"
#include "plot/XYPlotViewGraphs.hpp"
#include "ui/widgets/FeatureParameterPanel.hpp"
#include "features/FeatureRegistry.hpp"
#include "app/LicenseManager.h"
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSplitter>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>

XYAnalysisWindow::XYAnalysisWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_plotView(nullptr)
    , m_toolbar(nullptr)
    , m_runAction(nullptr)
    , m_cancelAction(nullptr)
    , m_closeAction(nullptr)
    , m_parameterPanel(nullptr)
{
    setWindowTitle(tr("XY Plot Analysis"));
    resize(900, 600);
    
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

XYAnalysisWindow::~XYAnalysisWindow() = default;

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
    
    // Validate parameters
    if (!m_parameterPanel->isValid()) {
        QStringList errors = m_parameterPanel->validationErrors();
        QMessageBox::warning(this, tr("Invalid Parameters"),
            tr("Please correct the following parameter errors:\n\n%1")
            .arg(errors.join("\n")));
        return;
    }
    
    // For now, just show a message (analysis execution will be added later)
    QMessageBox::information(this, tr("Run Analysis"),
        tr("Analysis execution will be implemented in a future chunk."));
}

void XYAnalysisWindow::onCancelClicked()
{
    // Cancel action (will be implemented when analysis execution is added)
    QMessageBox::information(this, tr("Cancel"),
        tr("Cancel functionality will be implemented when analysis execution is added."));
}

void XYAnalysisWindow::onCloseClicked()
{
    close();
}


void XYAnalysisWindow::closeEvent(QCloseEvent* event)
{
    // Unregister from window manager before closing
    AnalysisWindowManager::instance()->unregisterWindow(this);
    
    // Call base class implementation
    QMainWindow::closeEvent(event);
}
