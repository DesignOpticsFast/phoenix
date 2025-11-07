#pragma once

#include <QString>

class QApplication;

namespace i18n {
struct Result {
    QString lang;
    QString locale;
    QString qtBasePath;
    QString appPath;
};

Result setup(QApplication& app);
QString localeForLanguage(const QString& shortLang);
} // namespace i18n
