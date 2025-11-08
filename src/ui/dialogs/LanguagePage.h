#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSettings>
#include <QStringList>

namespace i18n {
QString localeForLanguage(const QString& shortLang);
}

class LanguagePage : public QWidget
{
    Q_OBJECT

public:
    explicit LanguagePage(QSettings& s, QWidget *parent = nullptr);
    ~LanguagePage();

    void loadSettings();
    void saveSettings();

private slots:
    void onLanguageActivated(int index);

protected:
    void showEvent(QShowEvent* e) override;

private:
    void setupUi();
    void populateLanguages();
    void updatePendingStatus();
    void applyLanguageSelection(const QString& code);

    QComboBox* m_languageCombo;
    QLabel* m_currentLanguageLabel;
    QLabel* m_pendingLabel;
    QSettings& m_settings;

    QStringList m_languageCodes;
    QStringList m_languageNames;
    bool m_initializing = false;
    QString m_storedLanguage;
    QString m_appliedLanguage;
};
