#include <QtTest/QtTest>
#include "features/FeatureRegistry.hpp"
#include "features/FeatureDescriptor.hpp"
#include "features/ParamSpec.hpp"
#include <QMap>
#include <QVariant>

class FeatureRegistryTests : public QObject {
    Q_OBJECT

private slots:
    void testRegistrySingleton();
    void testRegisterAndRetrieve();
    void testXYSineRegistration();
    void testXYSineParams();
    void testInvalidLookup();
    void testAllFeatures();
    void testFeaturesByCategory();
    void testParamValidation();
    void testParamStringConversion();
};

void FeatureRegistryTests::testRegistrySingleton()
{
    FeatureRegistry& reg1 = FeatureRegistry::instance();
    FeatureRegistry& reg2 = FeatureRegistry::instance();
    
    // Should return same instance
    QVERIFY(&reg1 == &reg2);
}

void FeatureRegistryTests::testRegisterAndRetrieve()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    
    // Register a test feature
    FeatureDescriptor testFeature("test_feature", "Test Feature");
    testFeature.setCategory("Test");
    reg.registerFeature(testFeature);
    
    // Retrieve it
    const FeatureDescriptor* retrieved = reg.getFeature("test_feature");
    QVERIFY(retrieved != nullptr);
    QCOMPARE(retrieved->id(), QString("test_feature"));
    QCOMPARE(retrieved->displayName(), QString("Test Feature"));
    QCOMPARE(retrieved->category(), QString("Test"));
}

void FeatureRegistryTests::testXYSineRegistration()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    const FeatureDescriptor* xySine = reg.getFeature("xy_sine");
    QVERIFY(xySine != nullptr);
    QCOMPARE(xySine->id(), QString("xy_sine"));
    QCOMPARE(xySine->displayName(), QString("XY Sine"));
    QCOMPARE(xySine->category(), QString("Analysis"));
    QCOMPARE(xySine->requiresLicenseFeature(), QString("feature_xy_sine"));
    QVERIFY(xySine->requiresTransport());
}

void FeatureRegistryTests::testXYSineParams()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    const FeatureDescriptor* xySine = reg.getFeature("xy_sine");
    QVERIFY(xySine != nullptr);
    
    QList<ParamSpec> params = xySine->params();
    QCOMPARE(params.size(), 4);
    
    // Check frequency param
    const ParamSpec* freqParam = xySine->findParam("frequency");
    QVERIFY(freqParam != nullptr);
    QCOMPARE(freqParam->name(), QString("frequency"));
    QCOMPARE(freqParam->label(), QString("Frequency"));
    QCOMPARE(freqParam->type(), ParamSpec::Type::Double);
    QCOMPARE(freqParam->defaultValue().toDouble(), 1.0);
    QCOMPARE(freqParam->minValue().toDouble(), 0.1);
    QCOMPARE(freqParam->maxValue().toDouble(), 100.0);
    
    // Check amplitude param
    const ParamSpec* ampParam = xySine->findParam("amplitude");
    QVERIFY(ampParam != nullptr);
    QCOMPARE(ampParam->name(), QString("amplitude"));
    QCOMPARE(ampParam->type(), ParamSpec::Type::Double);
    QCOMPARE(ampParam->defaultValue().toDouble(), 1.0);
    
    // Check phase param
    const ParamSpec* phaseParam = xySine->findParam("phase");
    QVERIFY(phaseParam != nullptr);
    QCOMPARE(phaseParam->name(), QString("phase"));
    QCOMPARE(phaseParam->type(), ParamSpec::Type::Double);
    QCOMPARE(phaseParam->defaultValue().toDouble(), 0.0);
    
    // Check samples param
    const ParamSpec* samplesParam = xySine->findParam("samples");
    QVERIFY(samplesParam != nullptr);
    QCOMPARE(samplesParam->name(), QString("samples"));
    QCOMPARE(samplesParam->type(), ParamSpec::Type::Int);
    QCOMPARE(samplesParam->defaultValue().toInt(), 1000);
    QCOMPARE(samplesParam->minValue().toInt(), 10);
    QCOMPARE(samplesParam->maxValue().toInt(), 100000);
}

void FeatureRegistryTests::testInvalidLookup()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    
    const FeatureDescriptor* notFound = reg.getFeature("no_such_feature");
    QVERIFY(notFound == nullptr);
}

void FeatureRegistryTests::testAllFeatures()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    QList<FeatureDescriptor> allFeatures = reg.allFeatures();
    QVERIFY(allFeatures.size() >= 1);
    
    // Should contain xy_sine
    bool foundXYSine = false;
    for (const FeatureDescriptor& desc : allFeatures) {
        if (desc.id() == "xy_sine") {
            foundXYSine = true;
            break;
        }
    }
    QVERIFY(foundXYSine);
}

void FeatureRegistryTests::testFeaturesByCategory()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    QList<FeatureDescriptor> analysisFeatures = reg.featuresByCategory("Analysis");
    QVERIFY(analysisFeatures.size() >= 1);
    
    bool foundXYSine = false;
    for (const FeatureDescriptor& desc : analysisFeatures) {
        if (desc.id() == "xy_sine") {
            foundXYSine = true;
            break;
        }
    }
    QVERIFY(foundXYSine);
    
    // Empty category should return empty list
    QList<FeatureDescriptor> emptyFeatures = reg.featuresByCategory("NonExistent");
    QVERIFY(emptyFeatures.isEmpty());
}

void FeatureRegistryTests::testParamValidation()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    const FeatureDescriptor* xySine = reg.getFeature("xy_sine");
    QVERIFY(xySine != nullptr);
    
    const ParamSpec* freqParam = xySine->findParam("frequency");
    QVERIFY(freqParam != nullptr);
    
    // Valid values
    QVERIFY(freqParam->isValid(1.0));
    QVERIFY(freqParam->isValid(50.0));
    QVERIFY(freqParam->isValid(0.1));  // Min
    QVERIFY(freqParam->isValid(100.0)); // Max
    
    // Invalid values
    QVERIFY(!freqParam->isValid(0.05));  // Below min
    QVERIFY(!freqParam->isValid(150.0)); // Above max
    QVERIFY(!freqParam->isValid("not_a_number")); // Wrong type
    
    // Test samples param (Int type)
    const ParamSpec* samplesParam = xySine->findParam("samples");
    QVERIFY(samplesParam != nullptr);
    
    QVERIFY(samplesParam->isValid(1000));
    QVERIFY(samplesParam->isValid(10));    // Min
    QVERIFY(samplesParam->isValid(100000)); // Max
    QVERIFY(!samplesParam->isValid(5));     // Below min
    QVERIFY(!samplesParam->isValid(200000)); // Above max
    QVERIFY(!samplesParam->isValid(3.14));  // Not an int
}

void FeatureRegistryTests::testParamStringConversion()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    const FeatureDescriptor* xySine = reg.getFeature("xy_sine");
    QVERIFY(xySine != nullptr);
    
    const ParamSpec* freqParam = xySine->findParam("frequency");
    QVERIFY(freqParam != nullptr);
    
    // Double conversion
    QVariant value(2.5);
    QString str = freqParam->valueToString(value);
    QCOMPARE(str, QString("2.5"));
    
    QVariant converted = freqParam->stringToValue(str);
    QVERIFY(converted.isValid());
    QCOMPARE(converted.toDouble(), 2.5);
    
    // Int conversion
    const ParamSpec* samplesParam = xySine->findParam("samples");
    QVERIFY(samplesParam != nullptr);
    
    QVariant intValue(5000);
    QString intStr = samplesParam->valueToString(intValue);
    QCOMPARE(intStr, QString("5000"));
    
    QVariant intConverted = samplesParam->stringToValue(intStr);
    QVERIFY(intConverted.isValid());
    QCOMPARE(intConverted.toInt(), 5000);
}

QTEST_MAIN(FeatureRegistryTests)
#include "feature_registry_tests.moc"

