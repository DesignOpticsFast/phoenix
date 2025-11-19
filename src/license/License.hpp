#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <optional>

class License {
public:
    License() = default;
    
    License(const QString& subject,
            const QStringList& features,
            const QDateTime& issuedAt,
            const std::optional<QDateTime>& expiresAt = std::nullopt);
    
    QString subject() const { return subject_; }
    QStringList features() const { return features_; }
    QDateTime issuedAt() const { return issuedAt_; }
    std::optional<QDateTime> expiresAt() const { return expiresAt_; }
    
    // Check if license is expired (compared to current UTC time or provided time)
    bool expired(const QDateTime& now = QDateTime::currentDateTimeUtc()) const;
    
    // Check if license has a specific feature
    bool hasFeature(const QString& feature) const;
    
    // Check if license is valid (not expired and has at least one feature)
    bool isValid(const QDateTime& now = QDateTime::currentDateTimeUtc()) const;

private:
    QString subject_;
    QStringList features_;
    QDateTime issuedAt_;
    std::optional<QDateTime> expiresAt_;
};

