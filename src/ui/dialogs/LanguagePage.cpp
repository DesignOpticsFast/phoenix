#include "LanguagePage.h"
#include "app/LocaleInit.hpp"
#include "app/SettingsKeys.h"
#include <QMessageBox>

LanguagePage::LanguagePage(QSettings& s, QWidget *parent)
    : QWidget(parent)
    , m_languageCombo(nullptr)
    , m_currentLanguageLabel(nullptr)
    , m_settings(s)
    , m_isInitializing(false)
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
    connect(m_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LanguagePage::onLanguageChanged);
    formLayout->addRow(tr("Interface Language:"), m_languageCombo);
    
    // Current language display
    m_currentLanguageLabel = new QLabel(this);
    m_currentLanguageLabel->setStyleSheet("color: #666; font-style: italic;");
    formLayout->addRow(tr("Current Language:"), m_currentLanguageLabel);
    
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    
    // Info text
    QLabel* infoLabel = new QLabel(
        tr("Language changes will take effect after restarting the application."),
        this
    );
    infoLabel->setStyleSheet("color: #888; font-size: 11px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);
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
    m_isInitializing = true;

    QString code = m_settings.value(PhxKeys::UI_LANGUAGE).toString();
    if (code.isEmpty()) {
        code = m_settings.value(PhxKeys::I18N_LANGUAGE).toString();
    }
    if (code != QStringLiteral("de")) {
        code = QStringLiteral("en");
    }

    m_selectedLanguage = code;

    int index = m_languageCodes.indexOf(code);
    if (index < 0) {
        index = 0;
    }

    m_languageCombo->setCurrentIndex(index);
    updateCurrentLanguageDisplay(index);

    m_isInitializing = false;
}

void LanguagePage::saveSettings()
{
    // Settings are stored immediately when the user selects a language.
}

void LanguagePage::onLanguageChanged(int index)
{
    if (index < 0 || index >= m_languageCodes.size()) {
        return;
    }

    updateCurrentLanguageDisplay(index);

    if (m_isInitializing) {
        return;
    }

    const QString code = m_languageCodes.at(index);
    applyLanguageSelection(code);
}

void LanguagePage::updateCurrentLanguageDisplay(int index)
{
    if (index >= 0 && index < m_languageNames.size()) {
        m_currentLanguageLabel->setText(m_languageNames.at(index));
    }
}

void LanguagePage::applyLanguageSelection(const QString& code)
{
    if (code.isEmpty() || code == m_selectedLanguage) {
        return;
    }

    m_settings.setValue(PhxKeys::UI_LANGUAGE, code);
    m_settings.setValue(PhxKeys::UI_LOCALE, i18n::localeForLanguage(code));
    m_settings.sync();

    // Legacy key update for compatibility
    m_settings.setValue(PhxKeys::I18N_LANGUAGE, code);

    m_selectedLanguage = code;

    QMessageBox::information(
        this,
        tr("Restart Required"),
        tr("Language will apply after restart.")
    );
}
