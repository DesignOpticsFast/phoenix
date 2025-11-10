#pragma once

#include <QtGlobal>

namespace phx {

class MemoryMonitor
{
public:
    // Returns resident memory in megabytes, or -1 on failure.
    static double getResidentMemoryMB();
};

} // namespace phx


