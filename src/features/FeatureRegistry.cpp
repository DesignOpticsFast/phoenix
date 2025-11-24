#include "FeatureRegistry.hpp"
#include <QDebug>

FeatureRegistry* FeatureRegistry::s_instance = nullptr;

FeatureRegistry& FeatureRegistry::instance()
{
    if (!s_instance) {
        s_instance = new FeatureRegistry();
    }
    return *s_instance;
}

void FeatureRegistry::registerFeature(const FeatureDescriptor& descriptor)
{
    if (m_features.contains(descriptor.id())) {
        qWarning() << "FeatureRegistry: Feature" << descriptor.id() << "already registered, overwriting";
    }
    m_features.insert(descriptor.id(), descriptor);
}

const FeatureDescriptor* FeatureRegistry::getFeature(const QString& id) const
{
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        qInfo() << "[REGISTRY] getFeature() called with id:" << id
                << "registeredFeatures:" << m_features.keys();
    }
#endif
    auto it = m_features.find(id);
    if (it == m_features.end()) {
#ifndef NDEBUG
        if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
            qWarning() << "[REGISTRY] Feature NOT FOUND:" << id;
        }
#endif
        return nullptr;
    }
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        qInfo() << "[REGISTRY] Feature FOUND:" << id
                << "displayName:" << it.value().displayName()
                << "params:" << it.value().params().size();
    }
#endif
    return &it.value();
}

QList<FeatureDescriptor> FeatureRegistry::allFeatures() const
{
    return m_features.values();
}

QList<FeatureDescriptor> FeatureRegistry::featuresByCategory(const QString& category) const
{
    QList<FeatureDescriptor> result;
    for (const FeatureDescriptor& desc : m_features.values()) {
        if (desc.category() == category) {
            result.append(desc);
        }
    }
    return result;
}

void FeatureRegistry::registerDefaultFeatures()
{
    // XY Sine feature (Phoenix-only, Phase 2B)
    FeatureDescriptor xySine("xy_sine", "XY Sine");
    xySine.setCategory("Analysis")
          .setRequiresLicenseFeature("feature_xy_sine")  // License check deferred to Phase 3+
          .setRequiresTransport(false);  // Phase 2B: local-only compute
    
    // Parameters based on experiments/analysis/XYWindow.cpp
    xySine.addParam(ParamSpec("frequency", "Frequency", ParamSpec::Type::Double)
                    .setDefaultValue(1.0)
                    .setMinValue(0.1)
                    .setMaxValue(100.0));
    
    xySine.addParam(ParamSpec("amplitude", "Amplitude", ParamSpec::Type::Double)
                    .setDefaultValue(1.0)
                    .setMinValue(0.0)
                    .setMaxValue(10.0));
    
    xySine.addParam(ParamSpec("phase", "Phase", ParamSpec::Type::Double)
                    .setDefaultValue(0.0)
                    .setMinValue(-6.28318)  // -2π
                    .setMaxValue(6.28318)); // 2π
    
    xySine.addParam(ParamSpec("samples", "Number of Samples", ParamSpec::Type::Int)
                    .setDefaultValue(1000)
                    .setMinValue(10)
                    .setMaxValue(100000));
    
    registerFeature(xySine);
}

