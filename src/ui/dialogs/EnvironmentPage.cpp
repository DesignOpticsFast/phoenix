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
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>
#include <algorithm>

EnvironmentPage::EnvironmentPage(QSettings& s, QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_systemInfoGroup(nullptr)
    , m_systemInfoLayout(nullptr)
    , m_fontAwesomeGroup(nullptr)
    , m_fontAwesomeLayout(nullptr)
    , m_resetButton(nullptr)
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
    
    // Separator and reset controls
    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    m_mainLayout->addWidget(separator);

    QHBoxLayout* resetLayout = new QHBoxLayout();
    resetLayout->setContentsMargins(0, 0, 0, 0);
    resetLayout->addStretch();

    m_resetButton = new QPushButton(tr("Reset to Factory Defaults…"), this);
    connect(m_resetButton, &QPushButton::clicked, this, &EnvironmentPage::onResetToDefaults);
    resetLayout->addWidget(m_resetButton);

    m_mainLayout->addLayout(resetLayout);
    
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
    
    // Font Awesome Families - enhanced diagnostics
    const auto& statuses = IconBootstrap::fontStatuses();
    int loadedCount = std::count_if(statuses.begin(), statuses.end(),
                                     [](const IconBootstrap::FontLoadStatus& s) { return s.ok(); });
    int totalCount = statuses.size();
    
    QString fontInfo = tr("Loaded %1/%2 Font Awesome fonts.\n\n").arg(loadedCount).arg(totalCount);
    
    if (IconBootstrap::faAvailable()) {
        // All loaded - show families
        QStringList allFamilies;
        for (const auto& st : statuses) {
            for (const QString& family : st.families) {
                if (!allFamilies.contains(family)) {
                    allFamilies << family;
                }
            }
        }
        if (!allFamilies.isEmpty()) {
            fontInfo += tr("Font families:\n");
            for (const QString& family : allFamilies) {
                fontInfo += QString("• %1\n").arg(family);
            }
        }
    } else {
        // Not all loaded - show detailed status
        fontInfo += tr("Expected fonts:\n");
        QStringList expected = IconBootstrap::expectedFontPaths();
        for (const QString& path : expected) {
            fontInfo += QString("• %1\n").arg(path);
        }
        
        fontInfo += tr("\nStatus:\n");
        for (const auto& st : statuses) {
            if (!st.exists) {
                fontInfo += QString("[MISSING] %1\n").arg(st.path);
            } else if (!st.ok()) {
                fontInfo += QString("[FAILED]  %1 (bytes=%2)\n").arg(st.path).arg(st.bytes);
            } else {
                QString familyList = st.families.join(", ");
                fontInfo += QString("[OK]      %1 (families: %2)\n").arg(st.path).arg(familyList);
            }
        }
        fontInfo += tr("\nSee log for details.");
    }
    
    m_fontAwesomeTextEdit->setPlainText(fontInfo);
}

void EnvironmentPage::onResetToDefaults()
{
    const auto response = QMessageBox::warning(
        this,
        tr("Reset Settings"),
        tr("This will clear all saved preferences and restore defaults.\n\nContinue?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (response != QMessageBox::Yes) {
        return;
    }

    m_settings.clear();
    m_settings.sync();

    QMessageBox::information(
        this,
        tr("Defaults Restored"),
        tr("Settings were reset. Please restart Phoenix to apply all changes."));

    updateSystemInfo();
}
