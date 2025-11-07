#pragma once

#include <QString>

class QApplication;

namespace i18nselftest {

// Runs the headless i18n self-test. Returns 0 on success.
int run(QApplication& app, const QString& explicitLangCode);

} // namespace i18nselftest
