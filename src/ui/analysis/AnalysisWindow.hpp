#pragma once

#include <QWidget>
#include <memory>

class IAnalysisView;
class QSplitter;
class FeatureParameterPanel;
class QPushButton;
class QVBoxLayout;

class AnalysisWindow : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisWindow(QWidget* parent = nullptr);
    ~AnalysisWindow() override;

    void setView(std::unique_ptr<IAnalysisView> view);
    IAnalysisView* view() const;
    
    // Set feature and create parameter panel
    void setFeature(const QString& featureId);

private slots:
    void runFeature();

private:
    void setupParameterPanel(const QString& featureId);
    
    std::unique_ptr<IAnalysisView> m_view;
    QSplitter* m_splitter;
    FeatureParameterPanel* m_parameterPanel;
    QPushButton* m_runButton;
    QVBoxLayout* m_panelLayout;
    QString m_currentFeatureId;
};

