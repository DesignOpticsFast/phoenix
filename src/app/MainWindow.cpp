#include "MainWindow.hpp"
#include "../ui/EnvironmentPrefs.hpp"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QActionGroup>
#include <QTimer>
#include <QElapsedTimer>
#include <QSettings>
#include <QLocale>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , menuBar_(nullptr)
    , topRibbon_(nullptr)
    , rightRibbon_(nullptr)
    , statusBar_(nullptr)
    , statusSlot1_(nullptr)
    , statusSlot2_(nullptr)
    , statusSlot3_(nullptr)
    , statusSlot4_(nullptr)
    , statusSlot5_(nullptr)
    , prefsDialog_(nullptr)
    , settings_(new QSettings("Phoenix", "Sprint4", this))
{
    // High DPI scaling is enabled by default in Qt 6
    
    // Set window properties
    setWindowTitle("Phoenix IDE - Sprint 4");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    // Setup UI components
    setupMenuBar();
    setupRibbons();
    setupStatusBar();
    setupConnections();
    setupTelemetry();
    
    // Load settings
    loadSettings();
    
    // Show initial status
    updateStatusMessage("Phoenix IDE ready");
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
    menuBar_ = menuBar();
    
    // Create menus
    QMenu* fileMenu = createFileMenu();
    QMenu* editorsMenu = createEditorsMenu();
    QMenu* analysisMenu = createAnalysisMenu();
    QMenu* toolsMenu = createToolsMenu();
    QMenu* helpMenu = createHelpMenu();
    
    // Add to menu bar
    menuBar_->addMenu(fileMenu);
    menuBar_->addMenu(editorsMenu);
    menuBar_->addMenu(analysisMenu);
    menuBar_->addMenu(toolsMenu);
    menuBar_->addMenu(helpMenu);
}

QMenu* MainWindow::createFileMenu()
{
    QMenu* menu = new QMenu("&File", this);
    
    // Create actions
    newAction_ = new QAction("&New", this);
    newAction_->setShortcut(QKeySequence::New);
    newAction_->setStatusTip("Create a new file");
    connect(newAction_, &QAction::triggered, this, &MainWindow::newFile);
    
    openAction_ = new QAction("&Open...", this);
    openAction_->setShortcut(QKeySequence::Open);
    openAction_->setStatusTip("Open an existing file");
    connect(openAction_, &QAction::triggered, this, &MainWindow::openFile);
    
    saveAction_ = new QAction("&Save", this);
    saveAction_->setShortcut(QKeySequence::Save);
    saveAction_->setStatusTip("Save the current file");
    connect(saveAction_, &QAction::triggered, this, &MainWindow::saveFile);
    
    saveAsAction_ = new QAction("Save &As...", this);
    saveAsAction_->setShortcut(QKeySequence::SaveAs);
    saveAsAction_->setStatusTip("Save the current file with a new name");
    connect(saveAsAction_, &QAction::triggered, this, &MainWindow::saveAsFile);
    
    menu->addSeparator();
    
    exitAction_ = new QAction("E&xit", this);
    exitAction_->setShortcut(QKeySequence::Quit);
    exitAction_->setStatusTip("Exit the application");
    connect(exitAction_, &QAction::triggered, this, &MainWindow::exitApplication);
    
    // Add actions to menu
    menu->addAction(newAction_);
    menu->addAction(openAction_);
    menu->addSeparator();
    menu->addAction(saveAction_);
    menu->addAction(saveAsAction_);
    menu->addSeparator();
    menu->addAction(exitAction_);
    
    return menu;
}

QMenu* MainWindow::createEditorsMenu()
{
    QMenu* menu = new QMenu("&Editors", this);
    
    QAction* showEditorsAction = new QAction("&Show Editors", this);
    showEditorsAction->setStatusTip("Show all editors");
    connect(showEditorsAction, &QAction::triggered, this, &MainWindow::showEditors);
    
    QAction* hideEditorsAction = new QAction("&Hide Editors", this);
    hideEditorsAction->setStatusTip("Hide all editors");
    connect(hideEditorsAction, &QAction::triggered, this, &MainWindow::hideEditors);
    
    menu->addAction(showEditorsAction);
    menu->addAction(hideEditorsAction);
    
    return menu;
}

QMenu* MainWindow::createAnalysisMenu()
{
    QMenu* menu = new QMenu("&Analysis", this);
    
    QAction* newAnalysisAction = new QAction("&New Analysis", this);
    newAnalysisAction->setStatusTip("Create a new analysis");
    connect(newAnalysisAction, &QAction::triggered, this, &MainWindow::newAnalysis);
    
    QAction* showManagerAction = new QAction("&Analysis Manager", this);
    showManagerAction->setStatusTip("Show analysis manager");
    connect(showManagerAction, &QAction::triggered, this, &MainWindow::showAnalysisManager);
    
    menu->addAction(newAnalysisAction);
    menu->addAction(showManagerAction);
    
    return menu;
}

QMenu* MainWindow::createToolsMenu()
{
    QMenu* menu = new QMenu("&Tools", this);
    
    preferencesAction_ = new QAction("&Preferences...", this);
    preferencesAction_->setStatusTip("Open preferences dialog");
    connect(preferencesAction_, &QAction::triggered, this, &MainWindow::showPreferences);
    
    QAction* telemetryAction = new QAction("&Telemetry", this);
    telemetryAction->setStatusTip("Show telemetry data");
    connect(telemetryAction, &QAction::triggered, this, &MainWindow::showTelemetry);
    
    menu->addAction(preferencesAction_);
    menu->addAction(telemetryAction);
    
    return menu;
}

QMenu* MainWindow::createHelpMenu()
{
    QMenu* menu = new QMenu("&Help", this);
    
    aboutAction_ = new QAction("&About", this);
    aboutAction_->setStatusTip("About Phoenix IDE");
    connect(aboutAction_, &QAction::triggered, this, &MainWindow::showAbout);
    
    QAction* docsAction = new QAction("&Documentation", this);
    docsAction->setStatusTip("Open documentation");
    connect(docsAction, &QAction::triggered, this, &MainWindow::showDocumentation);
    
    menu->addAction(aboutAction_);
    menu->addAction(docsAction);
    
    return menu;
}

void MainWindow::setupRibbons()
{
    // Top ribbon (horizontal)
    topRibbon_ = createTopRibbon();
    addToolBar(Qt::TopToolBarArea, topRibbon_);
    
    // Right ribbon (vertical)
    rightRibbon_ = createRightRibbon();
    addToolBar(Qt::RightToolBarArea, rightRibbon_);
}

QToolBar* MainWindow::createTopRibbon()
{
    QToolBar* ribbon = new QToolBar("Top Ribbon", this);
    ribbon->setMovable(true);
    ribbon->setFloatable(true);
    ribbon->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    
    // Add placeholder actions with FontAwesome-style icons
    QAction* newAction = ribbon->addAction("📄", this, &MainWindow::newFile);
    newAction->setToolTip("New File");
    
    QAction* openAction = ribbon->addAction("📂", this, &MainWindow::openFile);
    openAction->setToolTip("Open File");
    
    QAction* saveAction = ribbon->addAction("💾", this, &MainWindow::saveFile);
    saveAction->setToolTip("Save File");
    
    ribbon->addSeparator();
    
    QAction* analysisAction = ribbon->addAction("📊", this, &MainWindow::newAnalysis);
    analysisAction->setToolTip("New Analysis");
    
    QAction* toolsAction = ribbon->addAction("⚙️", this, &MainWindow::showPreferences);
    toolsAction->setToolTip("Preferences");
    
    return ribbon;
}

QToolBar* MainWindow::createRightRibbon()
{
    QToolBar* ribbon = new QToolBar("Right Ribbon", this);
    ribbon->setMovable(true);
    ribbon->setFloatable(true);
    ribbon->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);
    ribbon->setOrientation(Qt::Vertical);
    
    // Add vertical actions
    QAction* editorsAction = ribbon->addAction("📝", this, &MainWindow::showEditors);
    editorsAction->setToolTip("Show Editors");
    
    QAction* analysisAction = ribbon->addAction("🔬", this, &MainWindow::showAnalysisManager);
    analysisAction->setToolTip("Analysis Manager");
    
    QAction* helpAction = ribbon->addAction("❓", this, &MainWindow::showAbout);
    helpAction->setToolTip("Help");
    
    return ribbon;
}

void MainWindow::setupStatusBar()
{
    statusBar_ = statusBar();
    
    // Create status slots (right half subdivided into 5 slots)
    statusSlot1_ = new QLabel("Ready");
    statusSlot2_ = new QLabel("");
    statusSlot3_ = new QLabel("");
    statusSlot4_ = new QLabel("");
    statusSlot5_ = new QLabel("");
    
    // Add slots to status bar
    statusBar_->addPermanentWidget(statusSlot1_);
    statusBar_->addPermanentWidget(statusSlot2_);
    statusBar_->addPermanentWidget(statusSlot3_);
    statusBar_->addPermanentWidget(statusSlot4_);
    statusBar_->addPermanentWidget(statusSlot5_);
    
    // Set initial status
    updateStatusMessage("Phoenix IDE ready");
}

void MainWindow::setupConnections()
{
    // Connect telemetry timer
    connect(&telemetryTimer_, &QTimer::timeout, this, &MainWindow::updateStatusSlots);
    telemetryTimer_.start(1000); // Update every second
}

void MainWindow::setupTelemetry()
{
    // Create telemetry directory
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    telemetryPath_ = homeDir + "/.phoenix/telemetry/";
    QDir().mkpath(telemetryPath_);
}

void MainWindow::loadSettings()
{
    // Load locale settings
    QString localeName = settings_->value("locale", QLocale::system().name()).toString();
    currentLocale_ = QLocale(localeName);
    
    // Load window geometry
    restoreGeometry(settings_->value("geometry").toByteArray());
    restoreState(settings_->value("windowState").toByteArray());
}

void MainWindow::saveSettings()
{
    // Save locale settings
    settings_->setValue("locale", currentLocale_.name());
    
    // Save window geometry
    settings_->setValue("geometry", saveGeometry());
    settings_->setValue("windowState", saveState());
}

void MainWindow::updateStatusMessage(const QString& message)
{
    statusBar_->showMessage(message, 3000); // Show for 3 seconds
}

void MainWindow::updateStatusSlots()
{
    // Update status slots with current information
    statusSlot1_->setText("Ready");
    statusSlot2_->setText(QString("Locale: %1").arg(currentLocale_.name()));
    statusSlot3_->setText(QString("Time: %1").arg(QTime::currentTime().toString()));
    statusSlot4_->setText("Phoenix Sprint 4");
    statusSlot5_->setText("Qt 6.9.3");
}

void MainWindow::logUIAction(const QString& action, qint64 latency)
{
    // Log UI action to telemetry CSV
    QString csvPath = telemetryPath_ + "ui_actions.csv";
    QFile file(csvPath);
    
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&file);
        if (file.size() == 0) {
            // Write header if new file
            stream << "timestamp,action,latency_ms\n";
        }
        stream << QDateTime::currentDateTime().toString(Qt::ISODate) 
               << "," << action << "," << latency << "\n";
    }
}

// File menu actions
void MainWindow::newFile()
{
    actionTimer_.start();
    updateStatusMessage("Creating new file...");
    // TODO: Implement new file creation
    logUIAction("new_file", actionTimer_.elapsed());
}

void MainWindow::openFile()
{
    actionTimer_.start();
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "All Files (*)");
    if (!fileName.isEmpty()) {
        updateStatusMessage(QString("Opened: %1").arg(fileName));
        // TODO: Implement file opening
    }
    logUIAction("open_file", actionTimer_.elapsed());
}

void MainWindow::saveFile()
{
    actionTimer_.start();
    updateStatusMessage("Saving file...");
    // TODO: Implement file saving
    logUIAction("save_file", actionTimer_.elapsed());
}

void MainWindow::saveAsFile()
{
    actionTimer_.start();
    QString fileName = QFileDialog::getSaveFileName(this, "Save As", "", "All Files (*)");
    if (!fileName.isEmpty()) {
        updateStatusMessage(QString("Saved as: %1").arg(fileName));
        // TODO: Implement save as
    }
    logUIAction("save_as_file", actionTimer_.elapsed());
}

void MainWindow::exitApplication()
{
    actionTimer_.start();
    close();
    logUIAction("exit_application", actionTimer_.elapsed());
}

// Editors menu actions
void MainWindow::showEditors()
{
    actionTimer_.start();
    updateStatusMessage("Showing editors...");
    // TODO: Implement show editors
    logUIAction("show_editors", actionTimer_.elapsed());
}

void MainWindow::hideEditors()
{
    actionTimer_.start();
    updateStatusMessage("Hiding editors...");
    // TODO: Implement hide editors
    logUIAction("hide_editors", actionTimer_.elapsed());
}

// Analysis menu actions
void MainWindow::newAnalysis()
{
    actionTimer_.start();
    updateStatusMessage("Creating new analysis...");
    // TODO: Implement new analysis
    logUIAction("new_analysis", actionTimer_.elapsed());
}

void MainWindow::showAnalysisManager()
{
    actionTimer_.start();
    updateStatusMessage("Opening analysis manager...");
    // TODO: Implement analysis manager
    logUIAction("show_analysis_manager", actionTimer_.elapsed());
}

// Tools menu actions
void MainWindow::showPreferences()
{
    actionTimer_.start();
    if (!prefsDialog_) {
        prefsDialog_ = std::make_unique<EnvironmentPrefs>(this);
    }
    prefsDialog_->show();
    prefsDialog_->raise();
    prefsDialog_->activateWindow();
    logUIAction("show_preferences", actionTimer_.elapsed());
}

void MainWindow::showTelemetry()
{
    actionTimer_.start();
    updateStatusMessage("Opening telemetry viewer...");
    // TODO: Implement telemetry viewer
    logUIAction("show_telemetry", actionTimer_.elapsed());
}

// Help menu actions
void MainWindow::showAbout()
{
    actionTimer_.start();
    QMessageBox::about(this, "About Phoenix IDE",
                      "Phoenix IDE - Sprint 4\n\n"
                      "A responsive, crash-resistant, multilingual IDE shell\n"
                      "Built with Qt 6.9.3 and C++17\n\n"
                      "© 2025 DesignOpticsFast");
    logUIAction("show_about", actionTimer_.elapsed());
}

void MainWindow::showDocumentation()
{
    actionTimer_.start();
    updateStatusMessage("Opening documentation...");
    // TODO: Implement documentation viewer
    logUIAction("show_documentation", actionTimer_.elapsed());
}

// Ribbon actions
void MainWindow::onRibbonAction()
{
    actionTimer_.start();
    updateStatusMessage("Ribbon action triggered");
    logUIAction("ribbon_action", actionTimer_.elapsed());
}

#include "MainWindow.moc"
