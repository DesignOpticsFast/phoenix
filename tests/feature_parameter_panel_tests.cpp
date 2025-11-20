#include <QtTest/QtTest>
#include "ui/widgets/FeatureParameterPanel.hpp"
#include "features/FeatureRegistry.hpp"
#include "features/FeatureDescriptor.hpp"
#include "features/ParamSpec.hpp"
#include <QApplication>
#include <QMap>
#include <QVariant>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSignalSpy>

class FeatureParameterPanelTests : public QObject {
    Q_OBJECT

private slots:
    void testPanelCreation();
    void testDefaultValues();
    void testSetParameters();
    void testGetParameters();
    void testValidation();
    void testMissingParams();
    void testParameterChangedSignal();
    void testWidgetTypes();
    void testIsValidAfterConstruction();
    void testValidationErrorUsesLabel();
};

void FeatureParameterPanelTests::testPanelCreation()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    const FeatureDescriptor* xySine = reg.getFeature("xy_sine");
    QVERIFY(xySine != nullptr);
    
    FeatureParameterPanel panel(*xySine);
    QVERIFY(panel.isValid());
    
    // Should have 4 parameters
    QMap<QString, QVariant> params = panel.parameters();
    QCOMPARE(params.size(), 4);
    QVERIFY(params.contains("frequency"));
    QVERIFY(params.contains("amplitude"));
    QVERIFY(params.contains("phase"));
    QVERIFY(params.contains("samples"));
}

void FeatureParameterPanelTests::testDefaultValues()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    const FeatureDescriptor* xySine = reg.getFeature("xy_sine");
    QVERIFY(xySine != nullptr);
    
    FeatureParameterPanel panel(*xySine);
    QMap<QString, QVariant> params = panel.parameters();
    
    // Check defaults match ParamSpec defaults
    QCOMPARE(params.value("frequency").toDouble(), 1.0);
    QCOMPARE(params.value("amplitude").toDouble(), 1.0);
    QCOMPARE(params.value("phase").toDouble(), 0.0);
    QCOMPARE(params.value("samples").toInt(), 1000);
}

void FeatureParameterPanelTests::testSetParameters()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    const FeatureDescriptor* xySine = reg.getFeature("xy_sine");
    QVERIFY(xySine != nullptr);
    
    FeatureParameterPanel panel(*xySine);
    
    // Set new values
    QMap<QString, QVariant> newParams;
    newParams.insert("frequency", 2.5);
    newParams.insert("amplitude", 3.0);
    newParams.insert("samples", 5000);
    
    panel.setParameters(newParams);
    
    // Verify values were set
    QMap<QString, QVariant> params = panel.parameters();
    QCOMPARE(params.value("frequency").toDouble(), 2.5);
    QCOMPARE(params.value("amplitude").toDouble(), 3.0);
    QCOMPARE(params.value("samples").toInt(), 5000);
    
    // Phase should still be default (wasn't in newParams)
    QCOMPARE(params.value("phase").toDouble(), 0.0);
}

void FeatureParameterPanelTests::testGetParameters()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    const FeatureDescriptor* xySine = reg.getFeature("xy_sine");
    QVERIFY(xySine != nullptr);
    
    FeatureParameterPanel panel(*xySine);
    
    // Set values
    QMap<QString, QVariant> inputParams;
    inputParams.insert("frequency", 5.0);
    inputParams.insert("amplitude", 2.0);
    inputParams.insert("phase", 1.57);  // π/2
    inputParams.insert("samples", 2000);
    
    panel.setParameters(inputParams);
    
    // Get values back
    QMap<QString, QVariant> outputParams = panel.parameters();
    
    // Round-trip should match
    QCOMPARE(outputParams.value("frequency").toDouble(), 5.0);
    QCOMPARE(outputParams.value("amplitude").toDouble(), 2.0);
    QCOMPARE(outputParams.value("phase").toDouble(), 1.57);
    QCOMPARE(outputParams.value("samples").toInt(), 2000);
}

void FeatureParameterPanelTests::testValidation()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    const FeatureDescriptor* xySine = reg.getFeature("xy_sine");
    QVERIFY(xySine != nullptr);
    
    FeatureParameterPanel panel(*xySine);
    
    // Set valid values - should be valid
    QMap<QString, QVariant> validParams;
    validParams.insert("frequency", 10.0);  // Within 0.1-100.0 range
    validParams.insert("samples", 5000);    // Within 10-100000 range
    panel.setParameters(validParams);
    
    QVERIFY(panel.isValid());
    QVERIFY(panel.validationErrors().isEmpty());
    
    // Note: We can't directly set invalid values through setParameters()
    // because it validates and uses defaults for invalid values.
    // But we can verify that the panel validates correctly by checking
    // that values outside range are clamped or rejected.
    
    // Try setting out-of-range frequency (should be clamped or use default)
    QMap<QString, QVariant> invalidParams;
    invalidParams.insert("frequency", 200.0);  // Above max (100.0)
    panel.setParameters(invalidParams);
    
    // The panel should either clamp or use default, but result should be valid
    QMap<QString, QVariant> result = panel.parameters();
    double freq = result.value("frequency").toDouble();
    QVERIFY(freq <= 100.0);  // Should be clamped to max or use default
    QVERIFY(panel.isValid());  // Resulting state should be valid
}

void FeatureParameterPanelTests::testMissingParams()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    const FeatureDescriptor* xySine = reg.getFeature("xy_sine");
    QVERIFY(xySine != nullptr);
    
    FeatureParameterPanel panel(*xySine);
    
    // Set only one parameter
    QMap<QString, QVariant> partialParams;
    partialParams.insert("frequency", 7.5);
    panel.setParameters(partialParams);
    
    // Other params should use defaults
    QMap<QString, QVariant> params = panel.parameters();
    QCOMPARE(params.value("frequency").toDouble(), 7.5);
    QCOMPARE(params.value("amplitude").toDouble(), 1.0);  // Default
    QCOMPARE(params.value("phase").toDouble(), 0.0);       // Default
    QCOMPARE(params.value("samples").toInt(), 1000);      // Default
}

void FeatureParameterPanelTests::testParameterChangedSignal()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    const FeatureDescriptor* xySine = reg.getFeature("xy_sine");
    QVERIFY(xySine != nullptr);
    
    FeatureParameterPanel panel(*xySine);
    
    QSignalSpy spy(&panel, &FeatureParameterPanel::parametersChanged);
    
    // Change a parameter value by accessing the widget directly
    // (We need to find the widget and change it)
    QMap<QString, QVariant> params = panel.parameters();
    
    // Set parameters programmatically should emit signal
    QMap<QString, QVariant> newParams;
    newParams.insert("frequency", 3.0);
    panel.setParameters(newParams);
    
    // Signal should be emitted
    QVERIFY(spy.count() > 0);
}

void FeatureParameterPanelTests::testWidgetTypes()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    const FeatureDescriptor* xySine = reg.getFeature("xy_sine");
    QVERIFY(xySine != nullptr);
    
    FeatureParameterPanel panel(*xySine);
    
    // Verify widget types are correct
    // We can't directly access m_editors, but we can verify by checking
    // that parameters() returns correct types
    
    QMap<QString, QVariant> params = panel.parameters();
    
    // frequency, amplitude, phase should be Double
    QVERIFY(params.value("frequency").canConvert<double>());
    QVERIFY(params.value("amplitude").canConvert<double>());
    QVERIFY(params.value("phase").canConvert<double>());
    
    // samples should be Int
    QVERIFY(params.value("samples").canConvert<int>());
}

void FeatureParameterPanelTests::testIsValidAfterConstruction()
{
    FeatureRegistry& reg = FeatureRegistry::instance();
    reg.registerDefaultFeatures();
    
    const FeatureDescriptor* xySine = reg.getFeature("xy_sine");
    QVERIFY(xySine != nullptr);
    
    FeatureParameterPanel panel(*xySine);
    
    // Panel should be valid immediately after construction with defaults
    QVERIFY(panel.isValid());
    QVERIFY(panel.validationErrors().isEmpty());
}

void FeatureParameterPanelTests::testValidationErrorUsesLabel()
{
    // Create a ParamSpec with label
    ParamSpec spec("frequency", "Frequency", ParamSpec::Type::Double);
    spec.setMinValue(0.1).setMaxValue(100.0);
    
    // Test with invalid value (out of range)
    QVariant invalidValue(-1.0);
    QString error = spec.validationError(invalidValue);
    
    // Error message should contain the label "Frequency", not empty string
    QVERIFY(!error.isEmpty());
    QVERIFY(error.contains("Frequency"));
    
    // Test with another invalid value (wrong type)
    QVariant wrongType("not a number");
    QString error2 = spec.validationError(wrongType);
    QVERIFY(!error2.isEmpty());
    QVERIFY(error2.contains("Frequency"));
    
    // Test with ParamSpec that has no label (should use name)
    ParamSpec specNoLabel("test_param", "", ParamSpec::Type::Int);
    QString error3 = specNoLabel.validationError(QVariant());
    QVERIFY(!error3.isEmpty());
    QVERIFY(error3.contains("test_param"));
}

QTEST_MAIN(FeatureParameterPanelTests)
#include "feature_parameter_panel_tests.moc"

