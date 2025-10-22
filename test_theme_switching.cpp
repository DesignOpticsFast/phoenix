#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QLoggingCategory>
#include "src/ui/themes/ThemeManager.h"
#include "src/ui/icons/IconProvider.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Enable icon logging
    QLoggingCategory::setFilterRules("phx.icons=true");
    
    // Get ThemeManager instance
    ThemeManager* themeManager = ThemeManager::instance();
    
    qDebug() << "=== Testing Theme Switching ===";
    qDebug() << "Initial theme:" << (themeManager->isDarkMode() ? "Dark" : "Light");
    
    // Test light mode
    qDebug() << "\n--- Testing Light Mode ---";
    themeManager->setDarkMode(false);
    qDebug() << "Theme after setDarkMode(false):" << (themeManager->isDarkMode() ? "Dark" : "Light");
    
    // Request an icon in light mode
    QIcon lightIcon = IconProvider::icon("file-plus", IconStyle::SharpSolid, 16, false);
    qDebug() << "Light mode icon null:" << lightIcon.isNull();
    
    // Test dark mode
    qDebug() << "\n--- Testing Dark Mode ---";
    themeManager->setDarkMode(true);
    qDebug() << "Theme after setDarkMode(true):" << (themeManager->isDarkMode() ? "Dark" : "Light");
    
    // Request an icon in dark mode
    QIcon darkIcon = IconProvider::icon("file-plus", IconStyle::SharpSolid, 16, true);
    qDebug() << "Dark mode icon null:" << darkIcon.isNull();
    
    // Test system theme
    qDebug() << "\n--- Testing System Theme ---";
    themeManager->setTheme(ThemeManager::Theme::System);
    qDebug() << "Theme after setTheme(System):" << (themeManager->isDarkMode() ? "Dark" : "Light");
    
    qDebug() << "\n=== Theme Switching Test Complete ===";
    
    // Exit after a short delay
    QTimer::singleShot(100, &app, &QApplication::quit);
    return app.exec();
}
