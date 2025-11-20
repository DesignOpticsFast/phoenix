#include "MainWindow.h"
#include "../dialogs/PreferencesDialog.h"
#include "../themes/ThemeManager.h"
#include "../icons/IconProvider.h"
#include "../icons/PhxLogging.h"
#include "../UILogging.h"
#include "app/LocaleInit.hpp"
#include "app/SettingsProvider.h"
#include "app/SettingsKeys.h"
#include "app/MemoryMonitor.hpp"
#include "app/io/FileIO.h"
#include "app/PhxConstants.h"
#include "version.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>
#include <QWidgetAction>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLayout>
#include <QStatusBar>
#include <QDockWidget>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QTimer>
#include <QElapsedTimer>
#include <QDir>
#include <QLocale>
#include <QThread>
#include <QDateTime>
#include <QShowEvent>
#include <QFocusEvent>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QSet>
#include <QAction>
#include <QMenu>
#include <QStyle>
#include <QVariant>
#include <functional>
#include <cmath>
#include "ui/analysis/AnalysisWindow.hpp"
#include "ui/analysis/AnalysisWindowManager.hpp"
#include "plot/XYPlotViewGraphs.hpp"
#include "ui/analysis/XYAnalysisWindow.hpp"
#include <QDockWidget>
#include <QPointF>
#include <vector>
#include <memory>

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
    , m_themeMenu(nullptr)
    , m_windowMenu(nullptr)
    , m_uiInitialized(false)
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
    , m_themeManager(nullptr)  // Defer initialization to avoid circular dependency
    , m_debugTimer(new QTimer(this))
{
    // Set initial window title (will be retranslated after translators are active)
    setWindowTitle(QStringLiteral("Phoenix %1 - Optical Design Studio")
                   .arg(QStringLiteral(PHOENIX_VERSION)));
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
    
    // Retranslate UI elements (including window title) now that translators are active
    retranslateUi();
    
    m_uiInitialized = true;
    
    // Keep timer setup in constructor (setup only, no start)
    m_debugTimer->setInterval(phx::ui::kTelemetryIntervalMs); // Update every second
    connect(m_debugTimer, &QTimer::timeout, this, &MainWindow::updateDebugInfo);
    
    // Defer dynamic UI work until the event loop is running
    QTimer::singleShot(0, this, [this]() {
        // m_startupStartTime is set from main.cpp when splash is shown
        // We'll compute duration in updateDebugInfo() when UI is ready
        
        // 1) Compute safe icon size now that style is ready
        const QSize startupIconSize = safeIconSizeHint();
        
        // 2) Refresh theme-dependent icons
        refreshThemeActionIcons(startupIconSize);
        
        // 3) Apply ribbon palette/QSS
        if (m_rightRibbon) {
            applyRibbonPalette(m_rightRibbon);
        }
        
        // 4) Sync initial theme checked state (Light/Dark/System)
        if (m_themeManager) {
            const auto startupTheme = m_themeManager->currentTheme();
            if (m_lightThemeAction) {
                m_lightThemeAction->setChecked(startupTheme == ThemeManager::Theme::Light);
            }
            if (m_darkThemeAction) {
                m_darkThemeAction->setChecked(startupTheme == ThemeManager::Theme::Dark);
            }
            if (m_systemThemeAction) {
                m_systemThemeAction->setChecked(startupTheme == ThemeManager::Theme::System);
            }
        }
        
        // 5) Initial status/debug update (will compute duration and flip to Ready)
        updateStatusBar();
        
        // 6) Start debug timer
        if (m_debugTimer) {
            m_debugTimer->start();
        }
    });
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Close all analysis windows and tool windows before closing main window
    AnalysisWindowManager::instance()->closeAllWindows();
    
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::focusInEvent(QFocusEvent* event)
{
    // Call base class first to preserve normal focus behavior
    QMainWindow::focusInEvent(event);
    
    // When MainWindow gets focus, raise all visible analysis windows above it
    // This ensures analysis windows stay above MainWindow but below tool windows
    if (AnalysisWindowManager* mgr = AnalysisWindowManager::instance()) {
        mgr->raiseAllAnalysisWindows();
    }
}

bool MainWindow::event(QEvent* e)
{
    if (e->type() == QEvent::WindowActivate) {
        // Removed raise() calls on floating toolbars/docks - on Wayland this triggers
        // popup-only mouse grab. Qt's default z-ordering handles floating widgets correctly.
        // No action needed here; Qt manages floating widget stacking automatically.
    } else if (e->type() == QEvent::WindowDeactivate) {
        // Don't raise floating windows when Phoenix loses focus
        return QMainWindow::event(e);
    } else if (e->type() == QEvent::PaletteChange || e->type() == QEvent::ApplicationPaletteChange) {
        // Delay icon refresh until after palette propagation
        QTimer::singleShot(0, this, [this] {
        IconProvider::clearCache();
        refreshAllIconsForTheme();
        if (m_uiInitialized && m_rightRibbon) {
            const QSize iconSize = safeIconSizeHint();
            refreshThemeActionIcons(iconSize);
            applyRibbonPalette(m_rightRibbon);
        }
        
        if (m_statusBar) {
            m_statusBar->setPalette(QApplication::palette());
            m_statusBar->setStyleSheet(
                QStringLiteral("QStatusBar { background: palette(window); color: palette(windowText); padding: 0 8px; }")
            );
            m_statusBar->update();
        }
        
        if (m_toolboxDock && m_toolboxDock->widget()) {
            m_toolboxDock->widget()->update();
        }
        if (m_propertiesDock && m_propertiesDock->widget()) {
            m_propertiesDock->widget()->update();
        }
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
    
    // Window menu (before Help menu, standard convention)
    QMenu* windowMenu = createWindowMenu();
    m_menuBar->addMenu(windowMenu);
    
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
    m_themeMenu = themeMenu;
    m_themeGroup = new QActionGroup(this);
    m_themeGroup->setExclusive(true);

    m_lightThemeAction = new QAction(tr("&Light"), this);
    m_lightThemeAction->setCheckable(true);
    m_lightThemeAction->setActionGroup(m_themeGroup);
    m_lightThemeAction->setProperty("phx_icon_key", "light");
    m_lightThemeAction->setStatusTip(tr("Switch to light theme"));
    m_lightThemeAction->setToolTip(tr("Switch to light theme"));
    connect(m_lightThemeAction, &QAction::triggered, this, &MainWindow::setLightTheme);
    themeMenu->addAction(m_lightThemeAction);
    
    m_darkThemeAction = new QAction(tr("&Dark"), this);
    m_darkThemeAction->setCheckable(true);
    m_darkThemeAction->setActionGroup(m_themeGroup);
    m_darkThemeAction->setProperty("phx_icon_key", "dark");
    m_darkThemeAction->setStatusTip(tr("Switch to dark theme"));
    m_darkThemeAction->setToolTip(tr("Switch to dark theme"));
    connect(m_darkThemeAction, &QAction::triggered, this, &MainWindow::setDarkTheme);
    themeMenu->addAction(m_darkThemeAction);
    
    m_systemThemeAction = new QAction(tr("&System"), this);
    m_systemThemeAction->setCheckable(true);
    m_systemThemeAction->setActionGroup(m_themeGroup);
    m_systemThemeAction->setProperty("phx_icon_key", "system");
    m_systemThemeAction->setStatusTip(tr("Follow the operating system theme"));
    m_systemThemeAction->setToolTip(tr("Follow the operating system theme"));
    connect(m_systemThemeAction, &QAction::triggered, this, &MainWindow::setSystemTheme);
    themeMenu->addAction(m_systemThemeAction);
    
    viewMenu->addMenu(themeMenu);
    
    // Language submenu
    QMenu* languageMenu = new QMenu(tr("&Language"), this);
    
    QAction* englishAction = new QAction(tr("&English"), this);
    connect(englishAction, &QAction::triggered, this, [this]() { setLanguage(QStringLiteral("en")); });
    languageMenu->addAction(englishAction);
    
    QAction* germanAction = new QAction(tr("&Deutsch"), this);
    connect(germanAction, &QAction::triggered, this, [this]() { setLanguage(QStringLiteral("de")); });
    languageMenu->addAction(germanAction);
    
    viewMenu->addMenu(languageMenu);
    
    viewMenu->addSeparator();
    
    QAction* resetLayoutAction = new QAction(tr("Reset &Layout"), this);
    resetLayoutAction->setStatusTip(tr("Reset window layout to defaults"));
    connect(resetLayoutAction, &QAction::triggered, this, &MainWindow::resetLayout);
    viewMenu->addAction(resetLayoutAction);
    
    return viewMenu;
}

QMenu* MainWindow::createWindowMenu()
{
    m_windowMenu = new QMenu(tr("&Window"), this);
    
    // "Bring All to Front" action
    QAction* bringAllAction = new QAction(tr("Bring All to Front"), this);
    bringAllAction->setStatusTip(tr("Bring all analysis windows to the front"));
    connect(bringAllAction, &QAction::triggered, this, &MainWindow::onBringAllToFront);
    m_windowMenu->addAction(bringAllAction);
    
    m_windowMenu->addSeparator();
    
    // Window list will be populated dynamically by updateWindowMenu()
    // Connect aboutToShow signal to update menu contents when user opens it
    connect(m_windowMenu, &QMenu::aboutToShow, this, &MainWindow::updateWindowMenu);
    
    return m_windowMenu;
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
    addToolBar(Qt::LeftToolBarArea, m_rightRibbon);

    refreshThemeActionIcons();
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

// Creates the vertical right ribbon toolbar (theme controls + Help/About rows).
// Uses QWidgetAction rows for post-separator items to maintain left alignment.
QToolBar* MainWindow::createRightRibbon()
{
    QToolBar* ribbon = new QToolBar(tr("Right Ribbon"), this);
    ribbon->setObjectName("sideRibbon");
    ribbon->setMovable(true);
    ribbon->setFloatable(true);
    ribbon->setAllowedAreas(Qt::AllToolBarAreas);
    ribbon->setOrientation(Qt::Vertical);
    ribbon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ribbon->setIconSize(QSize(phx::ui::kToolbarIconPx, phx::ui::kToolbarIconPx));
    
    // Editors actions
    // Lens Inspector action (reuse menu action if exists)
    QAction* lensInspectorAction = m_lensInspectorAction;
    if (!lensInspectorAction) {
        lensInspectorAction = new QAction(tr("Lens Inspector"), this);
        lensInspectorAction->setProperty("phx_icon_key", "search");
        connect(lensInspectorAction, &QAction::triggered, this, [this]() { 
            m_actionTimer.start(); 
            showLensInspector(); 
            logRibbonAction("lens_inspector");
        });
        m_lensInspectorAction = lensInspectorAction;
    }
    lensInspectorAction->setIcon(IconProvider::icon("search", ribbon->iconSize(), ribbon));
    lensInspectorAction->setToolTip(tr("Open lens inspector"));
    wireSideRibbonAction(lensInspectorAction, "search", ribbon);
    
    // System Viewer action (reuse menu action if exists)
    QAction* systemViewerAction = m_systemViewerAction;
    if (!systemViewerAction) {
        systemViewerAction = new QAction(tr("System Viewer"), this);
        systemViewerAction->setProperty("phx_icon_key", "view");
        connect(systemViewerAction, &QAction::triggered, this, [this]() { 
            m_actionTimer.start(); 
            showSystemViewer(); 
            logRibbonAction("system_viewer");
        });
        m_systemViewerAction = systemViewerAction;
    }
    systemViewerAction->setIcon(IconProvider::icon("view", ribbon->iconSize(), ribbon));
    systemViewerAction->setToolTip(tr("Open system viewer"));
    wireSideRibbonAction(systemViewerAction, "view", ribbon);
    
    ribbon->addSeparator();
    
    // View actions (reuse shared theme QAction instances for exclusivity across UI)
    auto wireThemeAction = [this, ribbon](QAction* action, const QString& iconKey, const QString& logKey) {
        if (!action) {
            return;
        }

        // Set log key property for theme actions
        if (action->property("phx_log_key").toString() != logKey) {
            action->setProperty("phx_log_key", logKey);
        }

        // Use shared styling helper for consistent appearance
        wireSideRibbonAction(action, iconKey, ribbon);

        // Connect theme-specific handler
        QObject::connect(action,
                         &QAction::triggered,
                         this,
                         &MainWindow::onThemeRibbonActionTriggered,
                         Qt::UniqueConnection);

        // Theme actions are checkable (for radio behavior)
        if (QWidget* widget = ribbon->widgetForAction(action)) {
            if (auto* button = qobject_cast<QToolButton*>(widget)) {
                button->setCheckable(true);
            }
        }
    };

    wireThemeAction(m_lightThemeAction, QStringLiteral("light"), QStringLiteral("light_theme"));
    wireThemeAction(m_darkThemeAction, QStringLiteral("dark"), QStringLiteral("dark_theme"));
    wireThemeAction(m_systemThemeAction, QStringLiteral("system"), QStringLiteral("system_theme"));
    
    // Expanding spacer to push Help/About to bottom while preserving left alignment
    QWidget* spacer = new QWidget(ribbon);
    spacer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);  // ignore horizontal; push vertically
    ribbon->addWidget(spacer);
    
    // Factory lambda to create left-aligned row widgets for post-separator actions
    auto makeRibbonRow = [&](QToolBar* ribbon,
                             const QString& text,
                             const QString& iconKey,
                             const QObject* receiver,
                             std::function<void()> slot) -> QWidgetAction* {
        auto* wa = new QWidgetAction(ribbon);
        
        // Row container with left-aligned layout
        auto* row = new QWidget(ribbon);
        auto* hl = new QHBoxLayout(row);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(6);
        
        // Icon button (handles click)
        auto* btn = new QToolButton(row);
        btn->setAutoRaise(true);
        btn->setIconSize(ribbon->iconSize());
        btn->setIcon(IconProvider::icon(iconKey, ribbon->iconSize(), ribbon));
        
        // Text label (expands to fill width; keeps row content left-aligned)
        auto* label = new QLabel(text, row);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        
        // Layout: [icon-button][label][stretch]
        hl->addWidget(btn);
        hl->addWidget(label, 1);  // stretch factor
        hl->addStretch();
        
        // Click handling: clicking the icon triggers slot
        QObject::connect(btn, &QToolButton::clicked, receiver, slot);
        
        // Make entire row clickable: event filter to forward mouse press to btn
        class RowClickFilter : public QObject {
        public:
            explicit RowClickFilter(QWidget* parent) : QObject(parent) {}
            bool eventFilter(QObject* o, QEvent* e) override {
                if (e->type() == QEvent::MouseButtonRelease) {
                    if (auto* w = qobject_cast<QWidget*>(o)) {
                        if (auto* b = w->findChild<QToolButton*>()) {
                            b->click();
                            return true;  // event handled
                        }
                    }
                }
                return false;  // let other events pass through
            }
        };
        row->installEventFilter(new RowClickFilter(row));
        
        wa->setDefaultWidget(row);
        // Keep a key for refresh (used in refreshAllIconsForTheme)
        wa->setProperty("phx_icon_key", iconKey);
        return wa;
    };
    
    // Help and About actions as QWidgetAction rows
    auto* helpWA = makeRibbonRow(ribbon, tr("Help"), "help", this, [this] {
        m_actionTimer.start();
        showHelp();
        logRibbonAction("help");
    });
    ribbon->addAction(helpWA);
    
    auto* aboutWA = makeRibbonRow(ribbon, tr("About"), "info", this, [this] {
        m_actionTimer.start();
        showAbout();
        logRibbonAction("about");
    });
    ribbon->addAction(aboutWA);
    
    // Ensure consistent left alignment for all items in the vertical ribbon
    ribbon->setLayoutDirection(Qt::LeftToRight);
    ribbon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ribbon->setContentsMargins(0, 0, 0, 0);
    
    if (QLayout* lay = ribbon->layout()) {
        lay->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        lay->setSpacing(4);  // keep vertical rhythm consistent
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
    
    QWidget* toolboxContent = new QWidget(m_toolboxDock);
    auto* tLayout = new QVBoxLayout(toolboxContent);
    tLayout->setContentsMargins(8, 8, 8, 8);
    auto* tLabel = new QLabel(tr("Toolbox (empty)\nTools will appear here."), toolboxContent);
    tLabel->setAlignment(Qt::AlignCenter);
    tLabel->setStyleSheet(QStringLiteral("color: palette(mid); font-size: 11px;"));
    tLayout->addWidget(tLabel);
    m_toolboxDock->setWidget(toolboxContent);
    addDockWidget(Qt::LeftDockWidgetArea, m_toolboxDock);
    
    // Register toolbox dock with window manager for clean shutdown
    AnalysisWindowManager::instance()->registerToolWindow(m_toolboxDock);
    
    // Properties dock (right)
    m_propertiesDock = new QDockWidget(tr("Properties"), this);
    m_propertiesDock->setObjectName("propertiesDock");
    m_propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_propertiesDock->setMinimumWidth(phx::ui::kDockMinWidth);
    m_propertiesDock->setMaximumWidth(phx::ui::kDockWideWidth);
    
    QWidget* propsContent = new QWidget(m_propertiesDock);
    auto* pLayout = new QVBoxLayout(propsContent);
    pLayout->setContentsMargins(8, 8, 8, 8);
    auto* pLabel = new QLabel(tr("No selection\nProperties will appear here."), propsContent);
    pLabel->setAlignment(Qt::AlignCenter);
    pLabel->setStyleSheet(QStringLiteral("color: palette(mid); font-size: 11px;"));
    pLayout->addWidget(pLabel);
    m_propertiesDock->setWidget(propsContent);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
    
    // Register properties dock with window manager for clean shutdown
    AnalysisWindowManager::instance()->registerToolWindow(m_propertiesDock);
    
    // Central widget placeholder
    QWidget* centralWidget = new QWidget();
    centralWidget->setMinimumSize(phx::ui::kDockWideWidth, phx::ui::kPanelMinHeight);
    setCentralWidget(centralWidget);
}

void MainWindow::setupFloatingToolbarsAndDocks()
{
    auto setupFloatingBehavior = [this](QWidget* w) {
        if (!w) {
            return;
        }

        if (auto* tb = qobject_cast<QToolBar*>(w)) {
            connect(tb, &QToolBar::topLevelChanged, this, [w](bool floating) {
                if (floating) {
                    // When the toolbar is floating, make it a top-level always-on-top window
                    w->setWindowFlag(Qt::WindowStaysOnTopHint, true);
                    w->show(); // Re-polish with new flags
                } else {
                    // When re-docked, clear the always-on-top hint
                    w->setWindowFlag(Qt::WindowStaysOnTopHint, false);
                    w->show(); // Re-apply normal docking behavior
                }
            });
        } else if (auto* dw = qobject_cast<QDockWidget*>(w)) {
            connect(dw, &QDockWidget::topLevelChanged, this, [w](bool floating) {
                if (floating) {
                    // When the dock widget is floating, keep it above MainWindow
                    w->setWindowFlag(Qt::WindowStaysOnTopHint, true);
                    w->show();
                } else {
                    // When docked back into MainWindow, remove the always-on-top hint
                    w->setWindowFlag(Qt::WindowStaysOnTopHint, false);
                    w->show();
                }
            });
        }
    };

    setupFloatingBehavior(m_mainToolBar);
    setupFloatingBehavior(m_topRibbon);
    setupFloatingBehavior(m_rightRibbon);

    setupFloatingBehavior(m_toolboxDock);
    setupFloatingBehavior(m_propertiesDock);
}

void MainWindow::setupStatusBar()
{
    m_statusBar = statusBar();
    if (m_statusBar) {
        m_statusBar->setContentsMargins(8, 0, 8, 0);
        m_statusBar->setAutoFillBackground(true);
        m_statusBar->setPalette(QApplication::palette());
        m_statusBar->setStyleSheet(
            QStringLiteral("QStatusBar { background: palette(window); color: palette(windowText); padding: 0 8px; }")
        );
    }
    
    // Left side - transient status (initially "Starting...", will change to "Ready" when startup completes)
    m_statusLabel = new QLabel(tr("Starting..."), this);
    m_statusLabel->setContentsMargins(6, 0, 6, 0);
    m_statusBar->addWidget(m_statusLabel);
    
    // Right side - permanent widgets (startup timing + debug info)
    m_startupLabel = new QLabel(this);
    m_startupLabel->setContentsMargins(6, 0, 6, 0);
    m_statusBar->addPermanentWidget(m_startupLabel);
    
    m_debugLabel = new QLabel(this);
    m_debugLabel->setContentsMargins(6, 0, 6, 0);
    m_statusBar->addPermanentWidget(m_debugLabel);
    
    // Do not call updateStatusBar() here; the constructor will call it
    // after all UI setup is complete to avoid early updateDebugInfo() on macOS.
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

    auto& s = m_settingsProvider->settings();
    QString language = s.value(PhxKeys::UI_LANGUAGE, QStringLiteral("en")).toString();

    m_currentLocale = QLocale(i18n::localeForLanguage(language));
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
    auto& s = m_settingsProvider->settings();
    
    // Detect language change by comparing current language with last stored value
    const QString currentLang = s.value(PhxKeys::UI_LANGUAGE, QStringLiteral("en")).toString();
    const QString lastLang = s.value(PhxKeys::UI_LAST_LANGUAGE, currentLang).toString();
    const bool languageChanged = (currentLang != lastLang);
    
    const bool firstRun = !s.contains(PhxKeys::UI_FIRST_RUN_COMPLETE);
    if (firstRun) {
        // First launch: apply canonical layout (toolbars/docks in default positions)
        applyCanonicalLayout();
        s.setValue(PhxKeys::UI_FIRST_RUN_COMPLETE, true);
        s.sync();
    } else if (languageChanged) {
        // Language changed: reset to canonical layout to ensure docks remain visible
        applyCanonicalLayout();
    } else {
        // Normal startup: restore saved geometry and window state
        const bool okG = restoreGeometry(s.value(PhxKeys::UI_GEOMETRY).toByteArray());
        const bool okS = restoreState(s.value(PhxKeys::UI_WINDOW_STATE).toByteArray());
        if (!okG || !okS) {
            // Restore failed: fall back to canonical layout
            applyCanonicalLayout();
        } else {
            // Restore succeeded: verify both docks are visible
            // If both are invisible, treat as failed restore (e.g., floating docks lost off-screen)
            const bool toolboxVisible = m_toolboxDock && m_toolboxDock->isVisible();
            const bool propertiesVisible = m_propertiesDock && m_propertiesDock->isVisible();
            
            if (!toolboxVisible && !propertiesVisible) {
                // Both primary docks are effectively lost → treat as failed layout
                qWarning() << "loadSettings: restored layout hides both docks; applying canonical layout";
                applyCanonicalLayout();
            }
        }
    }
    
    // Store current language for next launch (used to detect language changes)
    s.setValue(PhxKeys::UI_LAST_LANGUAGE, currentLang);
    s.sync();
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
    // Only ensure debug info is updated; "Ready" is controlled by updateDebugInfo()
    updateDebugInfo();
}

void MainWindow::updateStatusMessage(const QString& message)
{
    if (m_statusLabel) {
        m_statusLabel->setText(message);
    }
}

void MainWindow::setStartupStartTime(qint64 ms)
{
    m_startupStartTime = ms;
}


void MainWindow::showEvent(QShowEvent* ev)
{
    QMainWindow::showEvent(ev);
    if (!m_firstShowEmitted && isVisible()) {
        m_firstShowEmitted = true;
        emit firstShown();
        // Startup timing will be displayed via m_startupLabel when calculated
    }
}

void MainWindow::updateDebugInfo()
{
    // Bail out if status bar or labels are not yet initialized
    if (!m_statusBar || !m_statusLabel || !m_startupLabel || !m_debugLabel) {
        return;
    }

    const bool needsSample = !m_memorySampleTimer.isValid() || m_memorySampleTimer.elapsed() >= 5000;
    if (needsSample) {
        const double sampleMb = phx::MemoryMonitor::getResidentMemoryMB();
        if (sampleMb >= 0.0) {
            m_lastResidentMemoryMB = sampleMb;
            m_hasResidentSample = true;
        }

        if (m_memorySampleTimer.isValid()) {
            m_memorySampleTimer.restart();
        } else {
            m_memorySampleTimer.start();
        }
    }

    const QLocale locale;
    QString memoryDisplay;
    if (m_hasResidentSample) {
        const auto rounded = static_cast<qlonglong>(std::llround(m_lastResidentMemoryMB));
        memoryDisplay = tr("%1 MB").arg(locale.toString(rounded));
    } else {
        memoryDisplay = tr("N/A");
    }

    const int cpuCount = QThread::idealThreadCount();
    const QString cpuDisplay = locale.toString(cpuCount > 0 ? cpuCount : 1);

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

    const QString langStr = m_currentLocale.name().left(2);

    // Handle startup timing separately in dedicated label
    if (m_startupDuration < 0 && m_startupStartTime > 0) {
        const qint64 readyTime = QDateTime::currentMSecsSinceEpoch();
        m_startupDuration = readyTime - m_startupStartTime;

        // Startup complete - set startup label and show "Ready"
        if (m_startupLabel) {
            m_startupLabel->setText(tr("Startup: %1 ms").arg(m_startupDuration));
        }
        if (m_statusLabel) {
            m_statusLabel->setText(tr("Ready"));
        }
    }
    // After this point, only update m_debugLabel - never touch m_startupLabel or m_statusLabel

    const QString debugText = tr("Memory: %1 | CPUs: %2 | Theme: %3 | Lang: %4")
                              .arg(memoryDisplay)
                              .arg(cpuDisplay)
                              .arg(themeStr)
                              .arg(langStr);

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
        m_preferencesDialog = new PreferencesDialog(this, this);
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
    // Create XYAnalysisWindow as top-level window (no parent)
    // This ensures macOS allows it to appear above MainWindow
    // Tool windows (Qt::Tool) will naturally stay on top of both
    // License checking removed (transport-dependent, Phase 3+)
    auto* win = new XYAnalysisWindow(nullptr);
    
    // Cascade positioning: offset each new window by a small amount
    static int cascadeOffset = 0;
    constexpr int cascadeDelta = 20;
    constexpr int maxCascadeOffset = 100;
    
    const QPoint baseOffset(50, 50);  // Base offset from MainWindow's top-left
    const QPoint mainPos = pos();
    const QPoint cascadePos = mainPos + baseOffset + QPoint(cascadeOffset, cascadeOffset);
    
    win->move(cascadePos);
    
    // Update cascade offset for next window (with wrap-around)
    cascadeOffset += cascadeDelta;
    if (cascadeOffset > maxCascadeOffset) {
        cascadeOffset = 0;  // Wrap back to start
    }
    
    // Generate a simple test dataset: 1000-point sine wave
    std::vector<QPointF> points;
    points.reserve(1000);
    for (int i = 0; i < 1000; ++i) {
        double x = i * 0.01;  // x from 0 to 9.99
        double y = std::sin(x);
        points.emplace_back(x, y);
    }
    
    // Set data on the plot view
    win->plotView()->setData(points);
    win->plotView()->setTitle(tr("XY Sine"));
    
    // Note: setFeature() not called - parameter panel requires transport dependencies (Phase 3+)
    
    // Show and bring to front (tool windows will stay on top)
    win->show();
    win->raise();
    win->activateWindow();
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

void MainWindow::resetLayout()
{
    if (!m_settingsProvider) return;
    
    auto& s = m_settingsProvider->settings();
    s.remove(PhxKeys::UI_GEOMETRY);
    s.remove(PhxKeys::UI_WINDOW_STATE);
    // Remove any panel-specific settings if they exist
    s.remove(QStringLiteral("ui/panels"));
    s.sync();
    
    applyCanonicalLayout();
    
    if (m_statusBar) {
        m_statusBar->showMessage(tr("Layout reset to defaults"), 3000);
    }
}

void MainWindow::promptRestart()
{
    QMessageBox msg(this);
    msg.setWindowTitle(tr("Phoenix Restart Required"));
    msg.setText(tr("Phoenix must restart to apply these changes."));
    msg.setIcon(QMessageBox::Information);
    msg.setModal(true);
    
    QPushButton* restartNow = msg.addButton(tr("Restart Now"), QMessageBox::AcceptRole);
    QPushButton* later = msg.addButton(tr("Restart Later"), QMessageBox::RejectRole);
    msg.setDefaultButton(later);
    
    msg.exec();
    
    if (msg.clickedButton() == restartNow) {
        bool ok = false;
#ifdef Q_OS_MACOS
        ok = restartPhoenixOnMac();
#else
        ok = QProcess::startDetached(QCoreApplication::applicationFilePath(),
                                     QCoreApplication::arguments());
#endif
        if (ok) {
            qApp->quit();
        } else {
            qWarning() << "promptRestart: failed to relaunch Phoenix";
        }
    }
}

#ifdef Q_OS_MACOS
bool MainWindow::restartPhoenixOnMac()
{
    // applicationDirPath() → ".../Phoenix.app/Contents/MacOS"
    QString macOSDir = QCoreApplication::applicationDirPath();
    QDir dir(macOSDir);
    if (!dir.cdUp()) {
        // ".." → Contents
        qWarning() << "restartPhoenixOnMac: failed to cdUp to Contents";
        return false;
    }
    if (!dir.cdUp()) {
        // ".." → Phoenix.app
        qWarning() << "restartPhoenixOnMac: failed to cdUp to .app bundle";
        return false;
    }
    
    const QString appBundlePath = dir.absolutePath(); // ".../Phoenix.app"
    
    // Use 'open -n' to launch a new instance of the app bundle
    QStringList args;
    args << QStringLiteral("-n") << appBundlePath;
    
    const bool ok = QProcess::startDetached(QStringLiteral("/usr/bin/open"), args);
    if (!ok) {
        qWarning() << "restartPhoenixOnMac: QProcess::startDetached failed for" << appBundlePath;
    }
    return ok;
}
#endif

void MainWindow::onThemeRibbonActionTriggered(bool checked)
{
    Q_UNUSED(checked);

    auto* action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }

    const QString logKey = action->property("phx_log_key").toString();
    if (!logKey.isEmpty()) {
        logRibbonAction(logKey);
    }
}

void MainWindow::setLanguage(const QString& language)
{
    const QString normalized = (language == QStringLiteral("de")) ? QStringLiteral("de") : QStringLiteral("en");

    if (m_currentLocale.name().left(2) == normalized) {
        return;
    }

    if (m_settingsProvider) {
        auto& s = m_settingsProvider->settings();
        s.setValue(PhxKeys::UI_LANGUAGE, normalized);
        s.setValue(PhxKeys::UI_LOCALE, i18n::localeForLanguage(normalized));
        s.sync();
    }

    m_currentLocale = QLocale(i18n::localeForLanguage(normalized));
    updateDebugInfo();

    promptRestart();
}

void MainWindow::onThemeChanged()
{
    if (!m_uiInitialized) {
        return;
    }

    // Delay icon refresh until after palette propagation (QApplication::setPalette() is synchronous,
    // but we want to ensure all widgets have received the palette change event)
    QTimer::singleShot(0, this, [this] {
        if (!m_uiInitialized) {
            return;
        }
        IconProvider::clearCache();
        refreshAllIconsForTheme();
        const QSize iconSize = safeIconSizeHint();
        refreshThemeActionIcons(iconSize);

        if (m_themeManager) {
            const auto theme = m_themeManager->currentTheme();
            if (m_lightThemeAction) {
                m_lightThemeAction->setChecked(theme == ThemeManager::Theme::Light);
            }
            if (m_darkThemeAction) {
                m_darkThemeAction->setChecked(theme == ThemeManager::Theme::Dark);
            }
            if (m_systemThemeAction) {
                m_systemThemeAction->setChecked(theme == ThemeManager::Theme::System);
            }
        }

        if (!m_rightRibbon) {
            return;
        }
        applyRibbonPalette(m_rightRibbon);
        
        if (m_statusBar) {
            m_statusBar->setPalette(QApplication::palette());
            m_statusBar->setStyleSheet(
                QStringLiteral("QStatusBar { background: palette(window); color: palette(windowText); padding: 0 8px; }")
            );
            m_statusBar->update();
        }
        
        if (m_toolboxDock && m_toolboxDock->widget()) {
            m_toolboxDock->widget()->update();
        }
        if (m_propertiesDock && m_propertiesDock->widget()) {
            m_propertiesDock->widget()->update();
        }
    });
    // Handle theme changes
    updateDebugInfo();
}

void MainWindow::applyIcons()
{
    // This method is kept for backward compatibility but now delegates to refreshAllIconsForTheme()
    IconProvider::clearCache();
    refreshAllIconsForTheme();
}

void MainWindow::refreshAllIconsForTheme()
{
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
    
    // Toolbar actions (handle both plain QAction and QWidgetAction rows)
    for (QToolBar* tb : findChildren<QToolBar*>()) {
        const int px = tb->iconSize().isValid() ? tb->iconSize().width()
                                                : style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, tb);
        
        for (QAction* a : tb->actions()) {
            const QString key = a->property("phx_icon_key").toString();
            if (key.isEmpty()) continue;  // skip actions without icon key
            
            // Check if this is a QWidgetAction with a row widget
            if (QWidget* w = tb->widgetForAction(a)) {
                if (auto* btn = w->findChild<QToolButton*>()) {
                    // QWidgetAction row: update icon in the button
                    btn->setIcon(QIcon{});  // clear cache
                    btn->setIcon(IconProvider::icon(key, QSize(px, px), tb));
                    btn->setIconSize(tb->iconSize());
                    continue;
                }
            }
            
            // Fallback: plain QAction path
            rebuildAction(a, tb);
        }
        tb->update();  // repaint toolbar shell
    }
}

void MainWindow::refreshThemeActionIcons(const QSize& sizeHint)
{
    if (!m_lightThemeAction || !m_darkThemeAction || !m_systemThemeAction) {
        return;
    }

    QSize effectiveSize = sizeHint;
    if (!effectiveSize.isValid() || effectiveSize.width() <= 0 || effectiveSize.height() <= 0) {
        effectiveSize = safeIconSizeHint();
    }

    auto refreshForMenu = [&](QAction* action, QWidget* host) {
        if (!action) {
            return;
        }
        const QString key = action->property("phx_icon_key").toString();
        if (key.isEmpty()) {
            return;
        }

        if (host) {
            const int px = host->style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, host);
            const QSize size(px, px);
            action->setIcon(IconProvider::icon(key, size, host));
        } else {
            action->setIcon(IconProvider::icon(key, effectiveSize, nullptr));
        }
    };

    if (m_themeMenu) {
        refreshForMenu(m_lightThemeAction, m_themeMenu);
        refreshForMenu(m_darkThemeAction, m_themeMenu);
        refreshForMenu(m_systemThemeAction, m_themeMenu);
    } else {
        // Ensure actions always have icons even if menu isn't ready yet
        refreshForMenu(m_lightThemeAction, nullptr);
        refreshForMenu(m_darkThemeAction, nullptr);
        refreshForMenu(m_systemThemeAction, nullptr);
    }

    for (QAction* action : {m_lightThemeAction, m_darkThemeAction, m_systemThemeAction}) {
        if (!action) {
            continue;
        }
        const QString key = action->property("phx_icon_key").toString();
        if (key.isEmpty()) {
            continue;
        }

        action->setIcon(IconProvider::icon(key, effectiveSize, nullptr));

        if (m_rightRibbon) {
            if (QWidget* widget = m_rightRibbon->widgetForAction(action)) {
                if (auto* button = qobject_cast<QToolButton*>(widget)) {
                    button->setIcon(IconProvider::icon(key, effectiveSize, button));
                    button->setIconSize(effectiveSize);
                }
            }
        }
    }
}

QSize MainWindow::safeIconSizeHint() const
{
    if (const QStyle* st = QApplication::style()) {
        const int px = st->pixelMetric(QStyle::PM_SmallIconSize);
        if (px > 0) {
            return QSize(px, px);
        }
    }
    return QSize(16, 16);
}

void MainWindow::applyRibbonPalette(QToolBar* ribbon /*= nullptr*/)
{
    QToolBar* target = ribbon ? ribbon : m_rightRibbon;
    if (!target) {
        return;
    }

    target->setAutoFillBackground(true);
    target->setAttribute(Qt::WA_StyledBackground, true);
    target->setPalette(QApplication::palette());

    static const char* kSideRibbonQss = R"(
       QToolBar#sideRibbon { background: palette(window); border: none; }
       QToolBar#sideRibbon QToolButton {
         background: transparent; border: none; padding: 0 8px;
         color: palette(windowText); text-align: left;
       }
       QToolBar#sideRibbon QToolButton:hover {
         background: palette(alternateBase);
       }
       QToolBar#sideRibbon QToolButton:checked {
         background: palette(mid);
         color: palette(windowText);
         font-weight: 500;
         border: none;
       }
       QToolBar#sideRibbon QToolButton:focus { outline: 0; border: none; }
    )";

    target->setStyleSheet(QString::fromLatin1(kSideRibbonQss));
    if (auto* st = target->style()) {
        st->unpolish(target);
        st->polish(target);
    }
    target->update();

    if (auto* p = target->parentWidget()) {
        p->setPalette(QApplication::palette());
        p->update();
    }
}

void MainWindow::applyCanonicalLayout()
{
    // Toolbars
    if (m_mainToolBar) {
        removeToolBar(m_mainToolBar);
        addToolBar(Qt::TopToolBarArea, m_mainToolBar);
        m_mainToolBar->show();
    }
    
    if (m_topRibbon) {
        removeToolBar(m_topRibbon);
        addToolBar(Qt::TopToolBarArea, m_topRibbon);
        m_topRibbon->show();
    }
    
    if (m_rightRibbon) {
        removeToolBar(m_rightRibbon);
        addToolBar(Qt::LeftToolBarArea, m_rightRibbon);
        m_rightRibbon->show();
    }
    
    // Docks
    if (m_toolboxDock) {
        removeDockWidget(m_toolboxDock);
        addDockWidget(Qt::LeftDockWidgetArea, m_toolboxDock);
        m_toolboxDock->show();
    }
    
    if (m_propertiesDock) {
        removeDockWidget(m_propertiesDock);
        addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
        m_propertiesDock->show();
    }
    
    // Ensure docks have reasonable default sizes when docked
    const int minH = phx::ui::kPanelMinHeight;
    if (m_toolboxDock) {
        m_toolboxDock->setMinimumHeight(minH);
        m_toolboxDock->setMinimumWidth(phx::ui::kDockMinWidth);
    }
    
    if (m_propertiesDock) {
        m_propertiesDock->setMinimumHeight(minH);
        m_propertiesDock->setMinimumWidth(phx::ui::kDockMinWidth);
    }
    
    // Reapply ribbon theme so visuals match
    if (m_rightRibbon) {
        applyRibbonPalette(m_rightRibbon);
        IconProvider::clearCache();
        refreshThemeActionIcons(safeIconSizeHint());
    }
}

void MainWindow::wireSideRibbonAction(QAction* action, const QString& iconKey, QToolBar* ribbon /*= nullptr*/)
{
    QToolBar* target = ribbon ? ribbon : m_rightRibbon;
    if (!target || !action) {
        return;
    }
    
    if (action->property("phx_icon_key").toString().isEmpty()) {
        action->setProperty("phx_icon_key", iconKey);
    }
    
    target->addAction(action);
    
    if (QWidget* widget = target->widgetForAction(action)) {
        if (auto* button = qobject_cast<QToolButton*>(widget)) {
            button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            button->setAutoRaise(true);
            button->setMinimumWidth(140);
            button->setIconSize(target->iconSize());
        }
    }
}

void MainWindow::retranslateUi()
{
    // Retranslate all UI elements
    setWindowTitle(tr("Phoenix %1 - Optical Design Studio")
                   .arg(QStringLiteral(PHOENIX_VERSION)));
    
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
                   QString(tr("Phoenix\nVersion %1\n\nBuilt with Qt %2\nRunning on %3"))
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

<<<<<<< HEAD
=======
void MainWindow::showLicense()
{
    LicenseDialog* dialog = new LicenseDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void MainWindow::showEchoTestDialog()
{
    if (!m_echoTestDialog) {
        m_echoTestDialog = new EchoTestDialog(this);
        m_echoTestDialog->setAttribute(Qt::WA_DeleteOnClose);
    }
    m_echoTestDialog->exec();
}

void MainWindow::updateWindowMenu()
{
    if (!m_windowMenu) {
        return;
    }
    
    // Remove all window-specific actions (keep "Bring All to Front" and separator)
    // Window actions always carry a non-null data(); static menu entries don't
    QList<QAction*> actions = m_windowMenu->actions();
    for (QAction* action : actions) {
        if (action->data().isValid()) {
            m_windowMenu->removeAction(action);
            delete action;
        }
    }
    
    // Get current windows from manager
    AnalysisWindowManager* mgr = AnalysisWindowManager::instance();
    QList<QMainWindow*> windows = mgr->windows();
    
    // Add action for each window
    int index = 1;
    for (QMainWindow* window : windows) {
        if (!window) {
            continue;
        }
        
        QString title = window->windowTitle();
        if (title.isEmpty()) {
            title = tr("XY Plot #%1").arg(index);
        } else {
            title = tr("%1 (%2)").arg(title).arg(index);
        }
        
        QAction* windowAction = new QAction(title, this);
        windowAction->setData(QVariant::fromValue<QMainWindow*>(window));
        windowAction->setCheckable(false);
        connect(windowAction, &QAction::triggered, this, [this, windowAction]() {
            onWindowMenuActionTriggered(windowAction);
        });
        m_windowMenu->addAction(windowAction);
        
        index++;
    }
}

void MainWindow::onWindowMenuActionTriggered(QAction* action)
{
    QMainWindow* window = action->data().value<QMainWindow*>();
    if (window) {
        window->raise();
        window->activateWindow();
    }
}

void MainWindow::onBringAllToFront()
{
    AnalysisWindowManager* mgr = AnalysisWindowManager::instance();
    QList<QMainWindow*> windows = mgr->windows();
    
    // Raise all windows
    for (QMainWindow* window : windows) {
        if (window && window->isVisible()) {
            window->raise();
        }
    }
    
    // Activate the last window (most recently created)
    if (!windows.isEmpty() && windows.last()) {
        windows.last()->activateWindow();
    }
}

void MainWindow::updateActionLicenseState(QAction* action, const QString& feature)
{
    if (!action) return;
    
    LicenseManager* mgr = LicenseManager::instance();
    LicenseManager::LicenseState state = mgr->currentState();
    
    // If licensing is not configured, allow feature (graceful degradation)
    if (state == LicenseManager::LicenseState::NotConfigured) {
        action->setEnabled(true);
        action->setToolTip(QString()); // Clear any previous tooltip
        return;
    }
    
    // Check if feature is available
    bool hasFeature = mgr->hasFeature(feature);
    
    if (hasFeature) {
        action->setEnabled(true);
        action->setToolTip(QString()); // Clear any previous tooltip
    } else {
        action->setEnabled(false);
        QString tooltip = tr("This feature requires a valid license with the '%1' feature").arg(feature);
        if (state == LicenseManager::LicenseState::Expired) {
            tooltip += tr(" (License expired)");
        } else if (state == LicenseManager::LicenseState::Invalid) {
            tooltip += tr(" (License invalid)");
        } else if (state == LicenseManager::LicenseState::NoLicense) {
            tooltip += tr(" (No license file)");
        }
        action->setToolTip(tooltip);
    }
}

>>>>>>> 6842673 (S4.3-Windowing-Polish-1: Cascade XY analysis windows and add Window menu)
// Telemetry hooks for UI latency logging
void MainWindow::logUIAction(const QString& action, qint64 elapsed)
{
    // Log UI action with timing for <50ms chrome response validation
    qCDebug(phxUI) << "UI Action:" << action << "took" << elapsed << "ms";
    
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
    qCDebug(phxUI) << "Ribbon Action:" << action;
    
    // Connect to the actual action completion
    QTimer::singleShot(0, this, [this, action]() {
        qint64 elapsed = m_actionTimer.elapsed();
        logUIAction(QString("ribbon_%1").arg(action), elapsed);
    });
}
