#include "LicenseVerifier.hpp"

#include "common/canonical_json.hpp"
#include <sodium.h>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QProcessEnvironment>
#include <QDebug>
#include <QDateTime>
#include <QRegularExpression>

using namespace phoenix::json;

namespace {
    // Ensure sodium_init() is called once
    struct SodiumInitializer {
        SodiumInitializer() {
            if (sodium_init() < 0) {
                qWarning() << "LicenseVerifier: sodium_init() failed";
            }
        }
    };
    static SodiumInitializer sodiumInit;
    
    // Convert QJsonValue to CanonicalValue
    CanonicalValue jsonValueToCanonical(const QJsonValue& value) {
        if (value.isNull()) {
            return CanonicalValue{nullptr};
        } else if (value.isBool()) {
            return CanonicalValue{value.toBool()};
        } else if (value.isDouble()) {
            // Check if it's an integer
            double d = value.toDouble();
            qint64 i = static_cast<qint64>(d);
            if (d == static_cast<double>(i)) {
                return CanonicalValue{int64_t{i}};
            }
            return CanonicalValue{d};
        } else if (value.isString()) {
            return CanonicalValue{value.toString().toStdString()};
        } else if (value.isArray()) {
            std::vector<CanonicalValue> arr;
            QJsonArray jsonArray = value.toArray();
            for (const QJsonValue& item : jsonArray) {
                arr.push_back(jsonValueToCanonical(item));
            }
            return CanonicalValue{arr};
        } else if (value.isObject()) {
            std::map<std::string, CanonicalValue> obj;
            QJsonObject jsonObj = value.toObject();
            for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
                obj[it.key().toStdString()] = jsonValueToCanonical(it.value());
            }
            return CanonicalValue{obj};
        }
        return CanonicalValue{nullptr};
    }
}

LicenseVerifier::LicenseVerifier(const QByteArray& publicKey)
    : publicKey_(publicKey)
{
    if (publicKey_.size() != ED25519_PUBLIC_KEY_SIZE) {
        qWarning() << "LicenseVerifier: Invalid public key size:" << publicKey_.size()
                   << "expected:" << ED25519_PUBLIC_KEY_SIZE;
    }
}

LicenseVerifier::LicenseVerifier(const std::array<unsigned char, ED25519_PUBLIC_KEY_SIZE>& publicKey)
    : publicKey_(reinterpret_cast<const char*>(publicKey.data()), ED25519_PUBLIC_KEY_SIZE)
{
}

std::optional<License> LicenseVerifier::loadAndVerify(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "LicenseVerifier: Failed to open license file:" << path
                   << "error:" << file.errorString();
        return std::nullopt;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "LicenseVerifier: Failed to parse JSON:" << parseError.errorString();
        return std::nullopt;
    }
    
    if (!doc.isObject()) {
        qWarning() << "LicenseVerifier: JSON document is not an object";
        return std::nullopt;
    }
    
    QJsonObject jsonObj = doc.object();
    
    // Extract signature
    auto [docWithoutSig, signature] = extractSignature(jsonObj);
    
    if (signature.isEmpty()) {
        qWarning() << "LicenseVerifier: Missing or invalid signature field";
        return std::nullopt;
    }
    
    // Verify signature
    if (!verifyDocument(docWithoutSig, signature)) {
        qWarning() << "LicenseVerifier: Signature verification failed";
        return std::nullopt;
    }
    
    // Parse and return license
    return parseLicense(docWithoutSig);
}

std::optional<License> LicenseVerifier::verifyDocument(const QJsonObject& doc, const QByteArray& signature) {
    // Validate schema first
    if (!validateSchema(doc)) {
        qWarning() << "LicenseVerifier: Schema validation failed";
        return std::nullopt;
    }
    
    // Canonicalize document
    QByteArray canonicalJson = canonicalizeJsonObject(doc);
    
    // Verify signature
    if (!verifyCanonicalized(canonicalJson, signature)) {
        qWarning() << "LicenseVerifier: Signature verification failed";
        return std::nullopt;
    }
    
    // Parse and return license
    return parseLicense(doc);
}

std::pair<QJsonObject, QByteArray> LicenseVerifier::extractSignature(const QJsonObject& doc) {
    QJsonObject docWithoutSig = doc;
    QByteArray signature;
    
    if (doc.contains("signature")) {
        QJsonValue sigValue = doc.value("signature");
        if (sigValue.isString()) {
            QString sigStr = sigValue.toString();
            signature = QByteArray::fromBase64(sigStr.toUtf8());
            docWithoutSig.remove("signature");
        }
    }
    
    return {docWithoutSig, signature};
}

QByteArray LicenseVerifier::canonicalizeJsonObject(const QJsonObject& doc) {
    // Convert QJsonObject to CanonicalValue
    std::map<std::string, CanonicalValue> canonicalObj;
    for (auto it = doc.begin(); it != doc.end(); ++it) {
        canonicalObj[it.key().toStdString()] = jsonValueToCanonical(it.value());
    }
    
    CanonicalValue canonicalValue{canonicalObj};
    
    // Canonicalize
    std::string canonicalStr = to_canonical_json(canonicalValue);
    
    // Convert to QByteArray
    return QByteArray(canonicalStr.data(), static_cast<int>(canonicalStr.size()));
}

bool LicenseVerifier::verifyCanonicalized(const QByteArray& canonicalJson, const QByteArray& signature) {
    if (publicKey_.size() != ED25519_PUBLIC_KEY_SIZE) {
        qWarning() << "LicenseVerifier: Invalid public key size for verification";
        return false;
    }
    
    if (signature.size() != crypto_sign_BYTES) {
        qWarning() << "LicenseVerifier: Invalid signature size:" << signature.size()
                   << "expected:" << crypto_sign_BYTES;
        return false;
    }
    
    int result = crypto_sign_verify_detached(
        reinterpret_cast<const unsigned char*>(signature.data()),
        reinterpret_cast<const unsigned char*>(canonicalJson.data()),
        canonicalJson.size(),
        reinterpret_cast<const unsigned char*>(publicKey_.data())
    );
    
    return result == 0;
}

std::optional<License> LicenseVerifier::parseLicense(const QJsonObject& doc) {
    // Validate schema
    if (!validateSchema(doc)) {
        return std::nullopt;
    }
    
    // Extract required fields
    QString subject = doc.value("subject").toString();
    QDateTime issuedAt = QDateTime::fromString(doc.value("issued_at").toString(), Qt::ISODate);
    
    if (subject.isEmpty()) {
        qWarning() << "LicenseVerifier: Empty subject field";
        return std::nullopt;
    }
    
    if (!issuedAt.isValid()) {
        qWarning() << "LicenseVerifier: Invalid issued_at field";
        return std::nullopt;
    }
    
    // Extract optional expires_at
    std::optional<QDateTime> expiresAt;
    if (doc.contains("expires_at")) {
        QJsonValue expiresValue = doc.value("expires_at");
        if (expiresValue.isNull()) {
            expiresAt = std::nullopt;
        } else if (expiresValue.isString()) {
            QDateTime dt = QDateTime::fromString(expiresValue.toString(), Qt::ISODate);
            if (dt.isValid()) {
                expiresAt = dt;
            } else {
                qWarning() << "LicenseVerifier: Invalid expires_at format";
                return std::nullopt;
            }
        } else {
            qWarning() << "LicenseVerifier: expires_at must be string or null";
            return std::nullopt;
        }
    }
    
    // Extract features
    QStringList features;
    if (doc.contains("features")) {
        QJsonValue featuresValue = doc.value("features");
        if (featuresValue.isArray()) {
            QJsonArray featuresArray = featuresValue.toArray();
            for (const QJsonValue& featureValue : featuresArray) {
                if (featureValue.isString()) {
                    features.append(featureValue.toString());
                } else {
                    qWarning() << "LicenseVerifier: features array contains non-string value";
                    return std::nullopt;
                }
            }
        } else {
            qWarning() << "LicenseVerifier: features must be an array";
            return std::nullopt;
        }
    }
    
    return License(subject, features, issuedAt, expiresAt);
}

bool LicenseVerifier::validateSchema(const QJsonObject& doc) {
    // Check required fields
    if (!doc.contains("subject") || !doc.value("subject").isString()) {
        qWarning() << "LicenseVerifier: Missing or invalid 'subject' field";
        return false;
    }
    
    if (!doc.contains("issued_at") || !doc.value("issued_at").isString()) {
        qWarning() << "LicenseVerifier: Missing or invalid 'issued_at' field";
        return false;
    }
    
    // Validate optional expires_at
    if (doc.contains("expires_at")) {
        QJsonValue expiresValue = doc.value("expires_at");
        if (!expiresValue.isNull() && !expiresValue.isString()) {
            qWarning() << "LicenseVerifier: 'expires_at' must be string or null";
            return false;
        }
    }
    
    // Validate optional features
    if (doc.contains("features")) {
        QJsonValue featuresValue = doc.value("features");
        if (!featuresValue.isArray()) {
            qWarning() << "LicenseVerifier: 'features' must be an array";
            return false;
        }
        QJsonArray featuresArray = featuresValue.toArray();
        for (const QJsonValue& featureValue : featuresArray) {
            if (!featureValue.isString()) {
                qWarning() << "LicenseVerifier: 'features' array must contain only strings";
                return false;
            }
        }
    }
    
    return true;
}

std::optional<LicenseVerifier> LicenseVerifier::fromEnv() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString publicKeyStr = env.value("PHOENIX_LICENSE_PUBLIC_KEY");
    
    if (publicKeyStr.isEmpty()) {
        qDebug() << "LicenseVerifier::fromEnv: PHOENIX_LICENSE_PUBLIC_KEY not set";
        return std::nullopt;
    }
    
    // Decode base64
    QByteArray publicKey = QByteArray::fromBase64(publicKeyStr.toUtf8());
    
    if (publicKey.size() != ED25519_PUBLIC_KEY_SIZE) {
        qWarning() << "LicenseVerifier::fromEnv: Invalid public key size:" << publicKey.size()
                   << "expected:" << ED25519_PUBLIC_KEY_SIZE;
        return std::nullopt;
    }
    
    return LicenseVerifier(publicKey);
}

