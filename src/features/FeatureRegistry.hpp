#pragma once

#include "FeatureDescriptor.hpp"
#include <QString>
#include <QList>
#include <QMap>

class FeatureRegistry {
public:
    static FeatureRegistry& instance();
    
    // Registration (call during app initialization)
    void registerFeature(const FeatureDescriptor& descriptor);
    
    // Lookup
    const FeatureDescriptor* getFeature(const QString& id) const;
    
    // Enumeration
    QList<FeatureDescriptor> allFeatures() const;
    QList<FeatureDescriptor> featuresByCategory(const QString& category) const;
    
    // Initialization (registers default features)
    void registerDefaultFeatures();

private:
    explicit FeatureRegistry() = default;
    ~FeatureRegistry() = default;
    FeatureRegistry(const FeatureRegistry&) = delete;
    FeatureRegistry& operator=(const FeatureRegistry&) = delete;
    
    static FeatureRegistry* s_instance;
    QMap<QString, FeatureDescriptor> m_features;  // id -> descriptor
};

