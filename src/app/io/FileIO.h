#pragma once

#include <QString>

namespace FileIO {
    // Canonicalize a file path (uses QFileInfo::canonicalFilePath() with fallback to absolutePath)
    QString canonicalize(const QString& path);

    // Ensure a directory exists (creates parent directories if needed)
    // Returns true on success, false on failure. If errOut is provided, fills it with error message.
    bool ensureDir(const QString& dirPath, QString* errOut = nullptr);

    // Read a text file into a QString
    // Returns true on success, false on failure. If errOut is provided, fills it with error message.
    bool readTextFile(const QString& path, QString* out, QString* errOut = nullptr);

    // Write text to a file atomically using QSaveFile
    // Returns true on success, false on failure. If errOut is provided, fills it with error message.
    bool writeTextFileAtomic(const QString& path, const QString& text, QString* errOut = nullptr);
}

