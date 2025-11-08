#pragma once

#include <QJsonObject>

namespace vega {

class VegaLiteLocalizer {
public:
    static QJsonObject localize(const QJsonObject& spec);

private:
    static QJsonObject injectLocaleConfig(QJsonObject cfg);
    static void        localizeSpec(QJsonObject& spec);
    static void        localizeEncoding(QJsonObject& spec);
    static void        localizeAxis(QJsonObject& axisCfg);
    static void        localizeLegend(QJsonObject& legendCfg);
    static void        localizeHeaders(QJsonObject& spec);
    static void        localizeNested(QJsonObject& spec);

    static bool        isObjectWith(const QJsonObject& obj, const char* key);
    static QString translateLabelPreservingUnits(const QString& src);
    static QString translateWholeWords(const QString& s);
};

} // namespace vega
