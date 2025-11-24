#include "ui/main/MainWindow.h"
#include "ui/splash/PhoenixSplashScreen.h"
#include "ui/icons/IconBootstrap.h"
#include "ui/icons/IconProvider.h"
#include "ui/icons/PhxLogging.h"
#include "ui/themes/ThemeManager.h"
#include "app/LocaleInit.hpp"
#include "app/SettingsProvider.h"
#include "ui/analysis/AnalysisWindowManager.hpp"
#include "version.h"
#include <QApplication>
#include <QTimer>
#include <QIcon>
#include <QSplashScreen>
#include <QElapsedTimer>
#include <QDateTime>
#include <QLoggingCategory>
#include <QProcessEnvironment>
#include <memory>
#include "app/I18nSelfTest.hpp"
#include <QSettings>

int main(int argc, char** argv) {
#ifdef Q_OS_LINUX
    // Use X11/XWayland by default for stable docking on Linux.
    // Advanced users can:
    //  - set PHOENIX_FORCE_WAYLAND to keep native Wayland, or
    //  - set QT_QPA_PLATFORM themselves to any backend they want.
    const bool forceWayland = qEnvironmentVariableIsSet("PHOENIX_FORCE_WAYLAND");
    const QByteArray existingPlatform = qgetenv("QT_QPA_PLATFORM");

    if (forceWayland) {
        qInfo() << "[platform] Linux detected - PHOENIX_FORCE_WAYLAND set,"
                << "using native Wayland backend (QT_QPA_PLATFORM left as-is)";
    } else if (!existingPlatform.isEmpty()) {
        qInfo() << "[platform] Linux detected - QT_QPA_PLATFORM already set to"
                << existingPlatform << "- using user-specified backend";
    } else {
        qputenv("QT_QPA_PLATFORM", QByteArray("xcb"));
        qInfo() << "[platform] Linux detected - using X11/XWayland backend"
                << "(QT_QPA_PLATFORM=xcb) for stable docking behavior";
    }
#endif

    QApplication app(argc, argv);
    
    // Disable verbose logs by default (still toggleable via QT_LOGGING_RULES)
    QLoggingCategory::setFilterRules(QStringLiteral(
        "phx.icons.debug=false\n"
        "phx.fonts.debug=false\n"
        "phx.ui.debug=false\n"
    ));
    
    // Set application properties
    QCoreApplication::setApplicationName("Phoenix");
    QCoreApplication::setApplicationVersion(QStringLiteral(PHOENIX_VERSION));
    QCoreApplication::setOrganizationName("Phoenix");
    QCoreApplication::setOrganizationDomain("phoenix.dev");
    
    const QStringList args = QCoreApplication::arguments();
    const bool testI18n = args.contains(QStringLiteral("--test-i18n"));
    QString optLang;
    for (int i = 0; i < args.size(); ++i) {
        if (args[i] == QStringLiteral("--lang") && i + 1 < args.size()) {
            optLang = args[i + 1];
        } else if (args[i].startsWith(QStringLiteral("--lang="))) {
            optLang = args[i].mid(7);
        }
    }

    if (testI18n) {
        qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
        return i18nselftest::run(app, optLang);
    }

    qInfo() << "[i18n] settings store org=" << QCoreApplication::organizationName()
            << "app=" << QCoreApplication::applicationName();

    auto settings = std::make_unique<QSettings>();
    auto* settingsProvider = new SettingsProvider(&app, std::move(settings));

    auto i18nResult = i18n::setup(app, settingsProvider->settings());
    Q_UNUSED(i18nResult);
    
    // Ensure translators are active before creating splash screen
    app.processEvents();

    // Set application icon for Dock on macOS
    app.setWindowIcon(QIcon(":/phoenix-icon.svg"));

    // Capture startup start time when splash is shown
    const qint64 startupStartMs = QDateTime::currentMSecsSinceEpoch();

    // Create and initialize splash screen (messages initialized after translators are active)
    PhoenixSplashScreen splash;
    splash.initializeMessages();
    splash.show();
    app.processEvents(); // Process splash screen display
    
    // Force repaint to ensure translated text is displayed
    splash.repaint();
    app.processEvents();
    
    // Record when splash was shown for minimum display duration
    const qint64 splashShownAt = QDateTime::currentMSecsSinceEpoch();

    // Initialize Font Awesome icons (must be before any icon rendering)
    IconBootstrap::InitFonts();
    
    // Clear icon cache only if explicitly requested via environment variable
    // (Cache is already cleared on palette/screen changes via setupCacheClearing())
    if (qEnvironmentVariableIsSet("PHX_ICON_NOCACHE")) {
        IconProvider::clearCache();
        qCDebug(phxIcons) << "Icon cache cleared and bypassed via PHX_ICON_NOCACHE";
    }
    
    // Setup automatic cache clearing on theme/DPR changes
    IconProvider::setupCacheClearing();
    
    // High DPI scaling is enabled by default in Qt 6
    
    // Wire ThemeManager singleton (must happen before ThemeManager::instance() use)
    ThemeManager::setSettingsProvider(settingsProvider);
    
    // Create main window (but don't show it yet)
    MainWindow mainWindow(settingsProvider);
    mainWindow.setStartupStartTime(startupStartMs);
    
    // Connect splash finish to firstShown signal with minimum display duration
    QObject::connect(&mainWindow, &MainWindow::firstShown, [&, splashShownAt]() {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        const qint64 elapsed = now - splashShownAt;
        const qint64 minDuration = 1000; // Minimum 1 second display time
        
        if (elapsed >= minDuration) {
            splash.finish(&mainWindow);
        } else {
            QTimer::singleShot(static_cast<int>(minDuration - elapsed), &splash, [&]() {
                splash.finish(&mainWindow);
            });
        }
    });
    
    // Show main window (triggers showEvent → firstShown → finish splash)
    mainWindow.show();
    mainWindow.raise();
    mainWindow.activateWindow();
    
    // Ensure all analysis windows are closed on application exit
    // This is a safety net for cases where main window close doesn't trigger closeEvent
    QObject::connect(&app, &QApplication::aboutToQuit, []() {
        AnalysisWindowManager::instance()->closeAll();
    });
    
    return app.exec();
}
