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
    , m_themeManager(ThemeManager::instance())
    , m_debugTimer(new QTimer(this))
{
    setWindowTitle("Phoenix - Optical Design Studio");
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    // Set application icon
    setWindowIcon(QIcon(":/icons/phoenix-icon.svg"));
    
    // Initialize components (defer translations until after UI is ready)
    setupMenuBar();  // This creates all the actions
    setupToolBar();  // This uses the actions created above
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
    
    m_newAction = new QAction(getIcon("plus", "file-plus"), tr("&New"), this);
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip(tr("Create a new file"));
    connect(m_newAction, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(m_newAction);
    
    m_openAction = new QAction(getIcon("folder-open", "folder-open"), tr("&Open"), this);
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
    
    m_saveAsAction = new QAction(getIcon("floppy-disk", "save"), tr("Save &As"), this);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip(tr("Save the current file with a new name"));
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveAsFile);
    fileMenu->addAction(m_saveAsAction);
    
    fileMenu->addSeparator();
    
    m_preferencesAction = new QAction(getIcon("gear", "settings"), tr("&Preferences..."), this);
    m_preferencesAction->setShortcut(QKeySequence::Preferences);
    m_preferencesAction->setStatusTip(tr("Open preferences dialog"));
    connect(m_preferencesAction, &QAction::triggered, this, &MainWindow::showPreferences);
    fileMenu->addAction(m_preferencesAction);
    
    fileMenu->addSeparator();
    
    m_exitAction = new QAction(getIcon("xmark", "close"), tr("E&xit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the application"));
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    fileMenu->addAction(m_exitAction);
    
    return fileMenu;
}

QMenu* MainWindow::createEditorsMenu()
{
    QMenu* editorsMenu = new QMenu(tr("&Editors"), this);
    
    m_lensInspectorAction = new QAction(getIcon("magnifying-glass", "search"), tr("&Lens Inspector"), this);
    m_lensInspectorAction->setStatusTip(tr("Open lens inspector"));
    connect(m_lensInspectorAction, &QAction::triggered, this, &MainWindow::showLensInspector);
    editorsMenu->addAction(m_lensInspectorAction);
    
    m_systemViewerAction = new QAction(getIcon("eye", "view"), tr("&System Viewer"), this);
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
    
    m_2dPlotAction = new QAction(getIcon("chart-area", "chart"), tr("&2D Plot"), this);
    m_2dPlotAction->setStatusTip(tr("Open 2D plot analysis"));
    connect(m_2dPlotAction, &QAction::triggered, this, &MainWindow::show2DPlot);
    analysisMenu->addAction(m_2dPlotAction);
    
    return analysisMenu;
}

QMenu* MainWindow::createToolsMenu()
{
    QMenu* toolsMenu = new QMenu(tr("&Tools"), this);
    
    // Add tools actions here in the future
    QAction* iconGalleryAction = new QAction(tr("&Icon Gallery"), this);
    iconGalleryAction->setStatusTip(tr("Open icon gallery"));
    connect(iconGalleryAction, &QAction::triggered, this, &MainWindow::showIconGallery);
    toolsMenu->addAction(iconGalleryAction);
    
    toolsMenu->addSeparator();
    
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
    connect(englishAction, &QAction::triggered, [this]() { setLanguage("en"); });
    languageMenu->addAction(englishAction);
    
    QAction* germanAction = new QAction(tr("&German"), this);
    connect(germanAction, &QAction::triggered, [this]() { setLanguage("de"); });
    languageMenu->addAction(germanAction);
    
    QAction* frenchAction = new QAction(tr("&French"), this);
    connect(frenchAction, &QAction::triggered, [this]() { setLanguage("fr"); });
    languageMenu->addAction(frenchAction);
    
    QAction* spanishAction = new QAction(tr("&Spanish"), this);
    connect(spanishAction, &QAction::triggered, [this]() { setLanguage("es"); });
    languageMenu->addAction(spanishAction);
    
    QAction* chineseTradAction = new QAction(tr("Chinese (&Traditional)"), this);
    connect(chineseTradAction, &QAction::triggered, [this]() { setLanguage("zh_TW"); });
    languageMenu->addAction(chineseTradAction);
    
    QAction* chineseSimpAction = new QAction(tr("Chinese (&Simplified)"), this);
    connect(chineseSimpAction, &QAction::triggered, [this]() { setLanguage("zh_CN"); });
    languageMenu->addAction(chineseSimpAction);
    
    QAction* koreanAction = new QAction(tr("&Korean"), this);
    connect(koreanAction, &QAction::triggered, [this]() { setLanguage("ko"); });
    languageMenu->addAction(koreanAction);
    
    QAction* japaneseAction = new QAction(tr("&Japanese"), this);
    connect(japaneseAction, &QAction::triggered, [this]() { setLanguage("ja"); });
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
    qDebug() << "Toolbar created and added:" << m_mainToolBar->isVisible();
}

QToolBar* MainWindow::createMainToolBar()
{
    QToolBar* toolBar = new QToolBar(tr("Main Toolbar"), this);
    toolBar->setObjectName("mainToolBar");
    toolBar->setMovable(true);
    toolBar->setFloatable(true);
    
    qDebug() << "Creating toolbar with actions:" << m_newAction << m_openAction << m_saveAction;
    
    toolBar->addAction(m_newAction);
    toolBar->addAction(m_openAction);
    toolBar->addAction(m_saveAction);
    toolBar->addSeparator();
    toolBar->addAction(m_preferencesAction);
    
    return toolBar;
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
    // Connect theme manager signals - simplified for now
    // TODO: Implement proper signal connections when ThemeManager signals are working
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
        m_themeManager->setTheme(ThemeManager::Theme::Light);
    } else if (theme == "dark") {
        m_darkThemeAction->setChecked(true);
        m_themeManager->setTheme(ThemeManager::Theme::Dark);
    } else {
        m_systemThemeAction->setChecked(true);
        m_themeManager->setTheme(ThemeManager::Theme::System);
    }
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
    
    // Get current language
    QString langStr = m_currentLocale.name().left(2);
    
    // Update debug label
    QString debugText = tr("Memory: %1MB | Threads: %2 | Theme: %3 | Lang: %4")
                       .arg(memoryMB)
                       .arg(threadCount)
                       .arg(themeStr)
                       .arg(langStr);
    
    m_debugLabel->setText(debugText);
}

QIcon MainWindow::getIcon(const QString& name, const QString& fallback)
{
    // Use IconProvider to get Font Awesome icons
    // For now, return a simple icon - this will be enhanced with the icon system
    return QIcon(); // Placeholder - will be implemented with IconProvider
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
    m_themeManager->setTheme(ThemeManager::Theme::Light);
    m_settings->setValue("theme", "light");
}

void MainWindow::setDarkTheme()
{
    m_themeManager->setTheme(ThemeManager::Theme::Dark);
    m_settings->setValue("theme", "dark");
}

void MainWindow::setSystemTheme()
{
    m_themeManager->setTheme(ThemeManager::Theme::System);
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

void MainWindow::showIconGallery()
{
    QMessageBox::information(this, tr("Icon Gallery"), tr("This feature is not yet implemented."));
}

void MainWindow::showHelp()
{
    QMessageBox::information(this, tr("Help"), tr("This feature is not yet implemented."));
}
