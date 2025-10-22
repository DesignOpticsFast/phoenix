#include "ui/main/MainWindow.h"
#include "ui/splash/PhoenixSplashScreen.h"
#include "ui/icons/IconBootstrap.h"
#include <QApplication>
#include <QTimer>
#include <QIcon>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Phoenix");
    app.setApplicationVersion("0.0.1");
    app.setOrganizationName("Phoenix");
    app.setOrganizationDomain("phoenix.dev");
    
    // Set application icon for Dock on macOS
    app.setWindowIcon(QIcon(":/src/resources/icons/phoenix-icon.svg"));
    
    // Initialize Font Awesome icons (must be before any icon rendering)
    IconBootstrap::InitFonts();
    
    // High DPI scaling is enabled by default in Qt 6
    
    // Show splash screen immediately
    PhoenixSplashScreen splash;
    splash.show();
    app.processEvents(); // Process splash screen display
    
    // Create main window (but don't show it yet)
    MainWindow mainWindow;
    
    // Ensure splash screen stays visible during initialization
    splash.raise();
    splash.activateWindow();
    
    // Simulate initialization time
    QTimer::singleShot(3000, &app, [&splash, &mainWindow]() {
        // Pass startup time to main window
        mainWindow.setStartupTime(splash.getStartTime());
        
        // Hide splash screen
        splash.finish(&mainWindow);
        
        // Show main window
        mainWindow.show();
        mainWindow.raise();
        mainWindow.activateWindow();
    });
    
    return app.exec();
}
