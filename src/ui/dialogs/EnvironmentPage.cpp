#include "EnvironmentPage.h"
#include "../icons/IconBootstrap.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QTextEdit>
#include <QGroupBox>
#include <QSettings>
#include <QApplication>
#include <QStandardPaths>
#include <QDebug>

EnvironmentPage::EnvironmentPage(QSettings& s, QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_systemInfoGroup(nullptr)
    , m_systemInfoLayout(nullptr)
    , m_fontAwesomeGroup(nullptr)
    , m_fontAwesomeLayout(nullptr)
    , m_qsettingsLocationLabel(nullptr)
    , m_qtVersionLabel(nullptr)
    , m_platformLabel(nullptr)
    , m_appVersionLabel(nullptr)
    , m_fontAwesomeTextEdit(nullptr)
    , m_settings(s)
{
    setupUI();
    updateSystemInfo();
}

EnvironmentPage::~EnvironmentPage()
{
}

void EnvironmentPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // System Information Group
    m_systemInfoGroup = new QGroupBox(tr("System Information"), this);
    m_systemInfoLayout = new QFormLayout(m_systemInfoGroup);
    
    // QSettings Location
    m_qsettingsLocationLabel = new QLabel();
    m_systemInfoLayout->addRow(tr("QSettings Location:"), m_qsettingsLocationLabel);
    
    // Qt Version
    m_qtVersionLabel = new QLabel();
    m_systemInfoLayout->addRow(tr("Qt Version:"), m_qtVersionLabel);
    
    // Platform
    m_platformLabel = new QLabel();
    m_systemInfoLayout->addRow(tr("Platform:"), m_platformLabel);
    
    // Application Version
    m_appVersionLabel = new QLabel();
    m_systemInfoLayout->addRow(tr("Application Version:"), m_appVersionLabel);
    
    m_mainLayout->addWidget(m_systemInfoGroup);
    
    // Font Awesome Information Group
    m_fontAwesomeGroup = new QGroupBox(tr("Font Awesome Families"), this);
    m_fontAwesomeLayout = new QVBoxLayout(m_fontAwesomeGroup);
    
    m_fontAwesomeTextEdit = new QTextEdit();
    m_fontAwesomeTextEdit->setReadOnly(true);
    m_fontAwesomeTextEdit->setMaximumHeight(150);
    m_fontAwesomeLayout->addWidget(m_fontAwesomeTextEdit);
    
    m_mainLayout->addWidget(m_fontAwesomeGroup);
    
    // Add stretch to push everything to the top
    m_mainLayout->addStretch();
}

void EnvironmentPage::loadSettings()
{
    // Load any page-specific settings if needed
    // For now, this page is read-only
}

void EnvironmentPage::saveSettings()
{
    // Save any page-specific settings if needed
    // For now, this page is read-only
}

void EnvironmentPage::updateSystemInfo()
{
    // QSettings Location
    QString settingsPath = m_settings.fileName();
    m_qsettingsLocationLabel->setText(settingsPath);
    m_qsettingsLocationLabel->setToolTip(settingsPath);
    
    // Qt Version
    QString qtVersion = QString("Qt %1").arg(QT_VERSION_STR);
    m_qtVersionLabel->setText(qtVersion);
    
    // Platform
    QString platform = QString("%1 (%2)").arg(QApplication::platformName()).arg(QSysInfo::prettyProductName());
    m_platformLabel->setText(platform);
    
    // Application Version
    QString appVersion = QApplication::applicationVersion();
    if (appVersion.isEmpty()) {
        appVersion = "0.0.1 (Development)";
    }
    m_appVersionLabel->setText(appVersion);
    
    // Font Awesome Families
    QStringList fontFamilies;
    if (!IconBootstrap::sharpSolidFamily().isEmpty()) fontFamilies << IconBootstrap::sharpSolidFamily();
    if (!IconBootstrap::sharpRegularFamily().isEmpty()) fontFamilies << IconBootstrap::sharpRegularFamily();
    if (!IconBootstrap::duotoneFamily().isEmpty()) fontFamilies << IconBootstrap::duotoneFamily();
    if (!IconBootstrap::brandsFamily().isEmpty()) fontFamilies << IconBootstrap::brandsFamily();
    
    if (fontFamilies.isEmpty()) {
        m_fontAwesomeTextEdit->setPlainText(tr("Font Awesome families not loaded"));
    } else {
        QString fontInfo = tr("Loaded Font Awesome families:\n");
        for (const QString& family : fontFamilies) {
            fontInfo += QString("• %1\n").arg(family);
        }
        m_fontAwesomeTextEdit->setPlainText(fontInfo);
    }
}
