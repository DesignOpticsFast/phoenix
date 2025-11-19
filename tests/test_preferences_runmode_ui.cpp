#include <QtTest/QtTest>
#include "ui/dialogs/AnalysisPage.h"
#include "app/AnalysisRunMode.hpp"
#include <QSettings>
#include <QTemporaryFile>
#include <QRadioButton>
#include <QGroupBox>

class PreferencesRunModeUITests : public QObject {
    Q_OBJECT

private slots:
    void testRunModeLoadsFromSettings();
    void testRunModeSavesToSettings();
    void testRunModeDefaultIsAuto();
};

void PreferencesRunModeUITests::testRunModeLoadsFromSettings()
{
    // Create test settings with ShowOptionsFirst mode
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString settingsPath = tempFile.fileName();
    tempFile.close();
    
    QSettings testSettings(settingsPath, QSettings::IniFormat);
    setAnalysisRunMode(testSettings, AnalysisRunMode::ShowOptionsFirst);
    testSettings.sync();
    
    // Create AnalysisPage with test settings
    AnalysisPage page(testSettings);
    page.loadSettings();
    
    // Verify "Show options first" radio is checked
    // We need to access the radio buttons - they're private, so we'll use findChild
    QGroupBox* runBehaviorGroup = page.findChild<QGroupBox*>("", Qt::FindDirectChildrenOnly);
    QVERIFY(runBehaviorGroup != nullptr);
    
    QRadioButton* showOptionsRadio = runBehaviorGroup->findChild<QRadioButton*>();
    QVERIFY(showOptionsRadio != nullptr);
    
    // Find the "Show options first" radio button
    QList<QRadioButton*> radios = runBehaviorGroup->findChildren<QRadioButton*>();
    QCOMPARE(radios.size(), 2);
    
    // The second radio should be checked (ShowOptionsFirst)
    QVERIFY(radios[1]->isChecked());
    QVERIFY(!radios[0]->isChecked());
}

void PreferencesRunModeUITests::testRunModeSavesToSettings()
{
    // Create test settings (no run mode set initially)
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString settingsPath = tempFile.fileName();
    tempFile.close();
    
    QSettings testSettings(settingsPath, QSettings::IniFormat);
    // Don't set run mode - should default to AutoRunOnOpen
    
    // Create AnalysisPage with test settings
    AnalysisPage page(testSettings);
    page.loadSettings();
    
    // Find radio buttons
    QGroupBox* runBehaviorGroup = page.findChild<QGroupBox*>("", Qt::FindDirectChildrenOnly);
    QVERIFY(runBehaviorGroup != nullptr);
    QList<QRadioButton*> radios = runBehaviorGroup->findChildren<QRadioButton*>();
    QCOMPARE(radios.size(), 2);
    
    // Programmatically check "Show options first" radio (second one)
    radios[1]->setChecked(true);
    radios[0]->setChecked(false);
    
    // Save settings
    page.saveSettings();
    testSettings.sync();
    
    // Verify settings were saved correctly
    AnalysisRunMode savedMode = getAnalysisRunMode(testSettings);
    QCOMPARE(savedMode, AnalysisRunMode::ShowOptionsFirst);
}

void PreferencesRunModeUITests::testRunModeDefaultIsAuto()
{
    // Create test settings with NO run mode set (should default to AutoRunOnOpen)
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString settingsPath = tempFile.fileName();
    tempFile.close();
    
    QSettings testSettings(settingsPath, QSettings::IniFormat);
    // Don't set run mode - should default to AutoRunOnOpen
    
    // Create AnalysisPage with test settings
    AnalysisPage page(testSettings);
    page.loadSettings();
    
    // Find radio buttons
    QGroupBox* runBehaviorGroup = page.findChild<QGroupBox*>("", Qt::FindDirectChildrenOnly);
    QVERIFY(runBehaviorGroup != nullptr);
    QList<QRadioButton*> radios = runBehaviorGroup->findChildren<QRadioButton*>();
    QCOMPARE(radios.size(), 2);
    
    // Verify "Auto-run" radio (first one) is checked by default
    QVERIFY(radios[0]->isChecked());
    QVERIFY(!radios[1]->isChecked());
}

QTEST_MAIN(PreferencesRunModeUITests)
#include "test_preferences_runmode_ui.moc"

