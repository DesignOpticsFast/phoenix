#include "ui/analysis/AnalysisWindow.hpp"
#include "ui/analysis/IAnalysisView.hpp"

#include <QVBoxLayout>

AnalysisWindow::AnalysisWindow(QWidget* parent)
    : QWidget(parent)
    , m_view(nullptr)
    , m_layout(new QVBoxLayout(this))
{
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);
}

AnalysisWindow::~AnalysisWindow() = default;

void AnalysisWindow::setView(std::unique_ptr<IAnalysisView> view) {
    // Remove old widget, if any
    if (m_view && m_view->widget()) {
        m_layout->removeWidget(m_view->widget());
        m_view->widget()->setParent(nullptr);
    }

    m_view = std::move(view);

    if (m_view && m_view->widget()) {
        m_layout->addWidget(m_view->widget());
    }
}

IAnalysisView* AnalysisWindow::view() const {
    return m_view.get();
}

