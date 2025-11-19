#include "License.hpp"

License::License(const QString& subject,
                 const QStringList& features,
                 const QDateTime& issuedAt,
                 const std::optional<QDateTime>& expiresAt)
    : subject_(subject)
    , features_(features)
    , issuedAt_(issuedAt)
    , expiresAt_(expiresAt)
{
}

bool License::expired(const QDateTime& now) const {
    if (!expiresAt_.has_value()) {
        return false;  // No expiry means never expires
    }
    return now > expiresAt_.value();
}

bool License::hasFeature(const QString& feature) const {
    return features_.contains(feature);
}

bool License::isValid(const QDateTime& now) const {
    return !expired(now) && !features_.isEmpty();
}

