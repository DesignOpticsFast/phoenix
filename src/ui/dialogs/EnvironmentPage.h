#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QTextEdit>
#include <QGroupBox>
#include <QSettings>

class EnvironmentPage : public QWidget
{
    Q_OBJECT

public:
    explicit EnvironmentPage(QWidget *parent = nullptr);
    ~EnvironmentPage() override;

    void loadSettings();
    void saveSettings();

private:
    void setupUI();
    void updateSystemInfo();
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_systemInfoGroup;
    QFormLayout* m_systemInfoLayout;
    QGroupBox* m_fontAwesomeGroup;
    QVBoxLayout* m_fontAwesomeLayout;
    
    // System information labels
    QLabel* m_qsettingsLocationLabel;
    QLabel* m_qtVersionLabel;
    QLabel* m_platformLabel;
    QLabel* m_appVersionLabel;
    
    // Font Awesome information
    QTextEdit* m_fontAwesomeTextEdit;
    
    // Settings
    QSettings* m_settings;
};
