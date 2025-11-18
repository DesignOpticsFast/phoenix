#pragma once

#include <QWidget>
#include <memory>

class IAnalysisView;
class QVBoxLayout;

class AnalysisWindow : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisWindow(QWidget* parent = nullptr);
    ~AnalysisWindow() override;

    void setView(std::unique_ptr<IAnalysisView> view);
    IAnalysisView* view() const;

private:
    std::unique_ptr<IAnalysisView> m_view;
    QVBoxLayout* m_layout;
};

