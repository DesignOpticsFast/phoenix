#include "VegaLiteLocalizer.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QLocale>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <QObject>

using namespace vega;

namespace {

const QPair<QRegularExpression, QString> TERM_MAP_DE[] = {
    { QRegularExpression(QStringLiteral(R"(\bLanguage\b)")), QObject::tr("Sprache") },
    { QRegularExpression(QStringLiteral(R"(\bPreferences\b)")), QObject::tr("Einstellungen") },
    { QRegularExpression(QStringLiteral(R"(\bEnvironment\b)")), QObject::tr("Umgebung") },
    { QRegularExpression(QStringLiteral(R"(\bCurrent Language\b)")), QObject::tr("Aktuelle Sprache") },
    { QRegularExpression(QStringLiteral(R"(\bRestart Required\b)")), QObject::tr("Neustart erforderlich") },
    { QRegularExpression(QStringLiteral(R"(\bDistance\b)")), QObject::tr("Abstand") },
    { QRegularExpression(QStringLiteral(R"(\bStress\b)")), QObject::tr("Spannung") },
    { QRegularExpression(QStringLiteral(R"(\bWavelength\b)")), QObject::tr("Wellenlänge") },
    { QRegularExpression(QStringLiteral(R"(\bIntensity\b)")), QObject::tr("Intensität") },
    { QRegularExpression(QStringLiteral(R"(\bField Angle\b)")), QObject::tr("Feldwinkel") },
    { QRegularExpression(QStringLiteral(R"(\bSpot Radius\b)")), QObject::tr("Spot-Radius") },
    { QRegularExpression(QStringLiteral(R"(\bRay Height\b)")), QObject::tr("Strahlenhöhe") },
    { QRegularExpression(QStringLiteral(R"(\bSurface Sag\b)")), QObject::tr("Oberflächensenkung") },
    { QRegularExpression(QStringLiteral(R"(\bZernike Coefficient\b)")), QObject::tr("Zernike-Koeffizient") },
    { QRegularExpression(QStringLiteral(R"(\bField Curvature\b)")), QObject::tr("Feldkrümmung") },
};

bool isIdentifierLike(const QString& s)
{
    static const QRegularExpression material(QStringLiteral(R"(^[A-Z]{1,3}-[A-Z0-9]{2,8}$)"));
    static const QRegularExpression hasExt(QStringLiteral(R"(\.[A-Za-z0-9]{1,6}$)"));
    static const QRegularExpression path(QStringLiteral(R"([/\\])"));
    static const QRegularExpression hex(QStringLiteral(R"(^[A-Fa-f0-9]{6,}$)"));

    return material.match(s).hasMatch()
        || hasExt.match(s).hasMatch()
        || path.match(s).hasMatch()
        || hex.match(s).hasMatch();
}

bool isUnitToken(const QString& u)
{
    static const QSet<QString> units = {
        QStringLiteral("mm"), QStringLiteral("μm"), QStringLiteral("um"), QStringLiteral("nm"),
        QStringLiteral("MPa"), QStringLiteral("deg"), QStringLiteral("°"), QStringLiteral("Hz"),
        QStringLiteral("kHz"), QStringLiteral("MHz"), QStringLiteral("GHz")
    };
    return units.contains(u);
}

QJsonObject makeNumberFormatLocale(const QLocale& locale)
{
    QJsonObject obj;
    const QString decimal = QString(locale.decimalPoint());
    const QString thousands = QString(locale.groupSeparator());

    obj.insert(QStringLiteral("decimal"), decimal);
    obj.insert(QStringLiteral("thousands"), thousands);
    obj.insert(QStringLiteral("grouping"), QJsonArray{ 3 });
    obj.insert(QStringLiteral("currency"), QJsonArray{ QString(), QString() });

    return obj;
}

QJsonObject makeTimeFormatLocale(const QLocale& locale)
{
    QJsonObject obj;
    QJsonArray days;
    QJsonArray shortDays;
    for (int i = 1; i <= 7; ++i) {
        days.append(locale.dayName(i, QLocale::LongFormat));
        shortDays.append(locale.dayName(i, QLocale::ShortFormat));
    }

    QJsonArray months;
    QJsonArray shortMonths;
    for (int month = 1; month <= 12; ++month) {
        months.append(locale.monthName(month, QLocale::LongFormat));
        shortMonths.append(locale.monthName(month, QLocale::ShortFormat));
    }

    obj.insert(QStringLiteral("days"), days);
    obj.insert(QStringLiteral("shortDays"), shortDays);
    obj.insert(QStringLiteral("months"), months);
    obj.insert(QStringLiteral("shortMonths"), shortMonths);

    const QString name = locale.name();
    if (name.startsWith(QLatin1String("de"))) {
        obj.insert(QStringLiteral("date"), QStringLiteral("%d.%m.%Y"));
        obj.insert(QStringLiteral("time"), QStringLiteral("%H:%M:%S"));
        obj.insert(QStringLiteral("dateTime"), QStringLiteral("%A %d.%m.%Y %H:%M:%S"));
        obj.insert(QStringLiteral("periods"), QJsonArray{ QString(), QString() });
    } else {
        obj.insert(QStringLiteral("date"), QStringLiteral("%m/%d/%Y"));
        obj.insert(QStringLiteral("time"), QStringLiteral("%H:%M:%S"));
        obj.insert(QStringLiteral("dateTime"), QStringLiteral("%A %m/%d/%Y %H:%M:%S"));
        obj.insert(QStringLiteral("periods"), QJsonArray{ QStringLiteral("AM"), QStringLiteral("PM") });
    }

    return obj;
}

} // namespace

QString VegaLiteLocalizer::translateWholeWords(const QString& s)
{
    QString out = s;
    for (const auto& entry : TERM_MAP_DE) {
        out.replace(entry.first, entry.second);
    }
    return out;
}

QString VegaLiteLocalizer::translateLabelPreservingUnits(const QString& src)
{
    if (src.isEmpty() || isIdentifierLike(src)) {
        return src;
    }

    static const QRegularExpression unitPattern(QStringLiteral("^(.*?)(\\s*\\(([^\\)]+)\\))$"));
    const QRegularExpressionMatch match = unitPattern.match(src);
    if (match.hasMatch()) {
        const QString stem = match.captured(1).trimmed();
        const QString unit = match.captured(3).trimmed();
        if (isUnitToken(unit)) {
            return QStringLiteral("%1 (%2)").arg(translateWholeWords(stem), unit);
        }
    }

    return translateWholeWords(src);
}

QJsonObject VegaLiteLocalizer::injectLocaleConfig(QJsonObject cfg)
{
    const QLocale locale; // honours QLocale::setDefault
    cfg.insert(QStringLiteral("numberFormat"), QStringLiteral(".2f"));
    cfg.insert(QStringLiteral("timeFormat"), QStringLiteral("%Y-%m-%d"));
    cfg.insert(QStringLiteral("numberFormatLocale"), makeNumberFormatLocale(locale));
    cfg.insert(QStringLiteral("timeFormatLocale"), makeTimeFormatLocale(locale));
    cfg.insert(QStringLiteral("locale"), locale.name());
    return cfg;
}

bool VegaLiteLocalizer::isObjectWith(const QJsonObject& obj, const char* key)
{
    return obj.contains(QLatin1String(key)) && obj.value(QLatin1String(key)).isObject();
}

void VegaLiteLocalizer::localizeAxis(QJsonObject& axisCfg)
{
    if (axisCfg.contains(QStringLiteral("title"))) {
        const QJsonValue titleVal = axisCfg.value(QStringLiteral("title"));
        if (titleVal.isString()) {
            axisCfg.insert(QStringLiteral("title"), translateLabelPreservingUnits(titleVal.toString()));
        } else if (titleVal.isObject()) {
            QJsonObject titleObj = titleVal.toObject();
            if (titleObj.contains(QStringLiteral("text")) && titleObj.value(QStringLiteral("text")).isString()) {
                titleObj.insert(QStringLiteral("text"), translateLabelPreservingUnits(titleObj.value(QStringLiteral("text")).toString()));
                axisCfg.insert(QStringLiteral("title"), titleObj);
            }
        }
    }
}

void VegaLiteLocalizer::localizeLegend(QJsonObject& legendCfg)
{
    if (legendCfg.contains(QStringLiteral("title"))) {
        const QJsonValue titleVal = legendCfg.value(QStringLiteral("title"));
        if (titleVal.isString()) {
            legendCfg.insert(QStringLiteral("title"), translateLabelPreservingUnits(titleVal.toString()));
        } else if (titleVal.isObject()) {
            QJsonObject titleObj = titleVal.toObject();
            if (titleObj.contains(QStringLiteral("text")) && titleObj.value(QStringLiteral("text")).isString()) {
                titleObj.insert(QStringLiteral("text"), translateLabelPreservingUnits(titleObj.value(QStringLiteral("text")).toString()));
                legendCfg.insert(QStringLiteral("title"), titleObj);
            }
        }
    }
}

void VegaLiteLocalizer::localizeEncoding(QJsonObject& spec)
{
    if (!spec.contains(QStringLiteral("encoding")) || !spec.value(QStringLiteral("encoding")).isObject()) {
        return;
    }

    QJsonObject encoding = spec.value(QStringLiteral("encoding")).toObject();
    const QStringList channels = {
        QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("x2"), QStringLiteral("y2"),
        QStringLiteral("color"), QStringLiteral("size"), QStringLiteral("shape"), QStringLiteral("opacity"),
        QStringLiteral("row"), QStringLiteral("column")
    };

    for (const QString& channelKey : channels) {
        if (!encoding.contains(channelKey) || !encoding.value(channelKey).isObject()) {
            continue;
        }

        QJsonObject channel = encoding.value(channelKey).toObject();

        if (channel.contains(QStringLiteral("axis")) && channel.value(QStringLiteral("axis")).isObject()) {
            QJsonObject axisCfg = channel.value(QStringLiteral("axis")).toObject();
            localizeAxis(axisCfg);
            channel.insert(QStringLiteral("axis"), axisCfg);
        }

        if (channel.contains(QStringLiteral("legend")) && channel.value(QStringLiteral("legend")).isObject()) {
            QJsonObject legendCfg = channel.value(QStringLiteral("legend")).toObject();
            localizeLegend(legendCfg);
            channel.insert(QStringLiteral("legend"), legendCfg);
        }

        encoding.insert(channelKey, channel);
    }

    spec.insert(QStringLiteral("encoding"), encoding);
}

void VegaLiteLocalizer::localizeHeaders(QJsonObject& spec)
{
    if (isObjectWith(spec, "facet")) {
        QJsonObject facet = spec.value(QStringLiteral("facet")).toObject();
        const QStringList headerKeys = { QStringLiteral("row"), QStringLiteral("column") };
        for (const QString& key : headerKeys) {
            if (facet.contains(key) && facet.value(key).isObject()) {
                QJsonObject header = facet.value(key).toObject();
                if (header.contains(QStringLiteral("header")) && header.value(QStringLiteral("header")).isObject()) {
                    QJsonObject headerCfg = header.value(QStringLiteral("header")).toObject();
                    if (headerCfg.contains(QStringLiteral("title")) && headerCfg.value(QStringLiteral("title")).isString()) {
                        headerCfg.insert(QStringLiteral("title"), translateLabelPreservingUnits(headerCfg.value(QStringLiteral("title")).toString()));
                    }
                    header.insert(QStringLiteral("header"), headerCfg);
                }
                if (header.contains(QStringLiteral("title")) && header.value(QStringLiteral("title")).isString()) {
                    header.insert(QStringLiteral("title"), translateLabelPreservingUnits(header.value(QStringLiteral("title")).toString()));
                }
                facet.insert(key, header);
            }
        }
        spec.insert(QStringLiteral("facet"), facet);
    }

    if (isObjectWith(spec, "repeat")) {
        QJsonObject repeat = spec.value(QStringLiteral("repeat")).toObject();
        const QStringList keys = { QStringLiteral("row"), QStringLiteral("column") };
        for (const QString& key : keys) {
            if (repeat.contains(key) && repeat.value(key).isObject()) {
                QJsonObject nested = repeat.value(key).toObject();
                localizeSpec(nested);
                repeat.insert(key, nested);
            }
        }
        spec.insert(QStringLiteral("repeat"), repeat);
    }
}

void VegaLiteLocalizer::localizeNested(QJsonObject& spec)
{
    const QStringList arrayKeys = { QStringLiteral("layer"), QStringLiteral("hconcat"), QStringLiteral("vconcat") };
    for (const QString& key : arrayKeys) {
        if (!spec.contains(key) || !spec.value(key).isArray()) {
            continue;
        }

        QJsonArray arr = spec.value(key).toArray();
        for (int i = 0; i < arr.size(); ++i) {
            if (!arr.at(i).isObject()) {
                continue;
            }
            QJsonObject nested = arr.at(i).toObject();
            localizeSpec(nested);
            arr.replace(i, nested);
        }
        spec.insert(key, arr);
    }

    if (isObjectWith(spec, "facet")) {
        QJsonObject facet = spec.value(QStringLiteral("facet")).toObject();
        if (facet.contains(QStringLiteral("spec")) && facet.value(QStringLiteral("spec")).isObject()) {
            QJsonObject inner = facet.value(QStringLiteral("spec")).toObject();
            localizeSpec(inner);
            facet.insert(QStringLiteral("spec"), inner);
        }
        spec.insert(QStringLiteral("facet"), facet);
    }

    if (isObjectWith(spec, "repeat")) {
        QJsonObject repeat = spec.value(QStringLiteral("repeat")).toObject();
        if (repeat.contains(QStringLiteral("spec")) && repeat.value(QStringLiteral("spec")).isObject()) {
            QJsonObject inner = repeat.value(QStringLiteral("spec")).toObject();
            localizeSpec(inner);
            repeat.insert(QStringLiteral("spec"), inner);
        }
        spec.insert(QStringLiteral("repeat"), repeat);
    }

    if (spec.contains(QStringLiteral("spec")) && spec.value(QStringLiteral("spec")).isObject()) {
        QJsonObject inner = spec.value(QStringLiteral("spec")).toObject();
        localizeSpec(inner);
        spec.insert(QStringLiteral("spec"), inner);
    }
}

void VegaLiteLocalizer::localizeSpec(QJsonObject& spec)
{
    QJsonObject cfg = spec.contains(QStringLiteral("config")) && spec.value(QStringLiteral("config")).isObject()
        ? spec.value(QStringLiteral("config")).toObject()
        : QJsonObject{};

    spec.insert(QStringLiteral("config"), injectLocaleConfig(cfg));

    if (spec.contains(QStringLiteral("title"))) {
        const QJsonValue titleVal = spec.value(QStringLiteral("title"));
        if (titleVal.isString()) {
            spec.insert(QStringLiteral("title"), translateLabelPreservingUnits(titleVal.toString()));
        } else if (titleVal.isObject()) {
            QJsonObject titleObj = titleVal.toObject();
            if (titleObj.contains(QStringLiteral("text")) && titleObj.value(QStringLiteral("text")).isString()) {
                titleObj.insert(QStringLiteral("text"), translateLabelPreservingUnits(titleObj.value(QStringLiteral("text")).toString()));
                spec.insert(QStringLiteral("title"), titleObj);
            }
        }
    }

    localizeEncoding(spec);
    localizeHeaders(spec);
    localizeNested(spec);
}

QJsonObject VegaLiteLocalizer::localize(const QJsonObject& specIn)
{
    QJsonObject out = specIn;
    localizeSpec(out);
    return out;
}
