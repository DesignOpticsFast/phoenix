#include "LanguagePage.h"
#include "app/LocaleInit.hpp"
#include "app/SettingsKeys.h"
#include <QMessageBox>
#include <QLocale>
#include <QSignalBlocker>
#include <QShowEvent>

namespace {
QString normalizedCode(const QString& value)
{
    const QString lowered = value.trimmed().toLower();
    if (lowered == QStringLiteral("de")) {
        return QStringLiteral("de");
    }
    return QStringLiteral("en");
}

QString humanNameFromCode(const QString& code)
{
    return (code == QStringLiteral("de")) ? QObject::tr("German") : QObject::tr("English");
}
}

LanguagePage::LanguagePage(QSettings& s, QWidget *parent)
    : QWidget(parent)
    , m_languageCombo(nullptr)
    , m_currentLanguageLabel(nullptr)
    , m_pendingLabel(nullptr)
    , m_settings(s)
    , m_initializing(false)
{
    setupUi();
    populateLanguages();
    loadSettings();
}

LanguagePage::~LanguagePage()
{
    // No specific cleanup needed
}

void LanguagePage::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Title
    QLabel* titleLabel = new QLabel(tr("Language Settings"), this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // Language selection form
    QFormLayout* formLayout = new QFormLayout();
    
    // Language combo box
    m_languageCombo = new QComboBox(this);
    m_languageCombo->setMinimumWidth(200);
    connect(m_languageCombo, QOverload<int>::of(&QComboBox::activated),
            this, &LanguagePage::onLanguageActivated);
    formLayout->addRow(tr("Interface Language:"), m_languageCombo);
    
    // Current language display
    m_currentLanguageLabel = new QLabel(this);
    m_currentLanguageLabel->setStyleSheet("color: #666; font-style: italic;");
    formLayout->addRow(tr("Current Language:"), m_currentLanguageLabel);
    
    mainLayout->addLayout(formLayout);

    m_pendingLabel = new QLabel(this);
    m_pendingLabel->setStyleSheet("color: #aa8800; font-style: italic;");
    m_pendingLabel->setVisible(false);
    mainLayout->addWidget(m_pendingLabel);
    
    // Info text
    QLabel* infoLabel = new QLabel(
        tr("Language changes will take effect after restarting the application."),
        this
    );
    infoLabel->setStyleSheet("color: #888; font-size: 11px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);
    
    mainLayout->addStretch();
}

void LanguagePage::populateLanguages()
{
    m_languageCodes = { QStringLiteral("en"), QStringLiteral("de") };
    m_languageNames = { tr("English"), tr("Deutsch") };

    m_languageCombo->clear();
    for (int i = 0; i < m_languageCodes.size(); ++i) {
        m_languageCombo->addItem(m_languageNames.at(i), m_languageCodes.at(i));
    }
}

void LanguagePage::loadSettings()
{
    m_initializing = true;

    QString stored = normalizedCode(m_settings.value(PhxKeys::UI_LANGUAGE, QStringLiteral("en")).toString());
    if (stored.isEmpty()) {
        stored = QStringLiteral("en");
    }

    QString applied = normalizedCode(m_settings.value(PhxKeys::UI_APPLIED_LANGUAGE).toString());
    if (applied.isEmpty()) {
        applied = stored;
    }

    m_storedLanguage = stored;
    m_appliedLanguage = applied;

    {
        QSignalBlocker blocker(m_languageCombo);
        int index = m_languageCodes.indexOf(m_storedLanguage);
        if (index < 0) {
            index = 0;
        }
        m_languageCombo->setCurrentIndex(index);
    }

    m_currentLanguageLabel->setText(humanNameFromCode(m_appliedLanguage));
    updatePendingStatus();

    m_initializing = false;
}

void LanguagePage::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    loadSettings();
}

void LanguagePage::saveSettings()
{
    // Settings are stored immediately when the user selects a language.
}

void LanguagePage::onLanguageActivated(int index)
{
    if (index < 0 || index >= m_languageCodes.size()) {
        return;
    }

    if (m_initializing) {
        return;
    }

    const QString code = m_languageCodes.at(index);
    applyLanguageSelection(code);
}

void LanguagePage::updatePendingStatus()
{
    const bool hasPending = !m_storedLanguage.isEmpty() && m_storedLanguage != m_appliedLanguage;
    if (hasPending) {
        m_pendingLabel->setText(tr("Pending: %1 (applies after restart)").arg(humanNameFromCode(m_storedLanguage)));
        m_pendingLabel->setVisible(true);
    } else {
        m_pendingLabel->clear();
        m_pendingLabel->setVisible(false);
    }
}

void LanguagePage::applyLanguageSelection(const QString& code)
{
    const QString normalized = normalizedCode(code);
    if (normalized.isEmpty() || normalized == m_storedLanguage) {
        updatePendingStatus();
        return;
    }

    m_settings.setValue(PhxKeys::UI_LANGUAGE, normalized);
    m_settings.setValue(PhxKeys::UI_LOCALE, i18n::localeForLanguage(normalized));
    m_settings.setValue(PhxKeys::I18N_LANGUAGE, normalized);
    m_settings.sync();

    m_storedLanguage = normalized;
    updatePendingStatus();

    QMessageBox::information(
        this,
        tr("Restart Required"),
        tr("Language changes will take effect after restarting the application."));
}
