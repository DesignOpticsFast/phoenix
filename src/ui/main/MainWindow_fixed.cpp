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
#include <QCloseEvent>
#include <QSettings>
#include <QLocale>
#include <QTranslator>
#include <QActionGroup>
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_settings(new QSettings("Phoenix", "Phoenix", this))
    , m_translator(new QTranslator(this))
    , m_themeManager(ThemeManager::instance())
    , m_debugTimer(new QTimer(this))
{
    setWindowTitle("Phoenix - Optical Design Studio");
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    // Initialize components (use plain strings initially)
    setupMenuBar();
    setupToolBar();
    setupDockWidgets();
    setupStatusBar();
    setupConnections();
    setupTheme();
    loadSettings();
    
    // Setup translations after UI is ready
    setupTranslations();
    
    // Start debug info updates
    m_debugTimer->setInterval(1000); // Update every second
    connect(m_debugTimer, &QTimer::timeout, this, &MainWindow::updateDebugInfo);
    m_debugTimer->start();
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}

void MainWindow::setupMenuBar()
{
    m_menuBar = new QMenuBar(this);
    setMenuBar(m_menuBar);
    
    // File menu
    QMenu* fileMenu = createFileMenu();
    m_menuBar->addMenu(fileMenu);
    
    // Editors menu
    QMenu* editorsMenu = createEditorsMenu();
    m_menuBar->addMenu(editorsMenu);
    
    // Analysis menu
    QMenu* analysisMenu = createAnalysisMenu();
    m_menuBar->addMenu(analysisMenu);
    
    // View menu
    QMenu* viewMenu = createViewMenu();
    m_menuBar->addMenu(viewMenu);
}

QMenu* MainWindow::createFileMenu()
{
    QMenu* fileMenu = new QMenu("&File", this);
    
    m_newAction = new QAction(getIcon("plus", "file-plus"), "&New", this);
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip("Create a new file");
    connect(m_newAction, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(m_newAction);
    
    m_openAction = new QAction(getIcon("folder-open", "folder-open"), "&Open", this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip("Open an existing file");
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(m_openAction);
    
    fileMenu->addSeparator();
    
    m_saveAction = new QAction(getIcon("floppy-disk", "save"), "&Save", this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip("Save the current file");
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    fileMenu->addAction(m_saveAction);
    
    m_saveAsAction = new QAction(getIcon("floppy-disk", "save"), "Save &As", this);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip("Save the current file with a new name");
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveAsFile);
    fileMenu->addAction(m_saveAsAction);
    
    fileMenu->addSeparator();
    
    m_preferencesAction = new QAction(getIcon("gear", "settings"), "&Preferences...", this);
    m_preferencesAction->setShortcut(QKeySequence::Preferences);
    m_preferencesAction->setStatusTip("Open preferences dialog");
    connect(m_preferencesAction, &QAction::triggered, this, &MainWindow::showPreferences);
    fileMenu->addAction(m_preferencesAction);
    
    fileMenu->addSeparator();
    
    m_exitAction = new QAction(getIcon("xmark", "close"), "E&xit", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip("Exit the application");
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    fileMenu->addAction(m_exitAction);
    
    return fileMenu;
}

QMenu* MainWindow::createEditorsMenu()
{
    QMenu* editorsMenu = new QMenu("&Editors", this);
    
    m_lensInspectorAction = new QAction(getIcon("magnifying-glass", "search"), "&Lens Inspector", this);
    m_lensInspectorAction->setStatusTip("Open lens inspector");
    connect(m_lensInspectorAction, &QAction::triggered, this, &MainWindow::showLensInspector);
    editorsMenu->addAction(m_lensInspectorAction);
    
    m_systemViewerAction = new QAction(getIcon("eye", "view"), "&System Viewer", this);
    m_systemViewerAction->setStatusTip("Open system viewer");
    connect(m_systemViewerAction, &QAction::triggered, this, &MainWindow::showSystemViewer);
    editorsMenu->addAction(m_systemViewerAction);
    
    return editorsMenu;
}

QMenu* MainWindow::createAnalysisMenu()
{
    QMenu* analysisMenu = new QMenu("&Analysis", this);
    
    m_xyPlotAction = new QAction(getIcon("chart-line", "chart"), "&XY Plot", this);
    m_xyPlotAction->setStatusTip("Open XY plot analysis");
    connect(m_xyPlotAction, &QAction::triggered, this, &MainWindow::showXYPlot);
    analysisMenu->addAction(m_xyPlotAction);
    
    m_2dPlotAction = new QAction(getIcon("chart-area", "chart"), "&2D Plot", this);
    m_2dPlotAction->setStatusTip("Open 2D plot analysis");
    connect(m_2dPlotAction, &QAction::triggered, this, &MainWindow::show2DPlot);
    analysisMenu->addAction(m_2dPlotAction);
    
    return analysisMenu;
}

QMenu* MainWindow::createViewMenu()
{
    QMenu* viewMenu = new QMenu("&View", this);
    
    // Theme submenu
    QMenu* themeMenu = new QMenu("&Theme", this);
    m_themeGroup = new QActionGroup(this);
    
    m_lightThemeAction = new QAction("&Light", this);
    m_lightThemeAction->setCheckable(true);
    m_lightThemeAction->setActionGroup(m_themeGroup);
    connect(m_lightThemeAction, &QAction::triggered, this, &MainWindow::setLightTheme);
    themeMenu->addAction(m_lightThemeAction);
    
    m_darkThemeAction = new QAction("&Dark", this);
    m_darkThemeAction->setCheckable(true);
    m_darkThemeAction->setActionGroup(m_themeGroup);
    connect(m_darkThemeAction, &QAction::triggered, this, &MainWindow::setDarkTheme);
    themeMenu->addAction(m_darkThemeAction);
    
    m_systemThemeAction = new QAction("&System", this);
    m_systemThemeAction->setCheckable(true);
    m_systemThemeAction->setActionGroup(m_themeGroup);
    connect(m_systemThemeAction, &QAction::triggered, this, &MainWindow::setSystemTheme);
    themeMenu->addAction(m_systemThemeAction);
    
    viewMenu->addMenu(themeMenu);
    
    // Language submenu
    QMenu* languageMenu = new QMenu("&Language", this);
    
    QAction* englishAction = new QAction("&English", this);
    connect(englishAction, &QAction::triggered, [this]() { setLanguage("en"); });
    languageMenu->addAction(englishAction);
    
    QAction* germanAction = new QAction("&German", this);
    connect(germanAction, &QAction::triggered, [this]() { setLanguage("de"); });
    languageMenu->addAction(germanAction);
    
    QAction* frenchAction = new QAction("&French", this);
    connect(frenchAction, &QAction::triggered, [this]() { setLanguage("fr"); });
    languageMenu->addAction(frenchAction);
    
    QAction* spanishAction = new QAction("&Spanish", this);
    connect(spanishAction, &QAction::triggered, [this]() { setLanguage("es"); });
    languageMenu->addAction(spanishAction);
    
    QAction* chineseTradAction = new QAction("Chinese (&Traditional)", this);
    connect(chineseTradAction, &QAction::triggered, [this]() { setLanguage("zh_TW"); });
    languageMenu->addAction(chineseTradAction);
    
    QAction* chineseSimpAction = new QAction("Chinese (&Simplified)", this);
    connect(chineseSimpAction, &QAction::triggered, [this]() { setLanguage("zh_CN"); });
    languageMenu->addAction(chineseSimpAction);
    
    QAction* koreanAction = new QAction("&Korean", this);
    connect(koreanAction, &QAction::triggered, [this]() { setLanguage("ko"); });
    languageMenu->addAction(koreanAction);
    
    QAction* japaneseAction = new QAction("&Japanese", this);
    connect(japaneseAction, &QAction::triggered, [this]() { setLanguage("ja"); });
    languageMenu->addAction(japaneseAction);
    
    viewMenu->addMenu(languageMenu);
    
    return viewMenu;
}

QToolBar* MainWindow::createMainToolBar()
{
    QToolBar* toolBar = new QToolBar("Main Toolbar", this);
    toolBar->setMovable(true);
    toolBar->setFloatable(true);
    
    toolBar->addAction(m_newAction);
    toolBar->addAction(m_openAction);
    toolBar->addSeparator();
    toolBar->addAction(m_saveAction);
    toolBar->addAction(m_saveAsAction);
    toolBar->addSeparator();
    toolBar->addAction(m_preferencesAction);
    
    return toolBar;
}

void MainWindow::setupToolBar()
{
    QToolBar* toolBar = createMainToolBar();
    addToolBar(toolBar);
}

void MainWindow::setupDockWidgets()
{
    // Toolbox dock (left)
    m_toolboxDock = new QDockWidget("Toolbox", this);
    m_toolboxDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_toolboxDock->setMinimumWidth(200);
    m_toolboxDock->setMaximumWidth(400);
    
    QLabel* toolboxLabel = new QLabel("Toolbox Content\n(Coming Soon)", m_toolboxDock);
    toolboxLabel->setAlignment(Qt::AlignCenter);
    m_toolboxDock->setWidget(toolboxLabel);
    addDockWidget(Qt::LeftDockWidgetArea, m_toolboxDock);
    
    // Properties dock (right)
    m_propertiesDock = new QDockWidget("Properties", this);
    m_propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_propertiesDock->setMinimumWidth(200);
    m_propertiesDock->setMaximumWidth(400);
    
    QLabel* propertiesLabel = new QLabel("Properties Content\n(Coming Soon)", m_propertiesDock);
    propertiesLabel->setAlignment(Qt::AlignCenter);
    m_propertiesDock->setWidget(propertiesLabel);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
}

void MainWindow::setupStatusBar()
{
    m_statusBar = new QStatusBar(this);
    setStatusBar(m_statusBar);
    
    // Left side - status message
    m_statusLabel = new QLabel("Ready");
    m_statusBar->addWidget(m_statusLabel);
    
    // Right side - debug info
    m_debugLabel = new QLabel("Debug Info");
    m_statusBar->addPermanentWidget(m_debugLabel);
    
    updateStatusBar();
}

void MainWindow::setupConnections()
{
    // Connect theme manager signals
    connect(m_themeManager, &ThemeManager::themeChanged, this, &MainWindow::onThemeChanged);
}

void MainWindow::setupTranslations()
{
    // Load current language from settings
    QString language = m_settings->value("language", "en").toString();
    setLanguage(language);
}

void MainWindow::setupTheme()
{
    // Apply current theme
    m_themeManager->applyTheme(m_themeManager->currentTheme());
    
    // Update theme action states
    switch (m_themeManager->currentTheme()) {
        case ThemeManager::Theme::Light:
            m_lightThemeAction->setChecked(true);
            break;
        case ThemeManager::Theme::Dark:
            m_darkThemeAction->setChecked(true);
            break;
        case ThemeManager::Theme::System:
            m_systemThemeAction->setChecked(true);
            break;
    }
}

void MainWindow::loadSettings()
{
    // Load window geometry and state
    restoreGeometry(m_settings->value("geometry").toByteArray());
    restoreState(m_settings->value("windowState").toByteArray());
    
    // Load theme
    QString themeStr = m_settings->value("theme", "system").toString();
    if (themeStr == "light") {
        m_themeManager->setTheme(ThemeManager::Theme::Light);
    } else if (themeStr == "dark") {
        m_themeManager->setTheme(ThemeManager::Theme::Dark);
    } else {
        m_themeManager->setTheme(ThemeManager::Theme::System);
    }
}

void MainWindow::saveSettings()
{
    // Save window geometry and state
    m_settings->setValue("geometry", saveGeometry());
    m_settings->setValue("windowState", saveState());
    
    // Save theme
    QString themeStr;
    switch (m_themeManager->currentTheme()) {
        case ThemeManager::Theme::Light:
            themeStr = "light";
            break;
        case ThemeManager::Theme::Dark:
            themeStr = "dark";
            break;
        case ThemeManager::Theme::System:
            themeStr = "system";
            break;
    }
    m_settings->setValue("theme", themeStr);
}

void MainWindow::updateStatusBar()
{
    updateStatusMessage("Ready");
    updateDebugInfo();
}

void MainWindow::updateStatusMessage(const QString& message)
{
    m_statusLabel->setText(message);
}

void MainWindow::updateDebugInfo()
{
    // Get memory usage (simplified for macOS)
    qint64 memoryMB = 0;
    QProcess process;
    process.start("ps", QStringList() << "-o" << "rss=" << "-p" << QString::number(QCoreApplication::applicationPid()));
    process.waitForFinished();
    QString output = process.readAllStandardOutput().trimmed();
    if (!output.isEmpty()) {
        memoryMB = output.toLongLong() / 1024; // Convert KB to MB
    }
    
    // Get thread count
    int threadCount = QThread::idealThreadCount();
    
    // Get current theme
    QString themeStr;
    switch (m_themeManager->currentTheme()) {
        case ThemeManager::Theme::Light:
            themeStr = "Light";
            break;
        case ThemeManager::Theme::Dark:
            themeStr = "Dark";
            break;
        case ThemeManager::Theme::System:
            themeStr = "System";
            break;
    }
    
    // Get current language
    QString language = m_settings->value("language", "en").toString();
    
    // Update debug label
    QString debugText = QString("Memory: %1MB | Threads: %2 | Theme: %3 | Lang: %4")
                       .arg(memoryMB)
                       .arg(threadCount)
                       .arg(themeStr)
                       .arg(language);
    m_debugLabel->setText(debugText);
}

void MainWindow::newFile()
{
    QMessageBox::information(this, "New File", "This feature is not yet implemented.");
}

void MainWindow::openFile()
{
    QMessageBox::information(this, "Open File", "This feature is not yet implemented.");
}

void MainWindow::saveFile()
{
    QMessageBox::information(this, "Save File", "This feature is not yet implemented.");
}

void MainWindow::saveAsFile()
{
    QMessageBox::information(this, "Save As File", "This feature is not yet implemented.");
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

void MainWindow::showLensInspector()
{
    QMessageBox::information(this, "Lens Inspector", "This feature is not yet implemented.");
}

void MainWindow::showSystemViewer()
{
    QMessageBox::information(this, "System Viewer", "This feature is not yet implemented.");
}

void MainWindow::showXYPlot()
{
    QMessageBox::information(this, "XY Plot", "This feature is not yet implemented.");
}

void MainWindow::show2DPlot()
{
    QMessageBox::information(this, "2D Plot", "This feature is not yet implemented.");
}

void MainWindow::setLightTheme()
{
    m_themeManager->setTheme(ThemeManager::Theme::Light);
}

void MainWindow::setDarkTheme()
{
    m_themeManager->setTheme(ThemeManager::Theme::Dark);
}

void MainWindow::setSystemTheme()
{
    m_themeManager->setTheme(ThemeManager::Theme::System);
}

void MainWindow::setLanguage(const QString& language)
{
    // Remove old translator
    if (m_translator) {
        QApplication::removeTranslator(m_translator);
    }
    
    // Load new translator
    if (m_translator->load(QString("phoenix_%1").arg(language), ":/translations/")) {
        QApplication::installTranslator(m_translator);
        m_settings->setValue("language", language);
        retranslateUi();
    }
}

void MainWindow::onThemeChanged()
{
    updateDebugInfo();
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

QIcon MainWindow::getIcon(const QString& name, const QString& fallback)
{
    return IconProvider::icon(name, IconStyle::SharpSolid, 16, m_themeManager->isDarkMode());
}
