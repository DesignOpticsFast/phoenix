#pragma once

#include <QDebug>

#ifndef PHX_BOOT_TRACE
#define PHX_BOOT_TRACE(tag) qInfo() << "[boot]" << tag;
#endif

