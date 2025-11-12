#include "ui/main/MainWindow.h"
#include "ui/splash/PhoenixSplashScreen.h"
#include "ui/icons/IconBootstrap.h"
#include "ui/icons/IconProvider.h"
#include "ui/icons/PhxLogging.h"
#include "ui/themes/ThemeManager.h"
#include "app/LocaleInit.hpp"
#include "app/SettingsProvider.h"
#include "version.h"
#include <QApplication>
#include <QTimer>
#include <QIcon>
#include <QSplashScreen>
#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QProcessEnvironment>
#include <memory>
#include "app/I18nSelfTest.hpp"
#include <QSettings>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    
    // Disable verbose logs by default (still toggleable via QT_LOGGING_RULES)
    QLoggingCategory::setFilterRules(QStringLiteral(
        "phx.icons.debug=false\n"
        "phx.fonts.debug=false\n"
        "phx.ui.debug=false\n"
        "phx.palantir.debug=false\n"
        "phx.palantir.conn.debug=false\n"
        "phx.palantir.proto.debug=false\n"
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
    
    // Connect splash finish to firstShown signal
    QObject::connect(&mainWindow, &MainWindow::firstShown, [&]() {
        splash.finish(&mainWindow);
    });
    
    // Show main window (triggers showEvent → firstShown → finish splash)
    mainWindow.show();
    mainWindow.raise();
    mainWindow.activateWindow();
    
    return app.exec();
}
