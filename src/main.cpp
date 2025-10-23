#include "ui/main/MainWindow.h"
#include "ui/splash/PhoenixSplashScreen.h"
#include "ui/icons/IconBootstrap.h"
#include "ui/icons/IconProvider.h"
#include "ui/icons/LogCategories.h"
#include <QApplication>
#include <QTimer>
#include <QIcon>
#include <QSplashScreen>
#include <QElapsedTimer>
#include <QLoggingCategory>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    
    // Disable verbose icon/font logs by default (still toggleable via QT_LOGGING_RULES)
    QLoggingCategory::setFilterRules(QStringLiteral(
        "phx.icons.debug=false\n"
        "phx.fonts.debug=false\n"
    ));
    
    // Set application properties
    app.setApplicationName("Phoenix");
    app.setApplicationVersion("0.0.1");
    app.setOrganizationName("Phoenix");
    app.setOrganizationDomain("phoenix.dev");
    
    // Set application icon for Dock on macOS
    app.setWindowIcon(QIcon(":/phoenix-icon.svg"));
    
    // Start timing immediately
    QElapsedTimer timer;
    timer.start();
    
    // Show splash screen immediately
    PhoenixSplashScreen splash;
    splash.show();
    app.processEvents(); // Process splash screen display
    
    // Initialize Font Awesome icons (must be before any icon rendering)
    IconBootstrap::InitFonts();
    
    // Setup automatic cache clearing on theme/DPR changes
    IconProvider::setupCacheClearing();
    
    // High DPI scaling is enabled by default in Qt 6
    
    // Create main window (but don't show it yet)
    MainWindow mainWindow;
    
    // Connect splash finish to firstShown signal
    QObject::connect(&mainWindow, &MainWindow::firstShown, [&]() {
        const qint64 elapsed = timer.elapsed();
        mainWindow.setStartupTimeMs(elapsed);
        splash.finish(&mainWindow);
    });
    
    // Show main window (triggers showEvent → firstShown → finish splash)
    mainWindow.show();
    mainWindow.raise();
    mainWindow.activateWindow();
    
    return app.exec();
}
