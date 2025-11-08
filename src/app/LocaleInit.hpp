#pragma once

#include <QString>
#include <QSettings>

class QApplication;

namespace i18n {
struct Result {
    QString lang;
    QString locale;
    QString qtBasePath;
    QString appPath;
};

Result setup(QApplication& app);
Result setup(QApplication& app, QSettings& settings);
QString localeForLanguage(const QString& shortLang);
} // namespace i18n
