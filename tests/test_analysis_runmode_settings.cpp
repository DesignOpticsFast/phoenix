#include <QtTest/QtTest>
#include "app/AnalysisRunMode.hpp"
#include "app/SettingsKeys.h"
#include <QSettings>
#include <QCoreApplication>
#include <QTemporaryFile>

class AnalysisRunModeSettingsTests : public QObject {
    Q_OBJECT

private slots:
    void testDefaultRunModeIsAutoRun();
    void testPersistRunMode();
    void testEnumConversion();
    void testInvalidValueHandling();
};

void AnalysisRunModeSettingsTests::testDefaultRunModeIsAutoRun()
{
    // Create isolated test settings (using temporary file)
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString settingsPath = tempFile.fileName();
    tempFile.close();
    
    QSettings settings(settingsPath, QSettings::IniFormat);
    
    // Get run mode without setting it (should return default)
    AnalysisRunMode mode = getAnalysisRunMode(settings);
    
    QVERIFY(mode == AnalysisRunMode::AutoRunOnOpen);
}

void AnalysisRunModeSettingsTests::testPersistRunMode()
{
    // Create isolated test settings
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString settingsPath = tempFile.fileName();
    tempFile.close();
    
    QSettings settings(settingsPath, QSettings::IniFormat);
    
    // Set ShowOptionsFirst
    setAnalysisRunMode(settings, AnalysisRunMode::ShowOptionsFirst);
    settings.sync();  // Ensure written to disk
    
    // Create new QSettings instance to simulate reload
    QSettings settings2(settingsPath, QSettings::IniFormat);
    
    // Get run mode (should be ShowOptionsFirst)
    AnalysisRunMode mode = getAnalysisRunMode(settings2);
    
    QVERIFY(mode == AnalysisRunMode::ShowOptionsFirst);
    
    // Test round-trip: set AutoRunOnOpen and verify
    setAnalysisRunMode(settings2, AnalysisRunMode::AutoRunOnOpen);
    settings2.sync();
    
    QSettings settings3(settingsPath, QSettings::IniFormat);
    AnalysisRunMode mode2 = getAnalysisRunMode(settings3);
    
    QVERIFY(mode2 == AnalysisRunMode::AutoRunOnOpen);
}

void AnalysisRunModeSettingsTests::testEnumConversion()
{
    // Test string conversion
    QString autoStr = analysisRunModeToString(AnalysisRunMode::AutoRunOnOpen);
    QCOMPARE(autoStr, QString("auto"));
    
    QString optionsStr = analysisRunModeToString(AnalysisRunMode::ShowOptionsFirst);
    QCOMPARE(optionsStr, QString("options"));
    
    // Test string to enum conversion
    AnalysisRunMode autoMode = analysisRunModeFromString(QString("auto"));
    QVERIFY(autoMode == AnalysisRunMode::AutoRunOnOpen);
    
    AnalysisRunMode optionsMode = analysisRunModeFromString(QString("options"));
    QVERIFY(optionsMode == AnalysisRunMode::ShowOptionsFirst);
    
    // Test round-trip
    AnalysisRunMode original = AnalysisRunMode::ShowOptionsFirst;
    QString str = analysisRunModeToString(original);
    AnalysisRunMode converted = analysisRunModeFromString(str);
    QVERIFY(converted == original);
    
    original = AnalysisRunMode::AutoRunOnOpen;
    str = analysisRunModeToString(original);
    converted = analysisRunModeFromString(str);
    QVERIFY(converted == original);
}

void AnalysisRunModeSettingsTests::testInvalidValueHandling()
{
    // Create isolated test settings
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString settingsPath = tempFile.fileName();
    tempFile.close();
    
    QSettings settings(settingsPath, QSettings::IniFormat);
    
    // Set invalid value directly
    settings.setValue(PhxKeys::ANALYSIS_RUN_MODE, QString("banana"));
    settings.sync();
    
    // Get run mode (should fall back to default)
    AnalysisRunMode mode = getAnalysisRunMode(settings);
    
    QVERIFY(mode == AnalysisRunMode::AutoRunOnOpen);
    
    // Test analysisRunModeFromString with invalid input
    AnalysisRunMode invalidMode = analysisRunModeFromString(QString("invalid"), 
                                                             AnalysisRunMode::ShowOptionsFirst);
    QVERIFY(invalidMode == AnalysisRunMode::ShowOptionsFirst);  // Should use provided default
    
    // Test with empty string
    AnalysisRunMode emptyMode = analysisRunModeFromString(QString(""));
    QVERIFY(emptyMode == AnalysisRunMode::AutoRunOnOpen);  // Should use default default
}

QTEST_MAIN(AnalysisRunModeSettingsTests)
#include "test_analysis_runmode_settings.moc"

