#pragma once

#include "ParamSpec.hpp"
#include <QString>
#include <QList>
#include <QMap>

class FeatureDescriptor {
public:
    FeatureDescriptor();
    FeatureDescriptor(const QString& id, const QString& displayName);
    
    // Getters
    QString id() const { return m_id; }
    QString displayName() const { return m_displayName; }
    QString category() const { return m_category; }
    QList<ParamSpec> params() const { return m_params; }
    QString requiresLicenseFeature() const { return m_requiresLicenseFeature; }
    bool requiresTransport() const { return m_requiresTransport; }
    bool autoRunOnOpen() const { return m_autoRunOnOpen; }
    
    // Setters (fluent API)
    FeatureDescriptor& setCategory(const QString& category);
    FeatureDescriptor& addParam(const ParamSpec& param);
    FeatureDescriptor& setRequiresLicenseFeature(const QString& feature);
    FeatureDescriptor& setRequiresTransport(bool required);
    FeatureDescriptor& setAutoRunOnOpen(bool enabled);
    
    // Lookup
    const ParamSpec* findParam(const QString& name) const;
    
    // Validation
    bool isValidParams(const QMap<QString, QVariant>& params) const;
    QStringList validationErrors(const QMap<QString, QVariant>& params) const;

private:
    QString m_id;                      // e.g., "xy_sine"
    QString m_displayName;             // e.g., "XY Sine"
    QString m_category;                 // e.g., "Analysis"
    QList<ParamSpec> m_params;         // Parameter specifications
    QString m_requiresLicenseFeature;   // e.g., "feature_xy_sine" (empty if none)
    bool m_requiresTransport = true;    // Default true for Bedrock features
    bool m_autoRunOnOpen = false;       // Default false - features opt in to auto-run
};

