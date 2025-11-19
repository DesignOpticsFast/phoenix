#pragma once

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QSettings>

class EnvironmentPage;
class LanguagePage;
class AnalysisPage;
class MainWindow;

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~PreferencesDialog() override;
    
    MainWindow* mainWindow() const { return m_mainWindow; }

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onCategoryChanged();
    void onOK();
    void onCancel();
    void onApply();
    void onSettingsReset();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    
    // UI Components
    QSplitter* m_splitter;
    QListWidget* m_categoryList;
    QStackedWidget* m_contentStack;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QPushButton* m_applyButton;
    
    // Pages
    EnvironmentPage* m_environmentPage;
    LanguagePage* m_languagePage;
    AnalysisPage* m_analysisPage;
    
    // MainWindow reference (for accessing settings and restart dialog)
    MainWindow* m_mainWindow = nullptr;
    
    // Settings (reference obtained from MainWindow)
    QSettings& m_settings;
};
