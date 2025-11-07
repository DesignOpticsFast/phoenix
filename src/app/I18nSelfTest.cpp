#include "I18nSelfTest.hpp"

#include "LocaleInit.hpp"
#include <QApplication>
#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QLocale>
#include <QSettings>
#include <QTextStream>

using i18n::Result;

namespace {

struct Probe {
    const char* ctx;
    const char* src;
};

static const Probe PROBES[] = {
    {"MainWindow", "&Language"},
    {"LanguagePage", "Current Language:"},
    {"LanguagePage", "Restart Required"},
    {"EnvironmentPage", "Reset to Factory Defaults…"},
    {"", "Preferences"}
};

QString translateProbe(const Probe& p)
{
    return QCoreApplication::translate(p.ctx && *p.ctx ? p.ctx : nullptr, p.src);
}

QString numberSample(const QLocale& loc)
{
    return QStringLiteral("1234.56 -> %1").arg(loc.toString(1234.56, 'f', 2));
}

QString dateSample(const QLocale& loc)
{
    const QDate date{2025, 12, 31};
    return QStringLiteral("2025-12-31 -> %1").arg(loc.toString(date, QLocale::ShortFormat));
}

} // namespace

namespace i18nselftest {

int run(QApplication& app, const QString& explicitLangCode)
{
    QTextStream out(stdout);

    if (!explicitLangCode.isEmpty()) {
        QSettings s;
        s.setValue(QStringLiteral("ui/language"), explicitLangCode);
        s.setValue(QStringLiteral("ui/locale"), explicitLangCode == QStringLiteral("de") ? QStringLiteral("de_DE") : QStringLiteral("en_US"));
        s.sync();
    }

    Result result = i18n::setup(app);

    QSettings settings;
    const QString storedLang = settings.value(QStringLiteral("ui/language"), QStringLiteral("en")).toString();
    const QString storedLocale = settings.value(QStringLiteral("ui/locale"), QStringLiteral("en_US")).toString();
    const QString appliedLang = settings.value(QStringLiteral("ui/applied_language"), QStringLiteral("en")).toString();
    const QString appliedLocale = settings.value(QStringLiteral("ui/applied_locale"), QStringLiteral("en_US")).toString();

    out << "=== i18n Self-Test ===\n";
    out << "stored ui/language : " << storedLang << "\n";
    out << "stored ui/locale   : " << storedLocale << "\n";
    out << "applied ui/language: " << appliedLang << "\n";
    out << "applied ui/locale  : " << appliedLocale << "\n";
    out << "QLocale(system)    : " << QLocale::system().name() << "\n";
    out << "QLocale(default)   : " << QLocale().name() << "\n";
    out << "setup.lang         : " << result.lang << "\n";
    out << "setup.locale       : " << result.locale << "\n";
    out << "qtbase.qm path     : " << result.qtBasePath << "\n";
    out << "app.qm path        : " << result.appPath << "\n";

    const QLocale loc;
    out << "format.number      : " << numberSample(loc) << "\n";
    out << "format.date        : " << dateSample(loc) << "\n";

    out << "--- translation probes ---\n";
    for (const auto& probe : PROBES) {
        const QString translated = translateProbe(probe);
        const bool same = translated == QString::fromUtf8(probe.src);
        out << "[" << (probe.ctx && *probe.ctx ? probe.ctx : "<no ctx>") << "] "
            << probe.src << " -> " << translated << (same ? " (same)" : "") << "\n";
    }

    out << "=== end ===\n";
    out.flush();
    return 0;
}

} // namespace i18nselftest
