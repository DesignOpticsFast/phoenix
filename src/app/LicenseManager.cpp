#include "LicenseManager.h"
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

namespace {
    constexpr const char* LICENSE_ENV_PATH = "PHOENIX_LICENSE_PATH";
}

LicenseManager* LicenseManager::s_instance = nullptr;

LicenseManager* LicenseManager::instance()
{
    if (!s_instance) {
        s_instance = new LicenseManager();
    }
    return s_instance;
}

LicenseManager::LicenseManager(QObject* parent)
    : QObject(parent)
    , m_currentState(LicenseState::NotConfigured)
{
}

void LicenseManager::initialize()
{
    // Try to create verifier from environment
    auto verifier = LicenseVerifier::fromEnv();
    if (!verifier.has_value()) {
        qInfo() << "[LicenseManager] PHOENIX_LICENSE_PUBLIC_KEY not set - licensing not configured";
        updateState(LicenseState::NotConfigured);
        return;
    }
    
    m_verifier = std::move(verifier.value());
    m_licensePath = discoverLicensePath();
    
    qInfo() << "[LicenseManager] Initializing with license path:" << m_licensePath;
    
    // Try to load license
    loadLicense();
}

QString LicenseManager::discoverLicensePath() const
{
    // Check environment variable first
    QByteArray envPath = qgetenv(LICENSE_ENV_PATH);
    if (!envPath.isEmpty()) {
        return QString::fromUtf8(envPath);
    }
    
    // Default to ConfigLocation + "/Phoenix/license.json"
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.filePath("Phoenix/license.json");
}

void LicenseManager::loadLicense()
{
    if (!m_verifier.has_value()) {
        updateState(LicenseState::NotConfigured);
        return;
    }
    
    QFileInfo fileInfo(m_licensePath);
    if (!fileInfo.exists()) {
        qInfo() << "[LicenseManager] License file not found:" << m_licensePath;
        updateState(LicenseState::NoLicense);
        m_currentLicense = std::nullopt;
        return;
    }
    
    // Try to load and verify
    auto license = m_verifier->loadAndVerify(m_licensePath);
    if (!license.has_value()) {
        qWarning() << "[LicenseManager] License verification failed:" << m_licensePath;
        updateState(LicenseState::Invalid);
        m_currentLicense = std::nullopt;
        return;
    }
    
    // Check if expired
    if (license->expired()) {
        qWarning() << "[LicenseManager] License expired";
        updateState(LicenseState::Expired);
        m_currentLicense = std::move(license);
        return;
    }
    
    // Valid license
    qInfo() << "[LicenseManager] License loaded successfully - Subject:" << license->subject()
            << "Features:" << license->features();
    m_currentLicense = std::move(license);
    updateState(LicenseState::Valid);
}

void LicenseManager::refresh()
{
    loadLicense();
}

std::optional<License> LicenseManager::currentLicense() const
{
    if (m_currentState == LicenseState::Valid || m_currentState == LicenseState::Expired) {
        return m_currentLicense;
    }
    return std::nullopt;
}

bool LicenseManager::hasFeature(const QString& feature) const
{
    if (m_currentState != LicenseState::Valid) {
        return false;
    }
    
    if (!m_currentLicense.has_value()) {
        return false;
    }
    
    return m_currentLicense->hasFeature(feature);
}

void LicenseManager::updateState(LicenseState newState)
{
    if (m_currentState != newState) {
        m_currentState = newState;
        emit licenseStateChanged(newState);
    }
}

