#include "LanguagePage.h"
#include <QApplication>
#include <QTranslator>
#include <QDebug>

LanguagePage::LanguagePage(QWidget *parent)
    : QWidget(parent)
    , m_languageCombo(nullptr)
    , m_currentLanguageLabel(nullptr)
    , m_settings("Phoenix", "PhoenixApp")
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
    m_languageCodes.clear();
    m_languageNames.clear();
    
    // Add supported languages
    m_languageCodes << "en" << "de" << "fr" << "es" << "zh_TW" << "zh_CN" << "ko" << "ja";
    m_languageNames << tr("English") 
                    << tr("German") 
                    << tr("French") 
                    << tr("Spanish")
                    << tr("Chinese (Traditional)")
                    << tr("Chinese (Simplified)")
                    << tr("Korean")
                    << tr("Japanese");
    
    m_languageCombo->clear();
    for (int i = 0; i < m_languageCodes.size(); ++i) {
        m_languageCombo->addItem(m_languageNames[i], m_languageCodes[i]);
    }
}

void LanguagePage::loadSettings()
{
    QString currentLanguage = m_settings.value("language", "en").toString();
    
    // Find and select current language
    int index = m_languageCodes.indexOf(currentLanguage);
    if (index >= 0) {
        m_languageCombo->setCurrentIndex(index);
    } else {
        m_languageCombo->setCurrentIndex(0); // Default to English
    }
    
    // Update current language display
    updateCurrentLanguageDisplay();
}

void LanguagePage::saveSettings()
{
    int index = m_languageCombo->currentIndex();
    if (index >= 0 && index < m_languageCodes.size()) {
        QString selectedLanguage = m_languageCodes[index];
        m_settings.setValue("language", selectedLanguage);
        emit languageChanged(selectedLanguage);
    }
}

void LanguagePage::onLanguageChanged(int index)
{
    if (index >= 0 && index < m_languageCodes.size()) {
        updateCurrentLanguageDisplay();
    }
}

void LanguagePage::updateCurrentLanguageDisplay()
{
    int index = m_languageCombo->currentIndex();
    if (index >= 0 && index < m_languageNames.size()) {
        m_currentLanguageLabel->setText(m_languageNames[index]);
    }
}
