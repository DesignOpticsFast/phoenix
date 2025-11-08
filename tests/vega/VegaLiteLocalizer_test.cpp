#include <QtTest/QtTest>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>

#include "graphs/VegaLiteLocalizer.hpp"

class VegaLocalizerTest : public QObject
{
    Q_OBJECT

private slots:
    void injectsLocale_de();
    void preservesSchemaFields();
    void translatesAxisTitles_preservesUnits_de();
    void neverTranslatesIdentifiersOrLabelExpr();
};

void VegaLocalizerTest::injectsLocale_de()
{
    const QLocale previous = QLocale();
    QLocale::setDefault(QLocale(QStringLiteral("de_DE")));

    QJsonObject input;
    input.insert(QStringLiteral("data"), QJsonObject{ { QStringLiteral("values"), QJsonArray{ 1, 2, 3 } } });

    const QJsonObject output = vega::VegaLiteLocalizer::localize(input);
    QVERIFY(output.contains(QStringLiteral("config")));

    const QJsonObject cfg = output.value(QStringLiteral("config")).toObject();
    QCOMPARE(cfg.value(QStringLiteral("locale")).toString(), QStringLiteral("de_DE"));

    if (qEnvironmentVariableIsSet("VEGA_LOCALIZER_DUMP")) {
        const QByteArray payload = QJsonDocument(cfg).toJson(QJsonDocument::Compact);
        qInfo().noquote() << payload;
    }

    const QJsonObject numberLocale = cfg.value(QStringLiteral("numberFormatLocale")).toObject();
    QCOMPARE(numberLocale.value(QStringLiteral("decimal")).toString(), QStringLiteral(","));
    QCOMPARE(numberLocale.value(QStringLiteral("thousands")).toString(), QStringLiteral("."));

    const QJsonObject timeLocale = cfg.value(QStringLiteral("timeFormatLocale")).toObject();
    QVERIFY(timeLocale.value(QStringLiteral("months")).isArray());
    QVERIFY(timeLocale.value(QStringLiteral("shortMonths")).isArray());

    QLocale::setDefault(previous);
}

void VegaLocalizerTest::preservesSchemaFields()
{
    const QLocale previous = QLocale();
    QLocale::setDefault(QLocale(QStringLiteral("en_US")));

    QJsonObject input;
    input.insert(QStringLiteral("encoding"), QJsonObject{
        { QStringLiteral("x"), QJsonObject{
            { QStringLiteral("field"), QStringLiteral("distance") },
            { QStringLiteral("type"), QStringLiteral("quantitative") },
            { QStringLiteral("axis"), QJsonObject{} }
        } },
        { QStringLiteral("color"), QJsonObject{
            { QStringLiteral("field"), QStringLiteral("material") },
            { QStringLiteral("legend"), QJsonObject{} }
        } }
    });

    const QJsonObject output = vega::VegaLiteLocalizer::localize(input);
    QVERIFY(output.contains(QStringLiteral("encoding")));

    const QJsonObject encoding = output.value(QStringLiteral("encoding")).toObject();
    QCOMPARE(encoding.value(QStringLiteral("x")).toObject().value(QStringLiteral("field")).toString(), QStringLiteral("distance"));
    QCOMPARE(encoding.value(QStringLiteral("x")).toObject().value(QStringLiteral("type")).toString(), QStringLiteral("quantitative"));
    QVERIFY(encoding.value(QStringLiteral("x")).toObject().value(QStringLiteral("axis")).isObject());
    QVERIFY(encoding.value(QStringLiteral("color")).toObject().value(QStringLiteral("legend")).isObject());

    QLocale::setDefault(previous);
}

void VegaLocalizerTest::translatesAxisTitles_preservesUnits_de()
{
    const QLocale previous = QLocale();
    QLocale::setDefault(QLocale(QStringLiteral("de_DE")));

    QJsonObject input;
    input.insert(QStringLiteral("encoding"), QJsonObject{
        { QStringLiteral("x"), QJsonObject{{ QStringLiteral("axis"), QJsonObject{{ QStringLiteral("title"), QStringLiteral("Distance (mm)") }} }} },
        { QStringLiteral("y"), QJsonObject{{ QStringLiteral("axis"), QJsonObject{{ QStringLiteral("title"), QStringLiteral("Stress (MPa)") }} }} }
    });

    const QJsonObject output = vega::VegaLiteLocalizer::localize(input);
    const QJsonObject encoding = output.value(QStringLiteral("encoding")).toObject();
    QCOMPARE(encoding.value(QStringLiteral("x")).toObject().value(QStringLiteral("axis")).toObject().value(QStringLiteral("title")).toString(), QStringLiteral("Abstand (mm)"));
    QCOMPARE(encoding.value(QStringLiteral("y")).toObject().value(QStringLiteral("axis")).toObject().value(QStringLiteral("title")).toString(), QStringLiteral("Spannung (MPa)"));

    QLocale::setDefault(previous);
}

void VegaLocalizerTest::neverTranslatesIdentifiersOrLabelExpr()
{
    const QLocale previous = QLocale();
    QLocale::setDefault(QLocale(QStringLiteral("de_DE")));

    QJsonObject input;
    input.insert(QStringLiteral("encoding"), QJsonObject{
        { QStringLiteral("color"), QJsonObject{{ QStringLiteral("legend"), QJsonObject{{ QStringLiteral("title"), QStringLiteral("N-BK7") }} }} },
        { QStringLiteral("x"), QJsonObject{{ QStringLiteral("axis"), QJsonObject{
            { QStringLiteral("title"), QStringLiteral("Distance") },
            { QStringLiteral("labelExpr"), QStringLiteral("datum.label + ' mm'") }
        } }} }
    });

    const QJsonObject output = vega::VegaLiteLocalizer::localize(input);
    const QJsonObject encoding = output.value(QStringLiteral("encoding")).toObject();

    QCOMPARE(encoding.value(QStringLiteral("color")).toObject().value(QStringLiteral("legend")).toObject().value(QStringLiteral("title")).toString(), QStringLiteral("N-BK7"));

    const QJsonObject axis = encoding.value(QStringLiteral("x")).toObject().value(QStringLiteral("axis")).toObject();
    QCOMPARE(axis.value(QStringLiteral("labelExpr")).toString(), QStringLiteral("datum.label + ' mm'"));
    QCOMPARE(axis.value(QStringLiteral("title")).toString(), QStringLiteral("Abstand"));

    QLocale::setDefault(previous);
}

QTEST_MAIN(VegaLocalizerTest)
#include "VegaLiteLocalizer_test.moc"
