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
#include <QTranslator>
#include <memory>

QT_BEGIN_NAMESPACE
class QDockWidget;
class QLabel;
class QWidget;
QT_END_NAMESPACE

class PreferencesDialog;
class ThemeManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    
    // Startup timing
    void setStartupTime(qint64 startTime);
    void setStartupTimeMs(qint64 ms);

signals:
    void firstShown();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent* ev) override;

private slots:
    // File menu actions
    void newFile();
    void openFile();
    void saveFile();
    void saveAsFile();
    void showPreferences();
    void exitApplication();
    
    // Editors menu actions
    void showLensInspector();
    void showSystemViewer();
    
    // Analysis menu actions
    void showXYPlot();
    void show2DPlot();
    
    // Tools menu actions
    
    // Help menu actions
    void showAbout();
    void showHelp();
    
    // View menu actions
    void setLightTheme();
    void setDarkTheme();
    void setSystemTheme();
    void setLanguage(const QString& language);
    
    // Status bar updates
    void updateStatusBar();
    void updateDebugInfo();
    
    // Theme change handler
    void onThemeChanged();
    void applyIcons();
    
    // UI initialization (deferred)
    void initializeUI();
    
    // UI retranslation
    void retranslateUi();

private:
    void setupMenuBar();
    void setupToolBar();
    void setupRibbons();
    void setupDockWidgets();
    void setupStatusBar();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void setupTranslations();
    void setupTheme();
    void restoreRibbonState();
    void updateRibbonIcons();
    
    // Menu creation helpers
    QMenu* createFileMenu();
    QMenu* createEditorsMenu();
    QMenu* createAnalysisMenu();
    QMenu* createToolsMenu();
    QMenu* createViewMenu();
    QMenu* createHelpMenu();
    
    // Toolbar creation helpers
    QToolBar* createMainToolBar();
    QToolBar* createTopRibbon();
    QToolBar* createRightRibbon();
    
    // Status bar helpers
    void updateStatusMessage(const QString& message);
    
    // Icon selection for actions
    QIcon getIcon(const QString& name, const QString& fallback = "question");
    
    // UI Components
    QMenuBar* m_menuBar;
    QToolBar* m_mainToolBar;
    QToolBar* m_topRibbon;
    QToolBar* m_rightRibbon;
    QStatusBar* m_statusBar;
    
    // Dock widgets
    QDockWidget* m_toolboxDock;
    QDockWidget* m_propertiesDock;
    
    // Status bar widgets
    QLabel* m_statusLabel;
    QLabel* m_debugLabel;
    
    // Actions
    QAction* m_newAction;
    QAction* m_openAction;
    QAction* m_saveAction;
    QAction* m_saveAsAction;
    QAction* m_preferencesAction;
    QAction* m_exitAction;
    
    QAction* m_lensInspectorAction;
    QAction* m_systemViewerAction;
    
    QAction* m_xyPlotAction;
    QAction* m_2dPlotAction;
    
    QAction* m_lightThemeAction;
    QAction* m_darkThemeAction;
    QAction* m_systemThemeAction;
    QActionGroup* m_themeGroup;
    
    // Dialogs
    std::unique_ptr<PreferencesDialog> m_preferencesDialog;
    
    // Settings and state
    QSettings* m_settings;
    QLocale m_currentLocale;
    QTranslator* m_translator;
    ThemeManager* m_themeManager;
    
    // Debug info timer
    QTimer* m_debugTimer;
    
    // Startup timing
    qint64 m_startupTime;
    qint64 m_startupMs = -1;
    bool m_firstShowEmitted = false;
    
    // Performance tracking
    QElapsedTimer m_actionTimer;
    
    // Telemetry hooks
    void logUIAction(const QString& action, qint64 elapsed);
    void logRibbonAction(const QString& action);
};
