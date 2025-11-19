#pragma once

#include "license/License.hpp"
#include "license/LicenseVerifier.hpp"
#include <QObject>
#include <QString>
#include <optional>

class LicenseManager : public QObject
{
    Q_OBJECT

public:
    enum class LicenseState {
        Valid,           // License loaded and valid
        Expired,         // License expired
        Invalid,         // License file exists but verification failed
        NoLicense,       // No license file found (but verifier configured)
        NotConfigured    // PHOENIX_LICENSE_PUBLIC_KEY not set
    };
    Q_ENUM(LicenseState)

    static LicenseManager* instance();
    
    // Initialize license manager (call on app startup)
    void initialize();
    
    // Get current license state
    LicenseState currentState() const { return m_currentState; }
    
    // Get current license if valid
    std::optional<License> currentLicense() const;
    
    // Check if license has a specific feature
    bool hasFeature(const QString& feature) const;
    
    // Refresh license from file (useful for dialog refresh)
    void refresh();

signals:
    void licenseStateChanged(LicenseState state);

private:
    explicit LicenseManager(QObject* parent = nullptr);
    ~LicenseManager() override = default;
    
    static LicenseManager* s_instance;
    
    // Internal state
    std::optional<LicenseVerifier> m_verifier;
    std::optional<License> m_currentLicense;
    LicenseState m_currentState = LicenseState::NotConfigured;
    QString m_licensePath;
    
    // Helper: Determine license path from env or default
    QString discoverLicensePath() const;
    
    // Helper: Load and verify license
    void loadLicense();
    
    // Helper: Update state and emit signal if changed
    void updateState(LicenseState newState);
};

