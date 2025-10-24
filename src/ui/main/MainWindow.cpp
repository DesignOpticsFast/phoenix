#include "MainWindow.h"
#include "../dialogs/PreferencesDialog.h"
#include "../themes/ThemeManager.h"
#include "../icons/IconProvider.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>
#include <QSettings>
#include <QLocale>
#include <QTranslator>
#include <QProcess>
#include <QThread>
#include <QDebug>
#include <QShowEvent>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_menuBar(nullptr)
    , m_mainToolBar(nullptr)
    , m_statusBar(nullptr)
    , m_toolboxDock(nullptr)
    , m_propertiesDock(nullptr)
    , m_statusLabel(nullptr)
    , m_debugLabel(nullptr)
    , m_newAction(nullptr)
    , m_openAction(nullptr)
    , m_saveAction(nullptr)
    , m_saveAsAction(nullptr)
    , m_preferencesAction(nullptr)
    , m_exitAction(nullptr)
    , m_lensInspectorAction(nullptr)
    , m_systemViewerAction(nullptr)
    , m_xyPlotAction(nullptr)
    , m_2dPlotAction(nullptr)
    , m_lightThemeAction(nullptr)
    , m_darkThemeAction(nullptr)
    , m_systemThemeAction(nullptr)
    , m_themeGroup(nullptr)
    , m_settings(new QSettings("Phoenix", "Phoenix", this))
    , m_translator(new QTranslator(this))
    , m_themeManager(nullptr)  // Defer initialization to avoid circular dependency
    , m_debugTimer(new QTimer(this))
    , m_startupTime(0)
{
    setWindowTitle("Phoenix - Optical Design Studio");
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    // Set application icon
    setWindowIcon(QIcon(":/phoenix-icon.svg"));
    
    // Keep menu bar in MainWindow (not at top of screen)
    // Note: setNativeMenuBar(false) is the default, but being explicit
    menuBar()->setNativeMenuBar(false);
    
    // Initialize ThemeManager after QApplication is ready
    m_themeManager = ThemeManager::instance();
    
    // Initialize components (defer translations until after UI is ready)
    setupMenuBar();  // This creates all the actions
    setupToolBar();  // This uses the actions created above
    setupRibbons();  // This creates dockable ribbons
    setupDockWidgets();
    setupStatusBar();
    setupConnections();
    setupTheme();
    loadSettings();
    
    // Ensure ribbons are visible after state restoration
    restoreRibbonState();
    
    // Setup translations after UI is ready
    setupTranslations();
    
    // Start debug info updates
    m_debugTimer->setInterval(1000); // Update every second
    connect(m_debugTimer, &QTimer::timeout, this, &MainWindow::updateDebugInfo);
    m_debugTimer->start();
    
    // Initial status update
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::setupMenuBar()
{
    m_menuBar = menuBar();
    
    // Increase menu font size
    QFont menuFont = m_menuBar->font();
    menuFont.setPointSize(menuFont.pointSize() + 1);
    m_menuBar->setFont(menuFont);
    
    // File menu
    QMenu* fileMenu = createFileMenu();
    m_menuBar->addMenu(fileMenu);
    
    // Editors menu
    QMenu* editorsMenu = createEditorsMenu();
    m_menuBar->addMenu(editorsMenu);
    
    // Analysis menu
    QMenu* analysisMenu = createAnalysisMenu();
    m_menuBar->addMenu(analysisMenu);
    
    // Tools menu
    QMenu* toolsMenu = createToolsMenu();
    m_menuBar->addMenu(toolsMenu);
    
    // View menu
    QMenu* viewMenu = createViewMenu();
    m_menuBar->addMenu(viewMenu);
    
    // Help menu
    QMenu* helpMenu = createHelpMenu();
    m_menuBar->addMenu(helpMenu);
}

QMenu* MainWindow::createFileMenu()
{
    QMenu* fileMenu = new QMenu("&File", this);
    
    m_newAction = new QAction(getIcon("file-plus", "file-plus"), tr("&New"), this);
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip(tr("Create a new file"));
    connect(m_newAction, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(m_newAction);
    
    m_openAction = new QAction(getIcon("folder", "folder-open"), tr("&Open"), this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip(tr("Open an existing file"));
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(m_openAction);
    
    fileMenu->addSeparator();
    
    m_saveAction = new QAction(getIcon("floppy-disk", "save"), tr("&Save"), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip(tr("Save the current file"));
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    fileMenu->addAction(m_saveAction);
    
    m_saveAsAction = new QAction(getIcon("floppy-disk-pen", "save-as"), tr("Save &As"), this);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip(tr("Save the current file with a new name"));
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveAsFile);
    fileMenu->addAction(m_saveAsAction);
    
    fileMenu->addSeparator();
    
    m_preferencesAction = new QAction(getIcon("sliders", "settings"), tr("&Preferences..."), this);
    m_preferencesAction->setShortcut(QKeySequence::Preferences);
    m_preferencesAction->setStatusTip(tr("Open preferences dialog"));
    connect(m_preferencesAction, &QAction::triggered, this, &MainWindow::showPreferences);
    fileMenu->addAction(m_preferencesAction);
    
    fileMenu->addSeparator();
    
    m_exitAction = new QAction(getIcon("power-off", "close"), tr("E&xit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the application"));
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    fileMenu->addAction(m_exitAction);
    
    return fileMenu;
}

QMenu* MainWindow::createEditorsMenu()
{
    QMenu* editorsMenu = new QMenu(tr("&Editors"), this);
    
    m_lensInspectorAction = new QAction(getIcon("lens", "search"), tr("&Lens Inspector"), this);
    m_lensInspectorAction->setStatusTip(tr("Open lens inspector"));
    connect(m_lensInspectorAction, &QAction::triggered, this, &MainWindow::showLensInspector);
    editorsMenu->addAction(m_lensInspectorAction);
    
    m_systemViewerAction = new QAction(getIcon("desktop", "view"), tr("&System Viewer"), this);
    m_systemViewerAction->setStatusTip(tr("Open system viewer"));
    connect(m_systemViewerAction, &QAction::triggered, this, &MainWindow::showSystemViewer);
    editorsMenu->addAction(m_systemViewerAction);
    
    return editorsMenu;
}

QMenu* MainWindow::createAnalysisMenu()
{
    QMenu* analysisMenu = new QMenu(tr("&Analysis"), this);
    
    m_xyPlotAction = new QAction(getIcon("chart-line", "chart"), tr("&XY Plot"), this);
    m_xyPlotAction->setStatusTip(tr("Open XY plot analysis"));
    connect(m_xyPlotAction, &QAction::triggered, this, &MainWindow::showXYPlot);
    analysisMenu->addAction(m_xyPlotAction);
    
    m_2dPlotAction = new QAction(getIcon("chart-bar", "chart"), tr("&2D Plot"), this);
    m_2dPlotAction->setStatusTip(tr("Open 2D plot analysis"));
    connect(m_2dPlotAction, &QAction::triggered, this, &MainWindow::show2DPlot);
    analysisMenu->addAction(m_2dPlotAction);
    
    return analysisMenu;
}

QMenu* MainWindow::createToolsMenu()
{
    QMenu* toolsMenu = new QMenu(tr("&Tools"), this);
    
    // Add tools actions here in the future
    QAction* settingsAction = new QAction(tr("&Settings"), this);
    settingsAction->setStatusTip(tr("Open application settings"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showPreferences);
    toolsMenu->addAction(settingsAction);
    
    return toolsMenu;
}

QMenu* MainWindow::createViewMenu()
{
    QMenu* viewMenu = new QMenu(tr("&View"), this);
    
    // Theme submenu
    QMenu* themeMenu = new QMenu(tr("&Theme"), this);
    m_themeGroup = new QActionGroup(this);
    
    m_lightThemeAction = new QAction(tr("&Light"), this);
    m_lightThemeAction->setCheckable(true);
    m_lightThemeAction->setActionGroup(m_themeGroup);
    connect(m_lightThemeAction, &QAction::triggered, this, &MainWindow::setLightTheme);
    themeMenu->addAction(m_lightThemeAction);
    
    m_darkThemeAction = new QAction(tr("&Dark"), this);
    m_darkThemeAction->setCheckable(true);
    m_darkThemeAction->setActionGroup(m_themeGroup);
    connect(m_darkThemeAction, &QAction::triggered, this, &MainWindow::setDarkTheme);
    themeMenu->addAction(m_darkThemeAction);
    
    m_systemThemeAction = new QAction(tr("&System"), this);
    m_systemThemeAction->setCheckable(true);
    m_systemThemeAction->setActionGroup(m_themeGroup);
    connect(m_systemThemeAction, &QAction::triggered, this, &MainWindow::setSystemTheme);
    themeMenu->addAction(m_systemThemeAction);
    
    viewMenu->addMenu(themeMenu);
    
    // Language submenu
    QMenu* languageMenu = new QMenu(tr("&Language"), this);
    
    QAction* englishAction = new QAction(tr("&English"), this);
    connect(englishAction, &QAction::triggered, this, [this]() { setLanguage("en"); });
    languageMenu->addAction(englishAction);
    
    QAction* germanAction = new QAction(tr("&German"), this);
    connect(germanAction, &QAction::triggered, this, [this]() { setLanguage("de"); });
    languageMenu->addAction(germanAction);
    
    QAction* frenchAction = new QAction(tr("&French"), this);
    connect(frenchAction, &QAction::triggered, this, [this]() { setLanguage("fr"); });
    languageMenu->addAction(frenchAction);
    
    QAction* spanishAction = new QAction(tr("&Spanish"), this);
    connect(spanishAction, &QAction::triggered, this, [this]() { setLanguage("es"); });
    languageMenu->addAction(spanishAction);
    
    QAction* chineseTradAction = new QAction(tr("Chinese (&Traditional)"), this);
    connect(chineseTradAction, &QAction::triggered, this, [this]() { setLanguage("zh_TW"); });
    languageMenu->addAction(chineseTradAction);
    
    QAction* chineseSimpAction = new QAction(tr("Chinese (&Simplified)"), this);
    connect(chineseSimpAction, &QAction::triggered, this, [this]() { setLanguage("zh_CN"); });
    languageMenu->addAction(chineseSimpAction);
    
    QAction* koreanAction = new QAction(tr("&Korean"), this);
    connect(koreanAction, &QAction::triggered, this, [this]() { setLanguage("ko"); });
    languageMenu->addAction(koreanAction);
    
    QAction* japaneseAction = new QAction(tr("&Japanese"), this);
    connect(japaneseAction, &QAction::triggered, this, [this]() { setLanguage("ja"); });
    languageMenu->addAction(japaneseAction);
    
    viewMenu->addMenu(languageMenu);
    
    return viewMenu;
}

QMenu* MainWindow::createHelpMenu()
{
    QMenu* helpMenu = new QMenu(tr("&Help"), this);
    
    QAction* aboutAction = new QAction(getIcon("info-circle", "info"), tr("&About Phoenix"), this);
    aboutAction->setStatusTip(tr("Show about dialog"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
    helpMenu->addAction(aboutAction);
    
    helpMenu->addSeparator();
    
    QAction* helpAction = new QAction(tr("&Help Contents"), this);
    helpAction->setStatusTip(tr("Open help documentation"));
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelp);
    helpMenu->addAction(helpAction);
    
    return helpMenu;
}

void MainWindow::setupToolBar()
{
    m_mainToolBar = createMainToolBar();
    addToolBar(m_mainToolBar);
}

QToolBar* MainWindow::createMainToolBar()
{
    QToolBar* toolBar = new QToolBar(tr("Main Toolbar"), this);
    toolBar->setObjectName("mainToolBar");
    toolBar->setMovable(true);
    toolBar->setFloatable(true);
    toolBar->setVisible(true);
    toolBar->show();
    
    
    toolBar->addAction(m_newAction);
    toolBar->addAction(m_openAction);
    toolBar->addAction(m_saveAction);
    toolBar->addSeparator();
    toolBar->addAction(m_preferencesAction);
    
    // Force toolbar to be visible and have content
    toolBar->setVisible(true);
    toolBar->show();
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBar->setIconSize(QSize(24, 24));
    
    return toolBar;
}

void MainWindow::setupRibbons()
{
    // Top ribbon (horizontal)
    m_topRibbon = createTopRibbon();
    addToolBar(Qt::TopToolBarArea, m_topRibbon);
    
    // Right ribbon (vertical)
    m_rightRibbon = createRightRibbon();
    addToolBar(Qt::RightToolBarArea, m_rightRibbon);
    
    // Ensure ribbons are visible by default
    m_topRibbon->setVisible(true);
    m_rightRibbon->setVisible(true);
}

QToolBar* MainWindow::createTopRibbon()
{
    QToolBar* ribbon = new QToolBar(tr("Top Ribbon"), this);
    ribbon->setObjectName("topRibbon");
    ribbon->setMovable(true);
    ribbon->setFloatable(true);
    ribbon->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    ribbon->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ribbon->setIconSize(QSize(24, 24));
    
    // Ensure floating ribbons stay visible
    ribbon->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    
    // Set size policy for compact floating
    ribbon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    // Connect floating behavior
    connect(ribbon, &QToolBar::topLevelChanged, this, [this, ribbon](bool floating) {
        if (floating) {
            setupFloatingRibbon(ribbon);
        } else {
            // Remove from floating state when docked
            m_floatingRibbons.remove(ribbon);
            // Reset to normal state
            ribbon->setWindowFlags(Qt::Widget);
            // Restore original layout
            RibbonLayout originalLayout = (ribbon == m_rightRibbon) ? MainWindow::RibbonLayout::Vertical : MainWindow::RibbonLayout::Horizontal;
            setRibbonLayout(ribbon, originalLayout);
        }
    });
    
    // Add context menu for layout switching (only when floating)
    ribbon->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ribbon, &QToolBar::customContextMenuRequested, this, [this, ribbon](const QPoint& pos) {
        if (m_floatingRibbons.contains(ribbon)) {
            showRibbonContextMenu(ribbon, ribbon->mapToGlobal(pos));
        }
    });
    
    // File actions
    QAction* newAction = ribbon->addAction(getIcon("plus", "document-new"), tr("New"));
    newAction->setToolTip(tr("Create new file"));
    connect(newAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        newFile(); 
        logRibbonAction("new_file");
    });
    
    QAction* openAction = ribbon->addAction(getIcon("folder-open", "document-open"), tr("Open"));
    openAction->setToolTip(tr("Open existing file"));
    connect(openAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        openFile(); 
        logRibbonAction("open_file");
    });
    
    QAction* saveAction = ribbon->addAction(getIcon("floppy-disk", "document-save"), tr("Save"));
    saveAction->setToolTip(tr("Save current file"));
    connect(saveAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        saveFile(); 
        logRibbonAction("save_file");
    });
    
    ribbon->addSeparator();
    
    // Analysis actions
    QAction* xyPlotAction = ribbon->addAction(getIcon("chart-line", "chart"), tr("XY Plot"));
    xyPlotAction->setToolTip(tr("Open XY plot analysis"));
    connect(xyPlotAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        showXYPlot(); 
        logRibbonAction("xy_plot");
    });
    
    QAction* plot2DAction = ribbon->addAction(getIcon("chart-bar", "chart"), tr("2D Plot"));
    plot2DAction->setToolTip(tr("Open 2D plot analysis"));
    connect(plot2DAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        show2DPlot(); 
        logRibbonAction("2d_plot");
    });
    
    ribbon->addSeparator();
    
    // Tools actions
    QAction* preferencesAction = ribbon->addAction(getIcon("sliders", "preferences"), tr("Preferences"));
    preferencesAction->setToolTip(tr("Open preferences"));
    connect(preferencesAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        showPreferences(); 
        logRibbonAction("preferences");
    });
    
    return ribbon;
}

QToolBar* MainWindow::createRightRibbon()
{
    QToolBar* ribbon = new QToolBar(tr("Right Ribbon"), this);
    ribbon->setObjectName("rightRibbon");
    ribbon->setMovable(true);
    ribbon->setFloatable(true);
    ribbon->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);
    ribbon->setOrientation(Qt::Vertical);
    ribbon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ribbon->setIconSize(QSize(20, 20));
    
    // Ensure floating ribbons stay visible
    ribbon->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    
    // Set size policy for compact floating
    ribbon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    // Connect floating behavior
    connect(ribbon, &QToolBar::topLevelChanged, this, [this, ribbon](bool floating) {
        if (floating) {
            setupFloatingRibbon(ribbon);
        } else {
            // Remove from floating state when docked
            m_floatingRibbons.remove(ribbon);
            // Reset to normal state
            ribbon->setWindowFlags(Qt::Widget);
            // Restore original layout
            RibbonLayout originalLayout = (ribbon == m_rightRibbon) ? MainWindow::RibbonLayout::Vertical : MainWindow::RibbonLayout::Horizontal;
            setRibbonLayout(ribbon, originalLayout);
        }
    });
    
    // Add context menu for layout switching (only when floating)
    ribbon->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ribbon, &QToolBar::customContextMenuRequested, this, [this, ribbon](const QPoint& pos) {
        if (m_floatingRibbons.contains(ribbon)) {
            showRibbonContextMenu(ribbon, ribbon->mapToGlobal(pos));
        }
    });
    
    // Editors actions
    QAction* lensInspectorAction = ribbon->addAction(getIcon("lens", "search"), tr("Lens Inspector"));
    lensInspectorAction->setToolTip(tr("Open lens inspector"));
    connect(lensInspectorAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        showLensInspector(); 
        logRibbonAction("lens_inspector");
    });
    
    QAction* systemViewerAction = ribbon->addAction(getIcon("desktop", "view"), tr("System Viewer"));
    systemViewerAction->setToolTip(tr("Open system viewer"));
    connect(systemViewerAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        showSystemViewer(); 
        logRibbonAction("system_viewer");
    });
    
    ribbon->addSeparator();
    
    // View actions
    QAction* lightThemeAction = ribbon->addAction(getIcon("sun", "light"), tr("Light Theme"));
    lightThemeAction->setToolTip(tr("Switch to light theme"));
    lightThemeAction->setCheckable(true);
    connect(lightThemeAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        setLightTheme(); 
        logRibbonAction("light_theme");
    });
    
    QAction* darkThemeAction = ribbon->addAction(getIcon("moon", "dark"), tr("Dark Theme"));
    darkThemeAction->setToolTip(tr("Switch to dark theme"));
    darkThemeAction->setCheckable(true);
    connect(darkThemeAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        setDarkTheme(); 
        logRibbonAction("dark_theme");
    });
    
    ribbon->addSeparator();
    
    // Help actions
    QAction* helpAction = ribbon->addAction(getIcon("question-circle", "help"), tr("Help"));
    helpAction->setToolTip(tr("Open help"));
    connect(helpAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        showHelp(); 
        logRibbonAction("help");
    });
    
    QAction* aboutAction = ribbon->addAction(getIcon("info-circle", "info"), tr("About"));
    aboutAction->setToolTip(tr("Show about dialog"));
    connect(aboutAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        showAbout(); 
        logRibbonAction("about");
    });
    
    return ribbon;
}

void MainWindow::setupDockWidgets()
{
    // Toolbox dock (left)
    m_toolboxDock = new QDockWidget(tr("Toolbox"), this);
    m_toolboxDock->setObjectName("toolboxDock");
    m_toolboxDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_toolboxDock->setMinimumWidth(200);
    m_toolboxDock->setMaximumWidth(400);
    
    QWidget* toolboxWidget = new QWidget();
    toolboxWidget->setMinimumSize(200, 300);
    m_toolboxDock->setWidget(toolboxWidget);
    addDockWidget(Qt::LeftDockWidgetArea, m_toolboxDock);
    
    // Properties dock (right)
    m_propertiesDock = new QDockWidget(tr("Properties"), this);
    m_propertiesDock->setObjectName("propertiesDock");
    m_propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_propertiesDock->setMinimumWidth(200);
    m_propertiesDock->setMaximumWidth(400);
    
    QWidget* propertiesWidget = new QWidget();
    propertiesWidget->setMinimumSize(200, 300);
    m_propertiesDock->setWidget(propertiesWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
    
    // Central widget placeholder
    QWidget* centralWidget = new QWidget();
    centralWidget->setMinimumSize(400, 300);
    setCentralWidget(centralWidget);
}

void MainWindow::setupStatusBar()
{
    m_statusBar = statusBar();
    
    // Left side - status message
    m_statusLabel = new QLabel(tr("Ready"));
    m_statusBar->addWidget(m_statusLabel);
    
    // Right side - debug information
    m_debugLabel = new QLabel();
    m_statusBar->addPermanentWidget(m_debugLabel);
    
    updateStatusBar();
}

void MainWindow::setupConnections()
{
    // Connect theme manager signals
    connect(m_themeManager, &ThemeManager::darkModeChanged, this, &MainWindow::applyIcons);
    connect(m_themeManager, QOverload<ThemeManager::Theme>::of(&ThemeManager::themeChanged), this, &MainWindow::onThemeChanged);
}

void MainWindow::setupTranslations()
{
    // Load current language from settings
    QString language = m_settings->value("language", "en").toString();
    setLanguage(language);
}

void MainWindow::setupTheme()
{
    // Load current theme from settings
    QString theme = m_settings->value("theme", "system").toString();
    if (theme == "light") {
        m_lightThemeAction->setChecked(true);
        if (m_themeManager) m_themeManager->setTheme(ThemeManager::Theme::Light);
    } else if (theme == "dark") {
        m_darkThemeAction->setChecked(true);
        if (m_themeManager) m_themeManager->setTheme(ThemeManager::Theme::Dark);
    } else {
        m_systemThemeAction->setChecked(true);
        if (m_themeManager) m_themeManager->setTheme(ThemeManager::Theme::System);
    }
}

void MainWindow::restoreRibbonState()
{
    // Ensure ribbons are visible after state restoration
    if (m_topRibbon) {
        m_topRibbon->setVisible(true);
        // If the ribbon was hidden by state restoration, make it visible
        if (!m_topRibbon->isVisible()) {
            m_topRibbon->show();
        }
    }
    
    if (m_rightRibbon) {
        m_rightRibbon->setVisible(true);
        // If the ribbon was hidden by state restoration, make it visible
        if (!m_rightRibbon->isVisible()) {
            m_rightRibbon->show();
        }
    }
}

void MainWindow::updateRibbonIcons()
{
    // Update top ribbon icons
    if (m_topRibbon) {
        QList<QAction*> actions = m_topRibbon->actions();
        for (QAction* action : actions) {
            QString text = action->text();
            if (text == tr("New")) {
                action->setIcon(getIcon("plus", "document-new"));
            } else if (text == tr("Open")) {
                action->setIcon(getIcon("folder-open", "document-open"));
            } else if (text == tr("Save")) {
                action->setIcon(getIcon("floppy-disk", "save"));
            } else if (text == tr("XY Plot")) {
                action->setIcon(getIcon("chart-line", "chart"));
            } else if (text == tr("2D Plot")) {
                action->setIcon(getIcon("chart-bar", "chart"));
            } else if (text == tr("Preferences")) {
                action->setIcon(getIcon("sliders", "settings"));
            }
        }
    }
    
    // Update right ribbon icons
    if (m_rightRibbon) {
        QList<QAction*> actions = m_rightRibbon->actions();
        for (QAction* action : actions) {
            QString text = action->text();
            if (text == tr("Lens Inspector")) {
                action->setIcon(getIcon("lens", "search"));
            } else if (text == tr("System Viewer")) {
                action->setIcon(getIcon("desktop", "view"));
            } else if (text == tr("Light Theme")) {
                action->setIcon(getIcon("sun", "light"));
            } else if (text == tr("Dark Theme")) {
                action->setIcon(getIcon("moon", "dark"));
            } else if (text == tr("System Theme")) {
                action->setIcon(getIcon("desktop", "system"));
            } else if (text == tr("Help")) {
                action->setIcon(getIcon("question", "help"));
            } else if (text == tr("About")) {
                // Force update of Help-About icon with proper theming
                action->setIcon(getIcon("info-circle", "info"));
                // Ensure the icon is updated immediately
                action->update();
            }
        }
    }
}

void MainWindow::setupFloatingRibbon(QToolBar* ribbon)
{
    if (!ribbon) return;
    
    // Set proper window flags for always-on-top floating
    ribbon->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    ribbon->setParent(this, Qt::Window);
    ribbon->setAttribute(Qt::WA_ShowWithoutActivating, true);
    
    // Force to front and ensure visibility
    ribbon->raise();
    ribbon->activateWindow();
    
    // Determine optimal layout for floating
    RibbonLayout layout = getOptimalLayout(ribbon, true);
    setRibbonLayout(ribbon, layout);
    
    // Calculate size based on layout
    updateRibbonLayout(ribbon);
    
    // Track as floating
    m_floatingRibbons.insert(ribbon);
    
    // Install event filter for resize handling
    ribbon->installEventFilter(this);
    
    // Ensure visibility and z-order
    ribbon->show();
    ribbon->raise();
    ribbon->activateWindow();
}

void MainWindow::setRibbonLayout(QToolBar* ribbon, RibbonLayout layout)
{
    if (!ribbon) return;
    
    m_ribbonLayouts[ribbon] = layout;
    updateRibbonLayout(ribbon);
}

void MainWindow::updateRibbonLayout(QToolBar* ribbon)
{
    if (!ribbon) return;
    
    RibbonLayout layout = m_ribbonLayouts.value(ribbon, MainWindow::RibbonLayout::Horizontal);
    bool isFloating = m_floatingRibbons.contains(ribbon);
    
    // Get all actions
    QList<QAction*> actions = ribbon->actions();
    int actionCount = actions.size();
    
    if (isFloating && layout == MainWindow::RibbonLayout::Grid) {
        // Set grid layout with configurable dimensions
        ribbon->setOrientation(Qt::Horizontal);
        ribbon->setToolButtonStyle(Qt::ToolButtonIconOnly);
        
        // Calculate optimal grid dimensions based on current size
        QSize currentSize = ribbon->size();
        QSize iconSize = ribbon->iconSize();
        QSize buttonSize = iconSize + QSize(8, 8); // Add padding
        
        // Calculate how many columns fit in current width
        int cols = qMax(1, currentSize.width() / buttonSize.width());
        int rows = qCeil(double(actionCount) / cols);
        
        // Calculate size for grid
        QSize gridSize = QSize(cols * buttonSize.width(), rows * buttonSize.height());
        
        // Set size constraints for resizable grid
        ribbon->setMinimumSize(QSize(buttonSize.width(), buttonSize.height())); // Minimum 1x1
        ribbon->setMaximumSize(QSize(16777215, 16777215)); // Allow very large
        ribbon->resize(gridSize);
        
        // Enable resizing by setting size policy
        ribbon->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        
    } else if (layout == MainWindow::RibbonLayout::Horizontal) {
        // Horizontal strip layout
        ribbon->setOrientation(Qt::Horizontal);
        ribbon->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        
        // Calculate horizontal size
        QSize iconSize = ribbon->iconSize();
        QSize buttonSize = iconSize + QSize(16, 32); // Text under icon
        QSize horizontalSize = QSize(actionCount * buttonSize.width(), buttonSize.height());
        
        ribbon->setMinimumSize(horizontalSize);
        ribbon->resize(horizontalSize);
        
    } else if (layout == MainWindow::RibbonLayout::Vertical) {
        // Vertical strip layout
        ribbon->setOrientation(Qt::Vertical);
        ribbon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        
        // Calculate vertical size
        QSize iconSize = ribbon->iconSize();
        QSize buttonSize = iconSize + QSize(32, 16); // Text beside icon
        QSize verticalSize = QSize(buttonSize.width(), actionCount * buttonSize.height());
        
        ribbon->setMinimumSize(verticalSize);
        ribbon->resize(verticalSize);
    }
}

MainWindow::RibbonLayout MainWindow::getOptimalLayout(QToolBar* ribbon, bool isFloating)
{
    if (!ribbon) return MainWindow::RibbonLayout::Horizontal;
    
    QList<QAction*> actions = ribbon->actions();
    int actionCount = actions.size();
    
    if (isFloating) {
        // For floating, choose grid if many actions, otherwise horizontal
        if (actionCount > 6) {
            return MainWindow::RibbonLayout::Grid;
        } else {
            return MainWindow::RibbonLayout::Horizontal;
        }
    } else {
        // For docked, use original orientation
        if (ribbon->orientation() == Qt::Vertical) {
            return MainWindow::RibbonLayout::Vertical;
        } else {
            return MainWindow::RibbonLayout::Horizontal;
        }
    }
}

void MainWindow::cycleRibbonLayout(QToolBar* ribbon)
{
    if (!ribbon || !m_floatingRibbons.contains(ribbon)) return;
    
    RibbonLayout current = m_ribbonLayouts.value(ribbon, MainWindow::RibbonLayout::Horizontal);
    RibbonLayout next;
    
    switch (current) {
        case MainWindow::RibbonLayout::Horizontal:
            next = MainWindow::RibbonLayout::Vertical;
            break;
        case MainWindow::RibbonLayout::Vertical:
            next = MainWindow::RibbonLayout::Grid;
            break;
        case MainWindow::RibbonLayout::Grid:
            next = MainWindow::RibbonLayout::Horizontal;
            break;
    }
    
    setRibbonLayout(ribbon, next);
}

void MainWindow::showRibbonContextMenu(QToolBar* ribbon, const QPoint& globalPos)
{
    if (!ribbon || !m_floatingRibbons.contains(ribbon)) return;
    
    QMenu contextMenu(this);
    contextMenu.setTitle(tr("Ribbon Layout"));
    
    // Add layout options
    QAction* horizontalAction = contextMenu.addAction(tr("Horizontal Layout"));
    QAction* verticalAction = contextMenu.addAction(tr("Vertical Layout"));
    QAction* gridAction = contextMenu.addAction(tr("Grid Layout"));
    
    // Mark current layout
    RibbonLayout current = m_ribbonLayouts.value(ribbon, MainWindow::RibbonLayout::Horizontal);
    switch (current) {
        case MainWindow::RibbonLayout::Horizontal:
            horizontalAction->setCheckable(true);
            horizontalAction->setChecked(true);
            break;
        case MainWindow::RibbonLayout::Vertical:
            verticalAction->setCheckable(true);
            verticalAction->setChecked(true);
            break;
        case MainWindow::RibbonLayout::Grid:
            gridAction->setCheckable(true);
            gridAction->setChecked(true);
            break;
    }
    
    // Connect actions
    connect(horizontalAction, &QAction::triggered, this, [this, ribbon]() {
        setRibbonLayout(ribbon, MainWindow::RibbonLayout::Horizontal);
    });
    connect(verticalAction, &QAction::triggered, this, [this, ribbon]() {
        setRibbonLayout(ribbon, MainWindow::RibbonLayout::Vertical);
    });
    connect(gridAction, &QAction::triggered, this, [this, ribbon]() {
        setRibbonLayout(ribbon, MainWindow::RibbonLayout::Grid);
    });
    
    // Show context menu
    contextMenu.exec(globalPos);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        // Ensure floating ribbons stay on top of everything
        for (QToolBar* ribbon : m_floatingRibbons) {
            if (ribbon && ribbon->isVisible()) {
                ribbon->raise();
                ribbon->activateWindow();
                // Ensure window flags are correct for always-on-top
                ribbon->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                ribbon->show();
            }
        }
    }
    QMainWindow::changeEvent(event);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Handle ribbon resize events for grid layout
    if (QToolBar* ribbon = qobject_cast<QToolBar*>(obj)) {
        if (m_floatingRibbons.contains(ribbon) && event->type() == QEvent::Resize) {
            RibbonLayout layout = m_ribbonLayouts.value(ribbon, MainWindow::RibbonLayout::Horizontal);
            if (layout == MainWindow::RibbonLayout::Grid) {
                // Update grid layout when ribbon is resized
                updateRibbonLayout(ribbon);
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::loadSettings()
{
    // Load window geometry
    restoreGeometry(m_settings->value("geometry").toByteArray());
    restoreState(m_settings->value("windowState").toByteArray());
}

void MainWindow::saveSettings()
{
    // Save window geometry
    m_settings->setValue("geometry", saveGeometry());
    m_settings->setValue("windowState", saveState());
}

void MainWindow::updateStatusBar()
{
    updateStatusMessage(tr("Ready"));
    updateDebugInfo();
}

void MainWindow::updateStatusMessage(const QString& message)
{
    if (m_statusLabel) {
        m_statusLabel->setText(message);
    }
}

void MainWindow::setStartupTime(qint64 startTime)
{
    m_startupTime = startTime;
}

void MainWindow::setStartupTimeMs(qint64 ms)
{
    m_startupMs = ms;
    if (m_firstShowEmitted && statusBar())
        statusBar()->showMessage(QString("Startup: %1 ms").arg(m_startupMs), 5000);
}

void MainWindow::showEvent(QShowEvent* ev)
{
    QMainWindow::showEvent(ev);
    if (!m_firstShowEmitted && isVisible()) {
        m_firstShowEmitted = true;
        emit firstShown();
        if (m_startupMs >= 0 && statusBar())
            statusBar()->showMessage(QString("Startup: %1 ms").arg(m_startupMs), 5000);
    }
}

void MainWindow::updateDebugInfo()
{
    if (!m_debugLabel) return;
    
    // Get memory usage (simplified)
    QProcess process;
    process.start("ps", QStringList() << "-o" << "rss=" << "-p" << QString::number(QApplication::applicationPid()));
    process.waitForFinished();
    QString memoryStr = process.readAllStandardOutput().trimmed();
    int memoryMB = memoryStr.toInt() / 1024; // Convert KB to MB
    
    // Get thread count
    int threadCount = QThread::idealThreadCount();
    
    // Get current theme
    QString themeStr;
    if (m_themeManager) {
        switch (m_themeManager->currentTheme()) {
        case ThemeManager::Theme::Light:
            themeStr = tr("Light");
            break;
        case ThemeManager::Theme::Dark:
            themeStr = tr("Dark");
            break;
        case ThemeManager::Theme::System:
            themeStr = tr("System");
            break;
        }
    } else {
        themeStr = tr("Unknown");
    }
    
    // Get current language
    QString langStr = m_currentLocale.name().left(2);
    
    // Show startup time if available (constant, not updating)
    QString startupInfo;
    if (m_startupTime > 0) {
        // Use static variable to store calculated startup time
        static qint64 startupDuration = 0;
        static bool startupCalculated = false;
        
        if (!startupCalculated) {
            // Calculate startup duration once when first called
            qint64 mainWindowReadyTime = QDateTime::currentMSecsSinceEpoch();
            startupDuration = mainWindowReadyTime - m_startupTime;
            startupCalculated = true;
        }
        
        startupInfo = tr(" | Startup: %1ms").arg(startupDuration);
    }
    
    // Update debug label
    QString debugText = tr("Memory: %1MB | Threads: %2 | Theme: %3 | Lang: %4%5")
                       .arg(memoryMB)
                       .arg(threadCount)
                       .arg(themeStr)
                       .arg(langStr)
                       .arg(startupInfo);
    
    m_debugLabel->setText(debugText);
}

QIcon MainWindow::getIcon(const QString& name, const QString& fallback)
{
    // Use IconProvider to get Font Awesome icons
    if (m_themeManager) {
        bool isDark = m_themeManager->isDarkMode();
        // Try Font Awesome icon first
        QIcon faIcon = IconProvider::icon(name, IconStyle::SharpSolid, 16, isDark);
        if (!faIcon.isNull()) {
            return faIcon;
        }
        
        // Fallback to system theme icons
        if (name == "plus") return QIcon::fromTheme("document-new");
        if (name == "folder-open") return QIcon::fromTheme("document-open");
        if (name == "floppy-disk") return QIcon::fromTheme("document-save");
        if (name == "copy") return QIcon::fromTheme("edit-copy");
        if (name == "gear") return QIcon::fromTheme("preferences-system");
        if (name == "xmark") return QIcon::fromTheme("application-exit");
        if (name == "magnifying-glass") return QIcon::fromTheme("edit-find");
        if (name == "eye") return QIcon::fromTheme("view-refresh");
        if (name == "chart-line") return QIcon::fromTheme("office-chart-line");
        if (name == "chart-area") return QIcon::fromTheme("office-chart-area");
        if (name == "info-circle") return QIcon::fromTheme("help-about");
        return QIcon::fromTheme("application-x-executable");
    } else {
        // Fallback to default icon if theme manager not ready
        return IconProvider::icon(name, IconStyle::SharpSolid, 16, m_themeManager->isDarkMode());
    }
}

// File menu actions
void MainWindow::newFile()
{
    QMessageBox::information(this, tr("New File"), tr("This feature is not yet implemented."));
}

void MainWindow::openFile()
{
    QMessageBox::information(this, tr("Open File"), tr("This feature is not yet implemented."));
}

void MainWindow::saveFile()
{
    QMessageBox::information(this, tr("Save File"), tr("This feature is not yet implemented."));
}

void MainWindow::saveAsFile()
{
    QMessageBox::information(this, tr("Save As File"), tr("This feature is not yet implemented."));
}

void MainWindow::showPreferences()
{
    if (!m_preferencesDialog) {
        m_preferencesDialog = std::make_unique<PreferencesDialog>(this);
    }
    m_preferencesDialog->show();
    m_preferencesDialog->raise();
    m_preferencesDialog->activateWindow();
}

void MainWindow::exitApplication()
{
    close();
}

// Editors menu actions
void MainWindow::showLensInspector()
{
    QMessageBox::information(this, tr("Lens Inspector"), tr("This feature is not yet implemented."));
}

void MainWindow::showSystemViewer()
{
    QMessageBox::information(this, tr("System Viewer"), tr("This feature is not yet implemented."));
}

// Analysis menu actions
void MainWindow::showXYPlot()
{
    QMessageBox::information(this, tr("XY Plot"), tr("This feature is not yet implemented."));
}

void MainWindow::show2DPlot()
{
    QMessageBox::information(this, tr("2D Plot"), tr("This feature is not yet implemented."));
}

// View menu actions
void MainWindow::setLightTheme()
{
    if (m_themeManager) m_themeManager->setTheme(ThemeManager::Theme::Light);
    m_settings->setValue("theme", "light");
}

void MainWindow::setDarkTheme()
{
    if (m_themeManager) m_themeManager->setTheme(ThemeManager::Theme::Dark);
    m_settings->setValue("theme", "dark");
}

void MainWindow::setSystemTheme()
{
    if (m_themeManager) m_themeManager->setTheme(ThemeManager::Theme::System);
    m_settings->setValue("theme", "system");
}

void MainWindow::setLanguage(const QString& language)
{
    if (language == m_currentLocale.name().left(2)) return;
    
    // Remove old translator
    QApplication::removeTranslator(m_translator);
    
    // Load new translation
    QString translationFile = QString(":/translations/phoenix_%1.qm").arg(language);
    if (m_translator->load(translationFile)) {
        QApplication::installTranslator(m_translator);
        m_currentLocale = QLocale(language);
        m_settings->setValue("language", language);
        
        // Retranslate UI
        retranslateUi();
    }
}

void MainWindow::onThemeChanged()
{
    // Handle theme changes
    updateDebugInfo();
}

void MainWindow::applyIcons()
{
    const bool dark = m_themeManager->isDarkMode();
    
    // File menu actions
    m_newAction->setIcon(getIcon("file-plus", "file-plus"));
    m_openAction->setIcon(getIcon("folder", "folder-open"));
    m_saveAction->setIcon(getIcon("floppy-disk", "save"));
    m_saveAsAction->setIcon(getIcon("floppy-disk-pen", "save-as"));
    m_preferencesAction->setIcon(getIcon("sliders", "settings"));
    m_exitAction->setIcon(getIcon("power-off", "close"));
    
    // Editors menu actions
    m_lensInspectorAction->setIcon(getIcon("lens", "search"));
    m_systemViewerAction->setIcon(getIcon("desktop", "view"));
    
    // Analysis menu actions
    m_xyPlotAction->setIcon(getIcon("chart-line", "chart"));
    m_2dPlotAction->setIcon(getIcon("chart-bar", "chart"));
    
    // Help menu actions (including Help-About)
    // Find the Help menu and update its About action
    QList<QMenu*> menus = m_menuBar->findChildren<QMenu*>();
    for (QMenu* menu : menus) {
        if (menu->title().contains("Help")) {
            QList<QAction*> helpActions = menu->actions();
            for (QAction* action : helpActions) {
                if (action->text().contains("About")) {
                    action->setIcon(getIcon("info-circle", "info"));
                    action->update();
                }
            }
            break;
        }
    }
    
    // Update ribbon icons (including Help-About)
    updateRibbonIcons();
}

void MainWindow::initializeUI()
{
    // This method is called by QTimer::singleShot in the constructor
    // It's already implemented in the constructor, so this is a stub
    // The actual initialization happens in the constructor
}

void MainWindow::retranslateUi()
{
    // Retranslate all UI elements
    setWindowTitle(tr("Phoenix - Optical Design Studio"));
    
    // Update menu titles
    // This would need to be called for all menu items
    // For now, we'll implement a basic retranslation
    updateStatusMessage(tr("Ready"));
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("About Phoenix"),
                       QString(tr("Phoenix UI for Bedrock\nVersion %1\n\nBuilt with Qt %2\nRunning on %3"))
                       .arg(QApplication::applicationVersion())
                       .arg(QT_VERSION_STR)
                       .arg(QSysInfo::prettyProductName()));
}


void MainWindow::showHelp()
{
    QMessageBox::information(this, tr("Help"), tr("This feature is not yet implemented."));
}

// Telemetry hooks for UI latency logging
void MainWindow::logUIAction(const QString& action, qint64 elapsed)
{
    // Log UI action with timing for <50ms chrome response validation
    qDebug() << "UI Action:" << action << "took" << elapsed << "ms";
    
    // Check if response time meets Phase 1 requirements (<50ms)
    if (elapsed > 50) {
        qWarning() << "SLOW UI ACTION:" << action << "took" << elapsed << "ms (exceeds 50ms target)";
    }
    
    // Update status bar with performance info
    updateStatusMessage(QString("Action: %1 (%2ms)").arg(action).arg(elapsed));
}

void MainWindow::logRibbonAction(const QString& action)
{
    // Start timing for ribbon actions
    m_actionTimer.start();
    
    // Log the ribbon action
    qDebug() << "Ribbon Action:" << action;
    
    // Connect to the actual action completion
    QTimer::singleShot(0, this, [this, action]() {
        qint64 elapsed = m_actionTimer.elapsed();
        logUIAction(QString("ribbon_%1").arg(action), elapsed);
    });
}
