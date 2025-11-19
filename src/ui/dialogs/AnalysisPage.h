#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QSettings>

class AnalysisPage : public QWidget
{
    Q_OBJECT

public:
    explicit AnalysisPage(QSettings& s, QWidget *parent = nullptr);
    ~AnalysisPage() override;

    void loadSettings();
    void saveSettings();

private:
    void setupUI();
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_runBehaviorGroup;
    QVBoxLayout* m_runBehaviorLayout;
    QRadioButton* m_autoRunRadio;
    QRadioButton* m_showOptionsRadio;
    
    // Settings (reference to injected QSettings)
    QSettings& m_settings;
};

