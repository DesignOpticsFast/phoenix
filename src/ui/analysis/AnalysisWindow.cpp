#include "ui/analysis/AnalysisWindow.hpp"
#include "ui/analysis/IAnalysisView.hpp"
#include "ui/widgets/FeatureParameterPanel.hpp"
#include "features/FeatureRegistry.hpp"
#include "transport/LocalSocketChannel.hpp"
#include "transport/TransportFactory.hpp"
#include "app/LicenseManager.h"
#include "plot/XYPlotViewGraphs.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QMessageBox>
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
    , m_panelLayout(nullptr)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_splitter);
    setLayout(layout);
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
        m_panelLayout = nullptr;
    }
    
    // Create container widget for parameter panel + Run button
    QWidget* panelContainer = new QWidget(this);
    m_panelLayout = new QVBoxLayout(panelContainer);
    m_panelLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create parameter panel
    m_parameterPanel = new FeatureParameterPanel(*desc, panelContainer);
    m_panelLayout->addWidget(m_parameterPanel);
    
    // Create Run button
    m_runButton = new QPushButton(tr("Run"), panelContainer);
    m_runButton->setMinimumHeight(32);
    connect(m_runButton, &QPushButton::clicked, this, &AnalysisWindow::runFeature);
    
    // Add button in horizontal layout (aligned right)
    QHBoxLayout* buttonLayout = new QHBoxLayout();
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
    // Check license
    LicenseManager* mgr = LicenseManager::instance();
    if (mgr->currentState() != LicenseManager::LicenseState::NotConfigured &&
        !mgr->hasFeature("feature_xy_plot")) {
        QMessageBox::warning(this, tr("Feature Unavailable"),
            tr("XY Sine computation requires a valid license with the 'feature_xy_plot' feature.\n\n"
               "Please check your license status via Help → License..."));
        return;
    }
    
    // Validate parameters
    if (!m_parameterPanel || !m_parameterPanel->isValid()) {
        QStringList errors = m_parameterPanel->validationErrors();
        QMessageBox::warning(this, tr("Invalid Parameters"),
            tr("Please correct the following parameter errors:\n\n%1")
            .arg(errors.join("\n")));
        return;
    }
    
    // Get parameters
    QMap<QString, QVariant> params = m_parameterPanel->parameters();
    
    // Create LocalSocketChannel (explicitly, not from env)
    auto client = std::make_unique<LocalSocketChannel>();
    
    // Connect
    if (!client->connect()) {
        QMessageBox::warning(this, tr("Connection Failed"),
            tr("Failed to connect to Bedrock server.\n\n"
               "Please ensure Bedrock is running and accessible via LocalSocket."));
        return;
    }
    
    // Compute XY Sine
    XYSineResult result;
    if (!client->computeXYSine(params, result)) {
        QMessageBox::warning(this, tr("Computation Failed"),
            tr("XY Sine computation failed.\n\n"
               "Please check the server logs for details."));
        client->disconnect();
        return;
    }
    
    client->disconnect();
    
    // Update plot view
    if (result.x.size() != result.y.size()) {
        qWarning() << "AnalysisWindow::runFeature: Mismatched x/y array sizes";
        QMessageBox::warning(this, tr("Data Error"),
            tr("Received invalid data from server (mismatched array sizes)."));
        return;
    }
    
    // Convert to QPointF vector
    std::vector<QPointF> points;
    points.reserve(result.x.size());
    for (size_t i = 0; i < result.x.size(); ++i) {
        points.emplace_back(result.x[i], result.y[i]);
    }
    
    // Update XYPlotViewGraphs
    XYPlotViewGraphs* xyView = dynamic_cast<XYPlotViewGraphs*>(m_view.get());
    if (xyView) {
        xyView->setData(points);
        qDebug() << "AnalysisWindow::runFeature: Updated plot with" << points.size() << "points";
    } else {
        qWarning() << "AnalysisWindow::runFeature: Current view is not XYPlotViewGraphs";
        QMessageBox::warning(this, tr("View Error"),
            tr("Current view does not support XY data display."));
    }
}

