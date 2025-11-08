#include "LocaleInit.hpp"
#include "SettingsKeys.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QProcessEnvironment>
#include <QSettings>
#include <QStringList>
#include <QTranslator>

namespace {
QString normalizedLang(const QString& value)
{
    const QString lowered = value.trimmed().toLower();
    if (lowered == QStringLiteral("de")) {
        return QStringLiteral("de");
    }
    return QStringLiteral("en");
}

QString languageFromArgs(const QStringList& args)
{
    for (int i = 1; i < args.size(); ++i) {
        const QString arg = args.at(i);
        if (arg == QStringLiteral("--lang") && i + 1 < args.size()) {
            return args.at(i + 1);
        }
        if (arg.startsWith(QStringLiteral("--lang="))) {
            return arg.mid(QStringLiteral("--lang=").length());
        }
    }
    return {};
}

QStringList translationSearchPaths()
{
    QStringList paths;
    const QString qtTranslations = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if (!qtTranslations.isEmpty()) {
        paths << qtTranslations;
    }

#ifdef Q_OS_MAC
    QDir bundleDir(QCoreApplication::applicationDirPath());
    if (bundleDir.cdUp() && bundleDir.cd(QStringLiteral("Resources")) && bundleDir.cd(QStringLiteral("translations"))) {
        paths << bundleDir.absolutePath();
    }
#endif

    paths << (QCoreApplication::applicationDirPath() + QStringLiteral("/translations"));

    paths.removeDuplicates();
    return paths;
}

bool loadTranslator(QTranslator& translator, const QString& prefix, const QString& lang, const QStringList& searchPaths, QString& resolvedPath)
{
    for (const auto& dirPath : searchPaths) {
        if (translator.load(QStringLiteral("%1_%2.qm").arg(prefix, lang), dirPath)) {
            if (!translator.isEmpty()) {
                resolvedPath = dirPath;
                return true;
            }
        }
    }
    resolvedPath.clear();
    return false;
}

void ensureSettingsDefaults(const QString& lang, const QString& locale, bool hadStoredLang)
{
    QSettings settings;
    if (!hadStoredLang && !settings.contains(PhxKeys::UI_LANGUAGE)) {
        settings.setValue(PhxKeys::UI_LANGUAGE, lang);
    }
    if (!settings.contains(PhxKeys::UI_LOCALE)) {
        settings.setValue(PhxKeys::UI_LOCALE, locale);
    }
}
} // namespace

namespace i18n {

QString localeForLanguage(const QString& shortLang)
{
    return (shortLang == QStringLiteral("de")) ? QStringLiteral("de_DE") : QStringLiteral("en_US");
}

Result setup(QApplication& app)
{
    Result result;

    static QTranslator qtTranslator;
    static QTranslator appTranslator;

    app.removeTranslator(&qtTranslator);
    app.removeTranslator(&appTranslator);

    QSettings settings;
    const QString storedLang = settings.value(PhxKeys::UI_LANGUAGE).toString();
    const bool hadStoredLang = !storedLang.isEmpty();

    QString lang = storedLang;

    if (lang.isEmpty()) {
        lang = languageFromArgs(QCoreApplication::arguments());
    }
    if (lang.isEmpty()) {
        lang = qEnvironmentVariable("PHOENIX_LANG");
    }
    if (lang.isEmpty()) {
        lang = QLocale::system().name().left(2);
    }

    lang = normalizedLang(lang);

    result.lang = lang;
    result.locale = localeForLanguage(lang);

    QLocale::setDefault(QLocale(result.locale));

    const QStringList searchPaths = translationSearchPaths();
    qInfo().noquote() << "[i18n] translation search paths:" << searchPaths.join(QStringLiteral("; "));

    bool qtLoaded = loadTranslator(qtTranslator, QStringLiteral("qtbase"), lang, searchPaths, result.qtBasePath);
    bool appLoaded = loadTranslator(appTranslator, QStringLiteral("phoenix"), lang, searchPaths, result.appPath);
    qInfo().nospace() << "[i18n] load qtbase (" << lang << ") => " << (qtLoaded ? "OK" : "FAIL")
                      << ", path=" << (result.qtBasePath.isEmpty() ? QStringLiteral("<none>") : result.qtBasePath);
    qInfo().nospace() << "[i18n] load phoenix (" << lang << ") => " << (appLoaded ? "OK" : "FAIL")
                      << ", path=" << (result.appPath.isEmpty() ? QStringLiteral("<none>") : result.appPath);

    if (!qtLoaded || !appLoaded) {
        if (lang != QStringLiteral("en")) {
            qWarning() << "[i18n] Missing translators for" << lang << "— falling back to English.";
        }
        lang = QStringLiteral("en");
        result.lang = lang;
        result.locale = localeForLanguage(lang);
        QLocale::setDefault(QLocale(result.locale));

        qtLoaded = loadTranslator(qtTranslator, QStringLiteral("qtbase"), lang, searchPaths, result.qtBasePath);
        appLoaded = loadTranslator(appTranslator, QStringLiteral("phoenix"), lang, searchPaths, result.appPath);
        qInfo().nospace() << "[i18n] fallback qtbase => " << (qtLoaded ? "OK" : "FAIL")
                          << ", path=" << (result.qtBasePath.isEmpty() ? QStringLiteral("<none>") : result.qtBasePath);
        qInfo().nospace() << "[i18n] fallback phoenix => " << (appLoaded ? "OK" : "FAIL")
                          << ", path=" << (result.appPath.isEmpty() ? QStringLiteral("<none>") : result.appPath);
    }

    if (qtLoaded && !qtTranslator.isEmpty()) {
        app.installTranslator(&qtTranslator);
    }
    if (appLoaded && !appTranslator.isEmpty()) {
        app.installTranslator(&appTranslator);
    }

    ensureSettingsDefaults(result.lang, result.locale, hadStoredLang);
    settings.setValue(PhxKeys::UI_APPLIED_LANGUAGE, result.lang);
    settings.setValue(PhxKeys::UI_APPLIED_LOCALE, result.locale);
    settings.sync();

    qInfo() << "[i18n] applied stored="
            << settings.value(PhxKeys::UI_LANGUAGE).toString()
            << "applied=" << settings.value(PhxKeys::UI_APPLIED_LANGUAGE).toString()
            << "QLocale(default)=" << QLocale().name();

    qInfo().nospace() << "[i18n] lang=" << result.lang
                      << " locale=" << result.locale
                      << " qtBasePath=" << (result.qtBasePath.isEmpty() ? QStringLiteral("<none>") : result.qtBasePath)
                      << " appPath=" << (result.appPath.isEmpty() ? QStringLiteral("<none>") : result.appPath)
                      << " stored=" << storedLang;

    return result;
}

} // namespace i18n
