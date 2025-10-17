#pragma once

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QLocale>

class EnvironmentPrefs : public QDialog
{
    Q_OBJECT

public:
    explicit EnvironmentPrefs(QWidget *parent = nullptr);
    ~EnvironmentPrefs() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onLanguageChanged();
    void onDecimalSeparatorChanged();
    void onApply();
    void onOK();
    void onCancel();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    void updateLocale();
    
    // UI Components
    QListWidget* categoryList_;
    QStackedWidget* contentStack_;
    
    // Environment page
    QWidget* environmentPage_;
    QComboBox* languageCombo_;
    QButtonGroup* decimalGroup_;
    QRadioButton* inheritDecimal_;
    QRadioButton* commaDecimal_;
    QRadioButton* periodDecimal_;
    
    // Buttons
    QPushButton* applyButton_;
    QPushButton* okButton_;
    QPushButton* cancelButton_;
    
    // Settings
    QSettings* settings_;
    QLocale currentLocale_;
    QString currentDecimalSeparator_;
};
