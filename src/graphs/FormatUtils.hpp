#pragma once

#include <QLocale>
#include <QString>

namespace fmt {

inline QString toLocaleString(double value, int decimals = 2)
{
    return QLocale().toString(value, 'f', decimals);
}

} // namespace fmt
