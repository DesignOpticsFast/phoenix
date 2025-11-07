#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSettings>
#include <QStringList>

class LanguagePage : public QWidget
{
    Q_OBJECT

public:
    explicit LanguagePage(QSettings& s, QWidget *parent = nullptr);
    ~LanguagePage();

    void loadSettings();
    void saveSettings();

private slots:
    void onLanguageChanged(int index);

private:
    void setupUi();
    void populateLanguages();
    void updateCurrentLanguageDisplay(int index);
    void applyLanguageSelection(const QString& code);

    QComboBox* m_languageCombo;
    QLabel* m_currentLanguageLabel;
    QSettings& m_settings;

    QStringList m_languageCodes;
    QStringList m_languageNames;
    bool m_isInitializing;
    QString m_selectedLanguage;
};
