#pragma once

#include <QString>
#include <QSettings>
#include "SettingsKeys.h"

enum class AnalysisRunMode {
    AutoRunOnOpen,      // "auto" (default) - Auto-run when feature says autoRunOnOpen=true
    ShowOptionsFirst    // "options" - Always show options first, require manual Run
};

// Convert enum to string for storage
inline QString analysisRunModeToString(AnalysisRunMode mode) {
    switch (mode) {
    case AnalysisRunMode::AutoRunOnOpen:
        return QStringLiteral("auto");
    case AnalysisRunMode::ShowOptionsFirst:
        return QStringLiteral("options");
    }
    return QStringLiteral("auto"); // Fallback
}

// Convert string to enum (with validation)
inline AnalysisRunMode analysisRunModeFromString(const QString& str,
                                                 AnalysisRunMode defaultMode = AnalysisRunMode::AutoRunOnOpen) {
    if (str == QLatin1String("auto"))
        return AnalysisRunMode::AutoRunOnOpen;
    if (str == QLatin1String("options"))
        return AnalysisRunMode::ShowOptionsFirst;
    
    // Invalid value - fall back to default
    // Optional: qWarning() << "Invalid analysis run mode:" << str << ", using default";
    return defaultMode;
}

// Get analysis run mode from settings (default: AutoRunOnOpen)
inline AnalysisRunMode getAnalysisRunMode(QSettings& settings) {
    const QString value = settings.value(PhxKeys::ANALYSIS_RUN_MODE,
                                         QStringLiteral("auto")).toString();
    return analysisRunModeFromString(value);
}

// Set analysis run mode in settings
inline void setAnalysisRunMode(QSettings& settings, AnalysisRunMode mode) {
    settings.setValue(PhxKeys::ANALYSIS_RUN_MODE, analysisRunModeToString(mode));
}


