#include "MainWindow.h"
#include "../dialogs/PreferencesDialog.h"
#include "../themes/ThemeManager.h"
#include "../icons/IconProvider.h"
#include "../icons/PhxLogging.h"
#include "app/SettingsProvider.h"
#include "app/SettingsKeys.h"
#include "app/io/FileIO.h"
#include "app/PhxConstants.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QToolButton>
#include <QLayout>
#include <QStatusBar>
#include <QDockWidget>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>
#include <QLocale>
#include <QTranslator>
#include <QProcess>
#include <QThread>
#include <QDebug>
#include <QShowEvent>
#include <QStatusBar>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QSet>
#include <QAction>
#include <QMenu>

namespace {
    // Gather all menus from both menubar actions and MainWindow tree
    // This ensures we find all menus regardless of parenting
    static QList<QMenu*> gatherAllMenus(QMenuBar* menubar, QWidget* root) {
        QSet<QMenu*> set;
        
        // A. Menus referenced by menubar actions (official source of top-level menus)
        if (menubar) {
            for (QAction* a : menubar->actions()) {
                if (QMenu* m = a->menu()) {
                    set.insert(m);
                }
            }
        }
        
        // B. Any QMenus parented in the MainWindow tree (covers new QMenu(title, this))
        for (QMenu* m : root->findChildren<QMenu*>()) {
            set.insert(m);
        }
        
        // (Optional) Recurse to collect submenus, if needed later:
        // auto gatherSubmenus = [&](QMenu* parent, auto&& gatherSubmenusRef) -> void {
        //     for (QAction* a : parent->actions()) {
        //         if (QMenu* sm = a->menu()) {
        //             if (set.contains(sm)) continue;
        //             set.insert(sm);
        //             gatherSubmenusRef(sm, gatherSubmenusRef);
        //         }
        //     }
        // };
        // for (QMenu* m : set) gatherSubmenus(m, gatherSubmenus);
        
        return set.values();
    }
} // anonymous namespace

MainWindow::MainWindow(SettingsProvider* sp, QWidget *parent)
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
    , m_settingsProvider(sp)
    , m_translator(new QTranslator(this))
    , m_themeManager(nullptr)  // Defer initialization to avoid circular dependency
    , m_debugTimer(new QTimer(this))
    , m_startupTime(0)
{
    setWindowTitle("Phoenix - Optical Design Studio");
    setMinimumSize(phx::ui::kMainMinSize);
    resize(phx::ui::kDefaultWindowSize);
    
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
    setupFloatingToolbarsAndDocks();  // Setup floating behavior for toolbars and docks
    setupStatusBar();
    setupConnections();
    setupTheme();
    loadSettings();
    
    // Setup translations after UI is ready
    setupTranslations();
    
    // Start debug info updates
    m_debugTimer->setInterval(phx::ui::kTelemetryIntervalMs); // Update every second
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

bool MainWindow::event(QEvent* e)
{
    if (e->type() == QEvent::WindowActivate) {
        // Nudge floating bars above their parent window when MainWindow regains focus
        for (auto *tb : findChildren<QToolBar*>()) {
            if (tb->isFloating()) {
                tb->raise();
            }
        }
        for (auto *dw : findChildren<QDockWidget*>()) {
            if (dw->isFloating()) {
                dw->raise();
            }
        }
    } else if (e->type() == QEvent::PaletteChange || e->type() == QEvent::ApplicationPaletteChange) {
        // Delay icon refresh until after palette propagation
        QTimer::singleShot(0, this, [this] {
            refreshAllIconsForTheme();
        });
    }
    return QMainWindow::event(e);
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
    
    m_newAction = new QAction(tr("&New"), this);
    m_newAction->setProperty("phx_icon_key", "file-plus");
    const int px = fileMenu->style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, fileMenu);
    m_newAction->setIcon(IconProvider::icon("file-plus", QSize(px, px), fileMenu));
    m_newAction->setIconVisibleInMenu(true);
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip(tr("Create a new file"));
    connect(m_newAction, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(m_newAction);
    
    m_openAction = new QAction(tr("&Open"), this);
    m_openAction->setProperty("phx_icon_key", "folder-open");
    m_openAction->setIcon(IconProvider::icon("folder-open", QSize(px, px), fileMenu));
    m_openAction->setIconVisibleInMenu(true);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip(tr("Open an existing file"));
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(m_openAction);
    
    fileMenu->addSeparator();
    
    m_saveAction = new QAction(tr("&Save"), this);
    m_saveAction->setProperty("phx_icon_key", "save");
    m_saveAction->setIcon(IconProvider::icon("save", QSize(px, px), fileMenu));
    m_saveAction->setIconVisibleInMenu(true);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip(tr("Save the current file"));
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    fileMenu->addAction(m_saveAction);
    
    m_saveAsAction = new QAction(tr("Save &As"), this);
    m_saveAsAction->setProperty("phx_icon_key", "save-as");
    m_saveAsAction->setIcon(IconProvider::icon("save-as", QSize(px, px), fileMenu));
    m_saveAsAction->setIconVisibleInMenu(true);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip(tr("Save the current file with a new name"));
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveAsFile);
    fileMenu->addAction(m_saveAsAction);
    
    fileMenu->addSeparator();
    
    m_preferencesAction = new QAction(tr("&Preferences..."), this);
    m_preferencesAction->setProperty("phx_icon_key", "settings");
    m_preferencesAction->setIcon(IconProvider::icon("settings", QSize(px, px), fileMenu));
    m_preferencesAction->setIconVisibleInMenu(true);
    m_preferencesAction->setShortcut(QKeySequence::Preferences);
    m_preferencesAction->setStatusTip(tr("Open preferences dialog"));
    connect(m_preferencesAction, &QAction::triggered, this, &MainWindow::showPreferences);
    fileMenu->addAction(m_preferencesAction);
    
    fileMenu->addSeparator();
    
    m_exitAction = new QAction(tr("E&xit"), this);
    m_exitAction->setProperty("phx_icon_key", "close");
    m_exitAction->setIcon(IconProvider::icon("close", QSize(px, px), fileMenu));
    m_exitAction->setIconVisibleInMenu(true);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the application"));
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    fileMenu->addAction(m_exitAction);
    
    return fileMenu;
}

QMenu* MainWindow::createEditorsMenu()
{
    QMenu* editorsMenu = new QMenu(tr("&Editors"), this);
    
    m_lensInspectorAction = new QAction(tr("&Lens Inspector"), this);
    m_lensInspectorAction->setProperty("phx_icon_key", "search");
    const int px = editorsMenu->style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, editorsMenu);
    m_lensInspectorAction->setIcon(IconProvider::icon("search", QSize(px, px), editorsMenu));
    m_lensInspectorAction->setIconVisibleInMenu(true);
    m_lensInspectorAction->setStatusTip(tr("Open lens inspector"));
    connect(m_lensInspectorAction, &QAction::triggered, this, &MainWindow::showLensInspector);
    editorsMenu->addAction(m_lensInspectorAction);
    
    m_systemViewerAction = new QAction(tr("&System Viewer"), this);
    m_systemViewerAction->setProperty("phx_icon_key", "view");
    m_systemViewerAction->setIcon(IconProvider::icon("view", QSize(px, px), editorsMenu));
    m_systemViewerAction->setIconVisibleInMenu(true);
    m_systemViewerAction->setStatusTip(tr("Open system viewer"));
    connect(m_systemViewerAction, &QAction::triggered, this, &MainWindow::showSystemViewer);
    editorsMenu->addAction(m_systemViewerAction);
    
    return editorsMenu;
}

QMenu* MainWindow::createAnalysisMenu()
{
    QMenu* analysisMenu = new QMenu(tr("&Analysis"), this);
    
    m_xyPlotAction = new QAction(tr("&XY Plot"), this);
    m_xyPlotAction->setProperty("phx_icon_key", "chart");
    const int px = analysisMenu->style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, analysisMenu);
    m_xyPlotAction->setIcon(IconProvider::icon("chart", QSize(px, px), analysisMenu));
    m_xyPlotAction->setIconVisibleInMenu(true);
    m_xyPlotAction->setStatusTip(tr("Open XY plot analysis"));
    connect(m_xyPlotAction, &QAction::triggered, this, &MainWindow::showXYPlot);
    analysisMenu->addAction(m_xyPlotAction);
    
    m_2dPlotAction = new QAction(tr("&2D Plot"), this);
    m_2dPlotAction->setProperty("phx_icon_key", "chart");
    m_2dPlotAction->setIcon(IconProvider::icon("chart", QSize(px, px), analysisMenu));
    m_2dPlotAction->setIconVisibleInMenu(true);
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
    
    QAction* aboutAction = new QAction(tr("&About Phoenix"), this);
    aboutAction->setProperty("phx_icon_key", "info");
    const int px = helpMenu->style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, helpMenu);
    aboutAction->setIcon(IconProvider::icon("info", QSize(px, px), helpMenu));
    aboutAction->setIconVisibleInMenu(true);
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
    toolBar->setAllowedAreas(Qt::AllToolBarAreas);
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
    toolBar->setIconSize(QSize(phx::ui::kRibbonIconPx, phx::ui::kRibbonIconPx));
    
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
}

QToolBar* MainWindow::createTopRibbon()
{
    QToolBar* ribbon = new QToolBar(tr("Top Ribbon"), this);
    ribbon->setObjectName("topRibbon");
    ribbon->setMovable(true);
    ribbon->setFloatable(true);
    ribbon->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    ribbon->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ribbon->setIconSize(QSize(phx::ui::kRibbonIconPx, phx::ui::kRibbonIconPx));
    
    // File actions
    QAction* newAction = new QAction(tr("New"), this);
    newAction->setProperty("phx_icon_key", "file-plus");
    newAction->setIcon(IconProvider::icon("file-plus", ribbon->iconSize(), ribbon));
    newAction->setToolTip(tr("Create new file"));
    connect(newAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        newFile(); 
        logRibbonAction("new_file");
    });
    ribbon->addAction(newAction);
    
    QAction* openAction = new QAction(tr("Open"), this);
    openAction->setProperty("phx_icon_key", "folder-open");
    openAction->setIcon(IconProvider::icon("folder-open", ribbon->iconSize(), ribbon));
    openAction->setToolTip(tr("Open existing file"));
    connect(openAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        openFile(); 
        logRibbonAction("open_file");
    });
    ribbon->addAction(openAction);
    
    QAction* saveAction = new QAction(tr("Save"), this);
    saveAction->setProperty("phx_icon_key", "save");
    saveAction->setIcon(IconProvider::icon("save", ribbon->iconSize(), ribbon));
    saveAction->setToolTip(tr("Save current file"));
    connect(saveAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        saveFile(); 
        logRibbonAction("save_file");
    });
    ribbon->addAction(saveAction);
    
    ribbon->addSeparator();
    
    // Analysis actions
    QAction* xyPlotAction = new QAction(tr("XY Plot"), this);
    xyPlotAction->setProperty("phx_icon_key", "chart");
    xyPlotAction->setIcon(IconProvider::icon("chart", ribbon->iconSize(), ribbon));
    xyPlotAction->setToolTip(tr("Open XY plot analysis"));
    connect(xyPlotAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        showXYPlot(); 
        logRibbonAction("xy_plot");
    });
    ribbon->addAction(xyPlotAction);
    
    QAction* plot2DAction = new QAction(tr("2D Plot"), this);
    plot2DAction->setProperty("phx_icon_key", "chart");
    plot2DAction->setIcon(IconProvider::icon("chart", ribbon->iconSize(), ribbon));
    plot2DAction->setToolTip(tr("Open 2D plot analysis"));
    connect(plot2DAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        show2DPlot(); 
        logRibbonAction("2d_plot");
    });
    ribbon->addAction(plot2DAction);
    
    ribbon->addSeparator();
    
    // Tools actions
    QAction* preferencesAction = new QAction(tr("Preferences"), this);
    preferencesAction->setProperty("phx_icon_key", "settings");
    preferencesAction->setIcon(IconProvider::icon("settings", ribbon->iconSize(), ribbon));
    preferencesAction->setToolTip(tr("Open preferences"));
    connect(preferencesAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        showPreferences(); 
        logRibbonAction("preferences");
    });
    ribbon->addAction(preferencesAction);
    
    return ribbon;
}

QToolBar* MainWindow::createRightRibbon()
{
    QToolBar* ribbon = new QToolBar(tr("Right Ribbon"), this);
    ribbon->setObjectName("rightRibbon");
    ribbon->setMovable(true);
    ribbon->setFloatable(true);
    ribbon->setAllowedAreas(Qt::AllToolBarAreas);
    ribbon->setOrientation(Qt::Vertical);
    ribbon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ribbon->setIconSize(QSize(phx::ui::kToolbarIconPx, phx::ui::kToolbarIconPx));
    
    // Editors actions
    QAction* lensInspectorAction = new QAction(tr("Lens Inspector"), this);
    lensInspectorAction->setProperty("phx_icon_key", "search");
    lensInspectorAction->setIcon(IconProvider::icon("search", ribbon->iconSize(), ribbon));
    lensInspectorAction->setToolTip(tr("Open lens inspector"));
    connect(lensInspectorAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        showLensInspector(); 
        logRibbonAction("lens_inspector");
    });
    ribbon->addAction(lensInspectorAction);
    
    QAction* systemViewerAction = new QAction(tr("System Viewer"), this);
    systemViewerAction->setProperty("phx_icon_key", "view");
    systemViewerAction->setIcon(IconProvider::icon("view", ribbon->iconSize(), ribbon));
    systemViewerAction->setToolTip(tr("Open system viewer"));
    connect(systemViewerAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        showSystemViewer(); 
        logRibbonAction("system_viewer");
    });
    ribbon->addAction(systemViewerAction);
    
    ribbon->addSeparator();
    
    // View actions
    QAction* lightThemeAction = new QAction(tr("Light Theme"), this);
    lightThemeAction->setProperty("phx_icon_key", "light");
    lightThemeAction->setIcon(IconProvider::icon("light", ribbon->iconSize(), ribbon));
    lightThemeAction->setToolTip(tr("Switch to light theme"));
    lightThemeAction->setCheckable(true);
    connect(lightThemeAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        setLightTheme(); 
        logRibbonAction("light_theme");
    });
    ribbon->addAction(lightThemeAction);
    
    QAction* darkThemeAction = new QAction(tr("Dark Theme"), this);
    darkThemeAction->setProperty("phx_icon_key", "dark");
    darkThemeAction->setIcon(IconProvider::icon("dark", ribbon->iconSize(), ribbon));
    darkThemeAction->setToolTip(tr("Switch to dark theme"));
    darkThemeAction->setCheckable(true);
    connect(darkThemeAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        setDarkTheme(); 
        logRibbonAction("dark_theme");
    });
    ribbon->addAction(darkThemeAction);
    
    ribbon->addSeparator();
    
    // Help actions
    QAction* helpAction = new QAction(tr("Help"), this);
    helpAction->setProperty("phx_icon_key", "help");
    helpAction->setIcon(IconProvider::icon("help", ribbon->iconSize(), ribbon));
    helpAction->setToolTip(tr("Open help"));
    connect(helpAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        showHelp(); 
        logRibbonAction("help");
    });
    ribbon->addAction(helpAction);
    
    QAction* aboutAction = new QAction(tr("About"), this);
    aboutAction->setProperty("phx_icon_key", "info");
    aboutAction->setIcon(IconProvider::icon("info", ribbon->iconSize(), ribbon));
    aboutAction->setToolTip(tr("Show about dialog"));
    connect(aboutAction, &QAction::triggered, this, [this]() { 
        m_actionTimer.start(); 
        showAbout(); 
        logRibbonAction("about");
    });
    ribbon->addAction(aboutAction);
    
    // Ensure consistent left alignment for all items in the vertical ribbon
    ribbon->setLayoutDirection(Qt::LeftToRight);
    ribbon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ribbon->setContentsMargins(0, 0, 0, 0);
    
    if (QLayout* lay = ribbon->layout()) {
        lay->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        lay->setSpacing(4);  // keep vertical rhythm consistent
    }
    
    // Normalize each button widget
    for (QAction* a : ribbon->actions()) {
        QWidget* w = ribbon->widgetForAction(a);
        if (!w) continue;  // separators etc.
        
        if (auto* btn = qobject_cast<QToolButton*>(w)) {
            btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            btn->setLayoutDirection(Qt::LeftToRight);
            btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            btn->setContentsMargins(0, 0, 0, 0);
            // Avoid stylesheets; inherent alignment via LTR + text-beside-icon is sufficient
        }
    }
    
    return ribbon;
}

void MainWindow::setupDockWidgets()
{
    // Toolbox dock (left)
    m_toolboxDock = new QDockWidget(tr("Toolbox"), this);
    m_toolboxDock->setObjectName("toolboxDock");
    m_toolboxDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_toolboxDock->setMinimumWidth(phx::ui::kDockMinWidth);
    m_toolboxDock->setMaximumWidth(phx::ui::kDockWideWidth);
    
    QWidget* toolboxWidget = new QWidget();
    toolboxWidget->setMinimumSize(phx::ui::kDockMinWidth, phx::ui::kPanelMinHeight);
    m_toolboxDock->setWidget(toolboxWidget);
    addDockWidget(Qt::LeftDockWidgetArea, m_toolboxDock);
    
    // Properties dock (right)
    m_propertiesDock = new QDockWidget(tr("Properties"), this);
    m_propertiesDock->setObjectName("propertiesDock");
    m_propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_propertiesDock->setMinimumWidth(phx::ui::kDockMinWidth);
    m_propertiesDock->setMaximumWidth(phx::ui::kDockWideWidth);
    
    QWidget* propertiesWidget = new QWidget();
    propertiesWidget->setMinimumSize(phx::ui::kDockMinWidth, phx::ui::kPanelMinHeight);
    m_propertiesDock->setWidget(propertiesWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
    
    // Central widget placeholder
    QWidget* centralWidget = new QWidget();
    centralWidget->setMinimumSize(phx::ui::kDockWideWidth, phx::ui::kPanelMinHeight);
    setCentralWidget(centralWidget);
}

void MainWindow::setupFloatingToolbarsAndDocks()
{
    // Lambda to setup floating behavior for both QToolBar and QDockWidget
    auto setupFloatingBehavior = [this](QWidget *w) {
        // Connect to topLevelChanged signal (available on both QToolBar and QDockWidget)
        if (auto *tb = qobject_cast<QToolBar*>(w)) {
            connect(tb, &QToolBar::topLevelChanged, this, [this, w](bool floating) {
                if (floating) {
                    w->setWindowFlag(Qt::Tool, true);
                    w->setWindowFlag(Qt::WindowStaysOnTopHint, true);
                    w->show();    // re-apply flags
                    w->raise();
                    w->activateWindow();
                } else {
                    w->setWindowFlag(Qt::WindowStaysOnTopHint, false);
                    w->setWindowFlag(Qt::Tool, false);
                    // No show() required here, docked windows are managed by QMainWindow
                }
            });
        } else if (auto *dw = qobject_cast<QDockWidget*>(w)) {
            connect(dw, &QDockWidget::topLevelChanged, this, [this, w](bool floating) {
                if (floating) {
                    w->setWindowFlag(Qt::Tool, true);
                    w->setWindowFlag(Qt::WindowStaysOnTopHint, true);
                    w->show();
                    w->raise();
                    w->activateWindow();
                } else {
                    w->setWindowFlag(Qt::WindowStaysOnTopHint, false);
                    w->setWindowFlag(Qt::Tool, false);
                    // No show() required here, docked windows are managed by QMainWindow
                }
            });
        }
    };
    
    // Apply to all toolbars
    setupFloatingBehavior(m_mainToolBar);
    setupFloatingBehavior(m_topRibbon);
    setupFloatingBehavior(m_rightRibbon);
    
    // Apply to all dock widgets
    setupFloatingBehavior(m_toolboxDock);
    setupFloatingBehavior(m_propertiesDock);
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
    if (!m_settingsProvider) return;
    // Load current language from settings
    auto& s = m_settingsProvider->settings();
    QString language = s.value(PhxKeys::I18N_LANGUAGE, "en").toString();
    setLanguage(language);
}

void MainWindow::setupTheme()
{
    if (!m_settingsProvider) return;
    // Load current theme from settings
    auto& s = m_settingsProvider->settings();
    QString theme = s.value(PhxKeys::UI_THEME, "system").toString();
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

void MainWindow::loadSettings()
{
    if (!m_settingsProvider) return;
    // Load window geometry
    auto& s = m_settingsProvider->settings();
    restoreGeometry(s.value(PhxKeys::UI_GEOMETRY).toByteArray());
    restoreState(s.value(PhxKeys::UI_WINDOW_STATE).toByteArray());
}

void MainWindow::saveSettings()
{
    if (!m_settingsProvider) return;
    // Save window geometry
    auto& s = m_settingsProvider->settings();
    s.setValue(PhxKeys::UI_GEOMETRY, saveGeometry());
    s.setValue(PhxKeys::UI_WINDOW_STATE, saveState());
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

QIcon MainWindow::getIcon(const QString& name, QWidget* widget) const
{
    QWidget* host = widget ? widget : const_cast<MainWindow*>(this);
    const int px = host->style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, host);
    return IconProvider::icon(name, QSize(px, px), host);
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
    // TODO: Implement actual save logic when file formats are defined
    // For now, show "not implemented" message
    
    // TODO (Future): When implementing:
    // 1. Get target path from current document or use QFileDialog
    // 2. Validate path using FileIO::canonicalize()
    // 3. Ensure parent directory exists: FileIO::ensureDir(QFileInfo(path).absolutePath(), &err)
    // 4. For long operations, use QtConcurrent::run + QFutureWatcher to avoid blocking GUI thread
    // 5. Write using: FileIO::writeTextFileAtomic(path, content, &err)
    // 6. On failure, show QMessageBox::warning with path and error message
    
    QMessageBox::information(this, tr("Save"), tr("Not implemented in this sprint."));
}

void MainWindow::saveAsFile()
{
    // TODO: Implement actual save-as logic when file formats are defined
    // For now, show "not implemented" message
    
    // TODO (Future): When implementing:
    // 1. Use QFileDialog::getSaveFileName() to get target path
    // 2. Validate path (non-empty, valid parent directory)
    // 3. Use FileIO::canonicalize() for final path
    // 4. Ensure parent directory: FileIO::ensureDir(QFileInfo(path).absolutePath(), &err)
    // 5. For long operations, use QtConcurrent::run + QFutureWatcher to avoid blocking GUI thread
    // 6. Write using: FileIO::writeTextFileAtomic(path, content, &err)
    // 7. On failure, show QMessageBox::warning with path and error message
    
    QString targetPath = QFileDialog::getSaveFileName(this, tr("Save As"), "", tr("All Files (*)"));
    if (targetPath.isEmpty()) {
        return; // User cancelled
    }
    
    // Validate path
    QString canonicalPath = FileIO::canonicalize(targetPath);
    if (canonicalPath.isEmpty()) {
        QMessageBox::warning(this, tr("Save As"), tr("Invalid file path."));
        return;
    }
    
    // Ensure parent directory exists
    QFileInfo fileInfo(canonicalPath);
    QString parentDir = fileInfo.absolutePath();
    QString dirError;
    if (!FileIO::ensureDir(parentDir, &dirError)) {
        QMessageBox::warning(this, tr("Save As"), 
            tr("Failed to create directory:\n%1\n\n%2").arg(parentDir, dirError));
        return;
    }
    
    // TODO: Actual file writing will be implemented later
    // For now, just show "not implemented" message
    QMessageBox::information(this, tr("Save As"), tr("Not implemented in this sprint."));
}

void MainWindow::showPreferences()
{
    if (!m_preferencesDialog) {
        if (!m_settingsProvider) return;
        // Pass QSettings& reference to PreferencesDialog
        m_preferencesDialog = new PreferencesDialog(m_settingsProvider->settings(), this);
        m_preferencesDialog->setAttribute(Qt::WA_DeleteOnClose, true);  // auto-delete on close
    } else {
        m_preferencesDialog->raise();
        m_preferencesDialog->activateWindow();
    }
    m_preferencesDialog->show();  // non-modal
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
    // ThemeManager now saves theme via SettingsProvider
}

void MainWindow::setDarkTheme()
{
    if (m_themeManager) m_themeManager->setTheme(ThemeManager::Theme::Dark);
    // ThemeManager now saves theme via SettingsProvider
}

void MainWindow::setSystemTheme()
{
    if (m_themeManager) m_themeManager->setTheme(ThemeManager::Theme::System);
    // ThemeManager now saves theme via SettingsProvider
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
        if (m_settingsProvider) {
            auto& s = m_settingsProvider->settings();
            s.setValue(PhxKeys::I18N_LANGUAGE, language);
        }
        
        // Retranslate UI
        retranslateUi();
    }
}

void MainWindow::onThemeChanged()
{
    // Delay icon refresh until after palette propagation (QApplication::setPalette() is synchronous,
    // but we want to ensure all widgets have received the palette change event)
    QTimer::singleShot(0, this, [this] {
        refreshAllIconsForTheme();
    });
    // Handle theme changes
    updateDebugInfo();
}

void MainWindow::applyIcons()
{
    // This method is kept for backward compatibility but now delegates to refreshAllIconsForTheme()
    refreshAllIconsForTheme();
}

void MainWindow::refreshAllIconsForTheme()
{
    qCDebug(phxIcons) << "GLOBAL ICON REFRESH (menus+toolbars) after theme/palette change";
    
    IconProvider::clearCache();
    
    // Unified rebuildAction lambda: always clear→set to drop cached pixmaps
    auto rebuildAction = [](QAction* a, QWidget* w) {
        if (!a) return;
        
        const auto key = a->property("phx_icon_key").toString();
        if (key.isEmpty()) return;  // skip actions without a logical icon key
        
        int px = w->style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, w);
        if (auto tb = qobject_cast<QToolBar*>(w)) {
            const auto sz = tb->iconSize();
            if (sz.isValid() && sz.width() > 0) px = sz.width();
        }
        
        // Clear cached pixmap then rebuild with new palette/host
        a->setIcon(QIcon());  // clear cached pixmap
        a->setIcon(IconProvider::icon(key, QSize(px, px), w));  // rebuilt with new palette/host
    };
    
    // Gather all menus from both menubar actions and MainWindow tree
    const auto menus = gatherAllMenus(menuBar(), this);
    
#ifdef PHX_DEV_DIAG
    {
        QStringList names;
        names.reserve(menus.size());
        for (auto* m : menus) {
            names << m->title();
        }
        qCDebug(phxIcons) << "MENUS FOUND:" << names;
    }
#endif
    
    // Close any open menus first to clear Qt's cached pixmaps
    for (QMenu* m : menus) {
        if (m && m->isVisible()) {
            m->hide();  // Close to clear cached pixmaps
        }
    }
    
    // Menu actions (use unified lambda with icon visibility forced)
    for (QMenu* m : menus) {
        if (!m) continue;
        
        for (QAction* a : m->actions()) {
            rebuildAction(a, m);
            // Force icon visibility in menus (Qt may cache this)
            a->setIconVisibleInMenu(true);
        }
        m->update();  // repaint menu shell
    }
    
    // Note: We intentionally don't reopen menus that were closed - user can reopen them
    // This ensures Qt rebuilds menu rendering with fresh icons
    
    // Toolbar actions (use unified lambda + repaint)
    for (QToolBar* tb : findChildren<QToolBar*>()) {
        for (QAction* a : tb->actions()) {
            rebuildAction(a, tb);
        }
        tb->update();  // repaint toolbar shell
    }
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
    // Use IconProvider glyph icon so it tints with theme
    const QIcon infoIcon = IconProvider::icon("info",
        QSize(phx::ui::kMenuIconPx, phx::ui::kMenuIconPx), this);
    
    QMessageBox mb(QMessageBox::Information,
                   tr("About Phoenix"),
                   QString(tr("Phoenix UI for Bedrock\nVersion %1\n\nBuilt with Qt %2\nRunning on %3"))
                       .arg(QApplication::applicationVersion())
                       .arg(QT_VERSION_STR)
                       .arg(QSysInfo::prettyProductName()),
                   QMessageBox::Ok,
                   this);
    
    mb.setWindowIcon(infoIcon);
    mb.setIconPixmap(infoIcon.pixmap(QSize(phx::ui::kMenuIconPx, phx::ui::kMenuIconPx)));
    mb.exec();
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
    if (elapsed > phx::ui::kUITargetResponseMs) {
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
