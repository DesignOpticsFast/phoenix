#pragma once

#include <QString>

struct AnalysisProgress {
    double progressPercent;  // 0.0 to 100.0
    QString status;          // e.g., "Computing...", "Receiving data...", "Done"
    
    AnalysisProgress() : progressPercent(0.0) {}
    AnalysisProgress(double percent, const QString& statusText)
        : progressPercent(percent), status(statusText) {}
};

Q_DECLARE_METATYPE(AnalysisProgress)

