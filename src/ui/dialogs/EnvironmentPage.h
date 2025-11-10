#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QTextEdit>
#include <QGroupBox>
#include <QSettings>

class QPushButton;

class EnvironmentPage : public QWidget
{
    Q_OBJECT

public:
    explicit EnvironmentPage(QSettings& s, QWidget *parent = nullptr);
    ~EnvironmentPage() override;

    void loadSettings();
    void saveSettings();

signals:
    void settingsReset();

private slots:
    void onResetToDefaults();

private:
    void setupUI();
    void updateSystemInfo();
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_systemInfoGroup;
    QFormLayout* m_systemInfoLayout;
    QGroupBox* m_fontAwesomeGroup;
    QVBoxLayout* m_fontAwesomeLayout;
    QPushButton* m_resetButton;
    
    // System information labels
    QLabel* m_qsettingsLocationLabel;
    QLabel* m_qtVersionLabel;
    QLabel* m_platformLabel;
    QLabel* m_appVersionLabel;
    
    // Font Awesome information
    QTextEdit* m_fontAwesomeTextEdit;
    
    // Settings (reference to injected QSettings)
    QSettings& m_settings;
};
