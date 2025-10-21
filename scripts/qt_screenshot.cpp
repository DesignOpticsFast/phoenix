#include <QApplication>
#include <QScreen>
#include <QPixmap>
#include <QGuiApplication>
#include <QWindow>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Get the primary screen
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        qDebug() << "No screen available";
        return 1;
    }
    
    // Capture the screen
    QPixmap screenshot = screen->grabWindow(0);
    
    if (screenshot.isNull()) {
        qDebug() << "Failed to capture screenshot";
        return 1;
    }
    
    // Save to screenshots directory
    QString screenshotsDir = QDir::currentPath() + "/screenshots";
    QDir().mkpath(screenshotsDir);
    
    QString filename = screenshotsDir + "/phoenix_ui_qt.png";
    bool saved = screenshot.save(filename, "PNG");
    
    if (saved) {
        qDebug() << "Screenshot saved to:" << filename;
        qDebug() << "Size:" << screenshot.size();
        return 0;
    } else {
        qDebug() << "Failed to save screenshot";
        return 1;
    }
}
