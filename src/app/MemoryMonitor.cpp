#include "MemoryMonitor.hpp"

#include <QDebug>
#include <QString>
#include <QByteArray>
#include <cctype>

#if defined(Q_OS_LINUX)
#include <QFile>
#elif defined(Q_OS_MACOS)
#include <mach/mach.h>
#include <mach/task_info.h>
#elif defined(Q_OS_WIN)
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#endif

namespace phx {

namespace {

constexpr double kBytesPerMegabyte = 1024.0 * 1024.0;

double bytesToMegabytes(quint64 bytes)
{
    return static_cast<double>(bytes) / kBytesPerMegabyte;
}

void logFailureOnce(const char* platformTag, const char* reason)
{
    static bool logged = false;
    if (logged)
        return;
    logged = true;
    qWarning() << "[MemoryMonitor]" << platformTag << reason;
}

} // namespace

double MemoryMonitor::getResidentMemoryMB()
{
#if defined(Q_OS_LINUX)
    QFile statusFile(QStringLiteral("/proc/self/status"));
    if (!statusFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        logFailureOnce("linux", "unable to open /proc/self/status");
        return -1.0;
    }

    while (true) {
        const QByteArray line = statusFile.readLine();
        if (line.isEmpty()) {
            if (statusFile.atEnd()) {
                break;
            }
            continue;
        }

        if (line.startsWith("VmRSS:")) {
            const QByteArray tail = line.mid(static_cast<int>(sizeof("VmRSS:") - 1)).trimmed();
            QByteArray digits;
            for (const char ch : tail) {
                if (std::isdigit(static_cast<unsigned char>(ch))) {
                    digits.append(ch);
                } else if (!digits.isEmpty()) {
                    break;
                }
            }

            if (!digits.isEmpty()) {
                bool ok = false;
                const quint64 valueKb = digits.toULongLong(&ok);
                if (ok) {
                    return static_cast<double>(valueKb) / 1024.0;
                }
            }
            break;
        }
    }

    logFailureOnce("linux", "VmRSS not found in /proc/self/status");
    return -1.0;

#elif defined(Q_OS_MACOS)
    task_basic_info_64 info;
    mach_msg_type_number_t count = TASK_BASIC_INFO_64_COUNT;
    kern_return_t kr = task_info(mach_task_self(),
                                 TASK_BASIC_INFO_64,
                                 reinterpret_cast<task_info_t>(&info),
                                 &count);
    if (kr != KERN_SUCCESS) {
        logFailureOnce("macos", "task_info(TASK_BASIC_INFO_64) failed");
        return -1.0;
    }

    return bytesToMegabytes(static_cast<quint64>(info.resident_size));

#elif defined(Q_OS_WIN)
    PROCESS_MEMORY_COUNTERS counters;
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters))) {
        logFailureOnce("windows", "GetProcessMemoryInfo failed");
        return -1.0;
    }

    return bytesToMegabytes(static_cast<quint64>(counters.WorkingSetSize));

#else
    logFailureOnce("unknown", "unsupported platform");
    return -1.0;
#endif
}

} // namespace phx


