#include "FileIO.h"
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QSaveFile>
#include <QDebug>

namespace FileIO {

QString canonicalize(const QString& path)
{
    if (path.isEmpty()) {
        return QString();
    }

    QFileInfo info(path);
    QString canonical = info.canonicalFilePath();
    if (canonical.isEmpty()) {
        // Fallback to absolute path if canonical fails (e.g., path doesn't exist yet)
        canonical = info.absoluteFilePath();
    }
    return canonical;
}

bool ensureDir(const QString& dirPath, QString* errOut)
{
    if (dirPath.isEmpty()) {
        if (errOut) {
            *errOut = "Directory path is empty";
        }
        qWarning() << "[IO] Directory path is empty";
        return false;
    }

    QDir dir;
    if (!dir.mkpath(dirPath)) {
        QString error = QString("Failed to create directory: %1").arg(dirPath);
        if (errOut) {
            *errOut = error;
        }
        qWarning() << "[IO]" << dirPath << error;
        return false;
    }

    return true;
}

bool readTextFile(const QString& path, QString* out, QString* errOut)
{
    if (path.isEmpty()) {
        if (errOut) {
            *errOut = "File path is empty";
        }
        qWarning() << "[IO] File path is empty";
        return false;
    }

    if (!out) {
        qWarning() << "[IO] readTextFile called with null output pointer";
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString error = QString("Failed to open file: %1").arg(file.errorString());
        if (errOut) {
            *errOut = error;
        }
        qWarning() << "[IO]" << path << error;
        return false;
    }

    QTextStream stream(&file);
    *out = stream.readAll();

    if (file.error() != QFile::NoError) {
        QString error = QString("Read error: %1").arg(file.errorString());
        if (errOut) {
            *errOut = error;
        }
        qWarning() << "[IO]" << path << error;
        return false;
    }

    return true;
}

bool writeTextFileAtomic(const QString& path, const QString& text, QString* errOut)
{
    if (path.isEmpty()) {
        if (errOut) {
            *errOut = "File path is empty";
        }
        qWarning() << "[IO] File path is empty";
        return false;
    }

    // Ensure parent directory exists
    QFileInfo fileInfo(path);
    QString parentDir = fileInfo.absolutePath();
    if (!parentDir.isEmpty()) {
        QString dirError;
        if (!ensureDir(parentDir, &dirError)) {
            if (errOut) {
                *errOut = QString("Failed to create parent directory: %1").arg(dirError);
            }
            return false;
        }
    }

    QSaveFile saveFile(path);
    if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QString error = QString("Failed to open file for writing: %1").arg(saveFile.errorString());
        if (errOut) {
            *errOut = error;
        }
        qWarning() << "[IO]" << path << error;
        return false;
    }

    QTextStream stream(&saveFile);
    stream << text;

    if (stream.status() != QTextStream::Ok) {
        QString error = "Write error: stream status indicates failure";
        if (errOut) {
            *errOut = error;
        }
        qWarning() << "[IO]" << path << error;
        saveFile.cancelWriting();
        return false;
    }

    if (!saveFile.commit()) {
        QString error = QString("Failed to commit file: %1").arg(saveFile.errorString());
        if (errOut) {
            *errOut = error;
        }
        qWarning() << "[IO]" << path << error;
        return false;
    }

    return true;
}

} // namespace FileIO

