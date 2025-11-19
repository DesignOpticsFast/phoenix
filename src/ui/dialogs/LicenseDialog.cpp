#include "LicenseDialog.h"
#include "app/LicenseManager.h"
#include "license/License.hpp"
#include <QMessageBox>
#include <QPalette>

LicenseDialog::LicenseDialog(QWidget* parent)
    : QDialog(parent)
    , m_statusLabel(nullptr)
    , m_subjectLabel(nullptr)
    , m_issuedLabel(nullptr)
    , m_expiresLabel(nullptr)
    , m_featuresList(nullptr)
    , m_refreshButton(nullptr)
    , m_okButton(nullptr)
    , m_mainLayout(nullptr)
{
    setWindowTitle(tr("License Information"));
    setModal(true);
    setMinimumSize(400, 300);
    resize(500, 400);
    
    setupUI();
    updateLicenseDisplay();
}

void LicenseDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Status label (colored based on state)
    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    QFont statusFont = m_statusLabel->font();
    statusFont.setBold(true);
    statusFont.setPointSize(statusFont.pointSize() + 2);
    m_statusLabel->setFont(statusFont);
    m_mainLayout->addWidget(m_statusLabel);
    
    m_mainLayout->addSpacing(10);
    
    // Subject
    m_subjectLabel = new QLabel(this);
    m_mainLayout->addWidget(m_subjectLabel);
    
    // Issued date
    m_issuedLabel = new QLabel(this);
    m_mainLayout->addWidget(m_issuedLabel);
    
    // Expires date
    m_expiresLabel = new QLabel(this);
    m_mainLayout->addWidget(m_expiresLabel);
    
    m_mainLayout->addSpacing(10);
    
    // Features list
    QLabel* featuresLabel = new QLabel(tr("Features:"), this);
    m_mainLayout->addWidget(featuresLabel);
    
    m_featuresList = new QListWidget(this);
    m_featuresList->setMaximumHeight(150);
    m_mainLayout->addWidget(m_featuresList);
    
    m_mainLayout->addStretch();
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_refreshButton = new QPushButton(tr("Refresh"), this);
    connect(m_refreshButton, &QPushButton::clicked, this, &LicenseDialog::onRefreshClicked);
    buttonLayout->addWidget(m_refreshButton);
    
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton(tr("OK"), this);
    m_okButton->setDefault(true);
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(m_okButton);
    
    m_mainLayout->addLayout(buttonLayout);
}

void LicenseDialog::updateLicenseDisplay()
{
    LicenseManager* mgr = LicenseManager::instance();
    LicenseManager::LicenseState state = mgr->currentState();
    
    // Update status
    QString statusText = QString(tr("Status: %1")).arg(stateToString());
    m_statusLabel->setText(statusText);
    
    // Set status color
    QPalette palette = m_statusLabel->palette();
    palette.setColor(QPalette::WindowText, stateColor());
    m_statusLabel->setPalette(palette);
    
    // Update license details
    auto license = mgr->currentLicense();
    
    if (state == LicenseManager::LicenseState::Valid || 
        state == LicenseManager::LicenseState::Expired) {
        if (license.has_value()) {
            m_subjectLabel->setText(QString(tr("Subject: %1")).arg(license->subject()));
            m_subjectLabel->setVisible(true);
            
            m_issuedLabel->setText(QString(tr("Issued: %1 UTC"))
                .arg(license->issuedAt().toUTC().toString(Qt::ISODate)));
            m_issuedLabel->setVisible(true);
            
            if (license->expiresAt().has_value()) {
                m_expiresLabel->setText(QString(tr("Expires: %1 UTC"))
                    .arg(license->expiresAt()->toUTC().toString(Qt::ISODate)));
            } else {
                m_expiresLabel->setText(tr("Expires: No expiry"));
            }
            m_expiresLabel->setVisible(true);
            
            // Update features list
            m_featuresList->clear();
            for (const QString& feature : license->features()) {
                m_featuresList->addItem(feature);
            }
            m_featuresList->setVisible(true);
        } else {
            // Shouldn't happen, but handle gracefully
            m_subjectLabel->setVisible(false);
            m_issuedLabel->setVisible(false);
            m_expiresLabel->setVisible(false);
            m_featuresList->setVisible(false);
        }
    } else {
        // No license to display
        m_subjectLabel->setVisible(false);
        m_issuedLabel->setVisible(false);
        m_expiresLabel->setVisible(false);
        m_featuresList->setVisible(false);
    }
}

QString LicenseDialog::stateToString() const
{
    LicenseManager* mgr = LicenseManager::instance();
    LicenseManager::LicenseState state = mgr->currentState();
    
    switch (state) {
        case LicenseManager::LicenseState::Valid:
            return tr("Valid");
        case LicenseManager::LicenseState::Expired:
            return tr("Expired");
        case LicenseManager::LicenseState::Invalid:
            return tr("Invalid");
        case LicenseManager::LicenseState::NoLicense:
            return tr("No License File");
        case LicenseManager::LicenseState::NotConfigured:
            return tr("Not Configured");
    }
    return tr("Unknown");
}

QColor LicenseDialog::stateColor() const
{
    LicenseManager* mgr = LicenseManager::instance();
    LicenseManager::LicenseState state = mgr->currentState();
    
    switch (state) {
        case LicenseManager::LicenseState::Valid:
            return QColor(0, 150, 0); // Green
        case LicenseManager::LicenseState::Expired:
            return QColor(200, 100, 0); // Orange
        case LicenseManager::LicenseState::Invalid:
            return QColor(200, 0, 0); // Red
        case LicenseManager::LicenseState::NoLicense:
            return QColor(100, 100, 100); // Gray
        case LicenseManager::LicenseState::NotConfigured:
            return QColor(100, 100, 100); // Gray
    }
    return QColor(0, 0, 0); // Black
}

void LicenseDialog::onRefreshClicked()
{
    LicenseManager* mgr = LicenseManager::instance();
    mgr->refresh();
    updateLicenseDisplay();
}

