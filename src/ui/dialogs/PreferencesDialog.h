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

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onCategoryChanged();
    void onOK();
    void onCancel();
    void onApply();

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
    
    // Settings
    QSettings* m_settings;
};
