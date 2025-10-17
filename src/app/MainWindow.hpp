#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QActionGroup>
#include <QTimer>
#include <QElapsedTimer>
#include <QSettings>
#include <QLocale>
#include <memory>

QT_BEGIN_NAMESPACE
class QDockWidget;
class QLabel;
QT_END_NAMESPACE

class EnvironmentPrefs;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // File menu actions
    void newFile();
    void openFile();
    void saveFile();
    void saveAsFile();
    void exitApplication();
    
    // Editors menu actions
    void showEditors();
    void hideEditors();
    
    // Analysis menu actions
    void newAnalysis();
    void showAnalysisManager();
    
    // Tools menu actions
    void showPreferences();
    void showTelemetry();
    
    // Help menu actions
    void showAbout();
    void showDocumentation();
    
    // Ribbon actions
    void onRibbonAction();
    
    // Telemetry
    void logUIAction(const QString& action, qint64 latency);

private:
    void setupMenuBar();
    void setupRibbons();
    void setupStatusBar();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void setupTelemetry();
    
    // Menu creation helpers
    QMenu* createFileMenu();
    QMenu* createEditorsMenu();
    QMenu* createAnalysisMenu();
    QMenu* createToolsMenu();
    QMenu* createHelpMenu();
    
    // Ribbon creation helpers
    QToolBar* createTopRibbon();
    QToolBar* createRightRibbon();
    
    // Status bar helpers
    void updateStatusMessage(const QString& message);
    void updateStatusSlots();
    
    // UI Components
    QMenuBar* menuBar_;
    QToolBar* topRibbon_;
    QToolBar* rightRibbon_;
    QStatusBar* statusBar_;
    
    // Status bar slots
    QLabel* statusSlot1_;
    QLabel* statusSlot2_;
    QLabel* statusSlot3_;
    QLabel* statusSlot4_;
    QLabel* statusSlot5_;
    
    // Actions
    QAction* newAction_;
    QAction* openAction_;
    QAction* saveAction_;
    QAction* saveAsAction_;
    QAction* exitAction_;
    QAction* preferencesAction_;
    QAction* aboutAction_;
    
    // Preferences
    std::unique_ptr<EnvironmentPrefs> prefsDialog_;
    
    // Settings
    QSettings* settings_;
    QLocale currentLocale_;
    
    // Telemetry
    QTimer telemetryTimer_;
    QString telemetryPath_;
    
    // Performance tracking
    QElapsedTimer actionTimer_;
};
