#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSettings>

class LanguagePage : public QWidget
{
    Q_OBJECT

public:
    explicit LanguagePage(QSettings& s, QWidget *parent = nullptr);
    ~LanguagePage();

    void loadSettings();
    void saveSettings();

signals:
    void languageChanged(const QString& language);

private slots:
    void onLanguageChanged(int index);

private:
    void setupUi();
    void populateLanguages();
    void updateCurrentLanguageDisplay();

    QComboBox* m_languageCombo;
    QLabel* m_currentLanguageLabel;
    QSettings& m_settings;
    
    QStringList m_languageCodes;
    QStringList m_languageNames;
};
