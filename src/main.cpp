#include "ui/main/MainWindow.h"
#include "ui/splash/PhoenixSplashScreen.h"
#include <QApplication>
#include <QTimer>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Phoenix");
    app.setApplicationVersion("0.0.1");
    app.setOrganizationName("Phoenix");
    app.setOrganizationDomain("phoenix.dev");
    
    // High DPI scaling is enabled by default in Qt 6
    
    // Show splash screen immediately
    PhoenixSplashScreen splash;
    splash.show();
    app.processEvents(); // Process splash screen display
    
    // Create main window (but don't show it yet)
    MainWindow mainWindow;
    
    // Simulate initialization time
    QTimer::singleShot(2000, [&splash, &mainWindow]() {
        // Hide splash screen
        splash.finish(&mainWindow);
        
        // Show main window
        mainWindow.show();
        mainWindow.raise();
        mainWindow.activateWindow();
    });
    
    return app.exec();
}
