#include "FeatureDescriptor.hpp"

FeatureDescriptor::FeatureDescriptor()
{
}

FeatureDescriptor::FeatureDescriptor(const QString& id, const QString& displayName)
    : m_id(id)
    , m_displayName(displayName)
{
}

FeatureDescriptor& FeatureDescriptor::setCategory(const QString& category)
{
    m_category = category;
    return *this;
}

FeatureDescriptor& FeatureDescriptor::addParam(const ParamSpec& param)
{
    m_params.append(param);
    return *this;
}

FeatureDescriptor& FeatureDescriptor::setRequiresLicenseFeature(const QString& feature)
{
    m_requiresLicenseFeature = feature;
    return *this;
}

FeatureDescriptor& FeatureDescriptor::setRequiresTransport(bool required)
{
    m_requiresTransport = required;
    return *this;
}

FeatureDescriptor& FeatureDescriptor::setAutoRunOnOpen(bool enabled)
{
    m_autoRunOnOpen = enabled;
    return *this;
}

const ParamSpec* FeatureDescriptor::findParam(const QString& name) const
{
    for (const ParamSpec& param : m_params) {
        if (param.name() == name) {
            return &param;
        }
    }
    return nullptr;
}

bool FeatureDescriptor::isValidParams(const QMap<QString, QVariant>& params) const
{
    // Check all provided params are valid
    for (auto it = params.begin(); it != params.end(); ++it) {
        const QString& paramName = it.key();
        const QVariant& paramValue = it.value();
        
        const ParamSpec* spec = findParam(paramName);
        if (!spec) {
            // Unknown parameter - this is an error
            return false;
        }
        
        if (!spec->isValid(paramValue)) {
            return false;
        }
    }
    
    return true;
}

QStringList FeatureDescriptor::validationErrors(const QMap<QString, QVariant>& params) const
{
    QStringList errors;
    
    // Check all provided params are valid
    for (auto it = params.begin(); it != params.end(); ++it) {
        const QString& paramName = it.key();
        const QVariant& paramValue = it.value();
        
        const ParamSpec* spec = findParam(paramName);
        if (!spec) {
            errors.append(QString("Unknown parameter: '%1'").arg(paramName));
            continue;
        }
        
        QString error = spec->validationError(paramValue);
        if (!error.isEmpty()) {
            errors.append(error);
        }
    }
    
    return errors;
}

