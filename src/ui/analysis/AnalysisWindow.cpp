#include "ui/analysis/AnalysisWindow.hpp"
#include "ui/analysis/IAnalysisView.hpp"
#include "ui/widgets/FeatureParameterPanel.hpp"
#include "features/FeatureRegistry.hpp"
#include <QHBoxLayout>
#include <QSplitter>

AnalysisWindow::AnalysisWindow(QWidget* parent)
    : QWidget(parent)
    , m_view(nullptr)
    , m_splitter(new QSplitter(Qt::Horizontal, this))
    , m_parameterPanel(nullptr)
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
    
    // Remove existing parameter panel if any
    if (m_parameterPanel) {
        int index = m_splitter->indexOf(m_parameterPanel);
        if (index >= 0) {
            m_splitter->widget(index)->setParent(nullptr);
        }
        delete m_parameterPanel;
        m_parameterPanel = nullptr;
    }
    
    // Create new parameter panel
    m_parameterPanel = new FeatureParameterPanel(*desc, this);
    m_splitter->addWidget(m_parameterPanel);
    
    // Set splitter sizes (70% view, 30% params)
    m_splitter->setSizes({700, 300});
    
    // Set minimum widths
    m_splitter->setChildrenCollapsible(false);
    if (m_parameterPanel) {
        m_parameterPanel->setMinimumWidth(200);
    }
}

