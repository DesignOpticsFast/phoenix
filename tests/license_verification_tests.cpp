#include <QtTest/QtTest>
#include "license/License.hpp"
#include "license/LicenseVerifier.hpp"
#include <sodium.h>
#include "common/canonical_json.hpp"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QTemporaryDir>
#include <QDir>
#include <QDateTime>
#include <QDebug>

using namespace phoenix::json;

namespace TestLicenseHelpers {
    // Generate test keypair using libsodium
    std::pair<QByteArray, QByteArray> generateTestKeypair() {
        std::array<unsigned char, crypto_sign_PUBLICKEYBYTES> pk{};
        std::array<unsigned char, crypto_sign_SECRETKEYBYTES> sk{};
        
        if (crypto_sign_keypair(pk.data(), sk.data()) != 0) {
            return {QByteArray(), QByteArray()};
        }
        
        QByteArray publicKey(reinterpret_cast<const char*>(pk.data()), crypto_sign_PUBLICKEYBYTES);
        QByteArray privateKey(reinterpret_cast<const char*>(sk.data()), crypto_sign_SECRETKEYBYTES);
        
        return {publicKey, privateKey};
    }
    
    // Convert QJsonValue to CanonicalValue (same as LicenseVerifier)
    CanonicalValue jsonValueToCanonical(const QJsonValue& value) {
        if (value.isNull()) {
            return CanonicalValue{nullptr};
        } else if (value.isBool()) {
            return CanonicalValue{value.toBool()};
        } else if (value.isDouble()) {
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
    
    // Sign canonical JSON bytes with private key
    QByteArray signCanonicalJson(const QByteArray& canonicalJson, const QByteArray& privateKey) {
        std::array<unsigned char, crypto_sign_BYTES> sig{};
        unsigned long long siglen = 0;
        
        if (crypto_sign_detached(sig.data(), &siglen,
                                 reinterpret_cast<const unsigned char*>(canonicalJson.data()),
                                 canonicalJson.size(),
                                 reinterpret_cast<const unsigned char*>(privateKey.data())) != 0) {
            return QByteArray();
        }
        
        return QByteArray(reinterpret_cast<const char*>(sig.data()), static_cast<int>(siglen));
    }
    
    // Create and sign license JSON
    QJsonObject createSignedLicense(
        const QByteArray& privateKey,
        const QString& subject,
        const QStringList& features,
        const QDateTime& issuedAt,
        const std::optional<QDateTime>& expiresAt = std::nullopt
    ) {
        // Create JSON without signature
        QJsonObject license;
        license["subject"] = subject;
        license["issued_at"] = issuedAt.toString(Qt::ISODate);
        if (expiresAt.has_value()) {
            license["expires_at"] = expiresAt.value().toString(Qt::ISODate);
        } else {
            license["expires_at"] = QJsonValue::Null;
        }
        
        QJsonArray featuresArray;
        for (const QString& feature : features) {
            featuresArray.append(feature);
        }
        license["features"] = featuresArray;
        
        // Canonicalize (without signature)
        std::map<std::string, CanonicalValue> canonicalObj;
        for (auto it = license.begin(); it != license.end(); ++it) {
            canonicalObj[it.key().toStdString()] = jsonValueToCanonical(it.value());
        }
        CanonicalValue canonicalValue{canonicalObj};
        std::string canonicalStr = to_canonical_json(canonicalValue);
        QByteArray canonicalJson(canonicalStr.data(), static_cast<int>(canonicalStr.size()));
        
        // Sign
        QByteArray signature = signCanonicalJson(canonicalJson, privateKey);
        if (signature.isEmpty()) {
            return QJsonObject();
        }
        
        // Add signature
        license["signature"] = QString::fromUtf8(signature.toBase64());
        
        return license;
    }
    
    // Write JSON to temporary file
    QString writeTempLicenseFile(const QJsonObject& licenseJson, QTemporaryDir& tempDir) {
        QString fileName = tempDir.path() + "/license.json";
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            return QString();
        }
        
        QJsonDocument doc(licenseJson);
        qint64 written = file.write(doc.toJson());
        file.close();
        
        if (written <= 0) {
            return QString();
        }
        
        return fileName;
    }
}

class LicenseVerificationTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Ensure sodium is initialized
        if (sodium_init() < 0) {
            QFAIL("sodium_init() failed");
        }
    }
    
    void testValidLicenseAccepted() {
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        QVERIFY(!publicKey.isEmpty());
        QVERIFY(!privateKey.isEmpty());
        
        QString subject = "test@example.com";
        QStringList features = {"feature_a", "feature_b"};
        QDateTime issuedAt = QDateTime::currentDateTimeUtc();
        QDateTime expiresAt = issuedAt.addYears(1);
        
        QJsonObject licenseJson = TestLicenseHelpers::createSignedLicense(
            privateKey, subject, features, issuedAt, expiresAt
        );
        QVERIFY(!licenseJson.isEmpty());
        
        QTemporaryDir tempDir;
        QString licensePath = TestLicenseHelpers::writeTempLicenseFile(licenseJson, tempDir);
        QVERIFY(!licensePath.isEmpty());
        
        LicenseVerifier verifier(publicKey);
        auto license = verifier.loadAndVerify(licensePath);
        
        QVERIFY(license.has_value());
        QCOMPARE(license->subject(), subject);
        QCOMPARE(license->features(), features);
        QVERIFY(license->hasFeature("feature_a"));
        QVERIFY(license->hasFeature("feature_b"));
        QVERIFY(!license->expired());
        QVERIFY(license->isValid());
    }
    
    void testTamperedLicenseRejected() {
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        
        QString subject = "test@example.com";
        QStringList features = {"feature_a"};
        QDateTime issuedAt = QDateTime::currentDateTimeUtc();
        
        QJsonObject licenseJson = TestLicenseHelpers::createSignedLicense(
            privateKey, subject, features, issuedAt
        );
        
        // Tamper with the license (modify subject)
        licenseJson["subject"] = "tampered@example.com";
        // Keep original signature
        
        QTemporaryDir tempDir;
        QString licensePath = TestLicenseHelpers::writeTempLicenseFile(licenseJson, tempDir);
        
        LicenseVerifier verifier(publicKey);
        auto license = verifier.loadAndVerify(licensePath);
        
        QVERIFY(!license.has_value());  // Should fail verification
    }
    
    void testWrongKeyRejected() {
        auto [publicKey1, privateKey1] = TestLicenseHelpers::generateTestKeypair();
        auto [publicKey2, privateKey2] = TestLicenseHelpers::generateTestKeypair();
        
        QString subject = "test@example.com";
        QStringList features = {"feature_a"};
        QDateTime issuedAt = QDateTime::currentDateTimeUtc();
        
        // Sign with keypair1
        QJsonObject licenseJson = TestLicenseHelpers::createSignedLicense(
            privateKey1, subject, features, issuedAt
        );
        
        QTemporaryDir tempDir;
        QString licensePath = TestLicenseHelpers::writeTempLicenseFile(licenseJson, tempDir);
        
        // Try to verify with keypair2
        LicenseVerifier verifier(publicKey2);
        auto license = verifier.loadAndVerify(licensePath);
        
        QVERIFY(!license.has_value());  // Should fail verification
    }
    
    void testExpiredLicense() {
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        
        QString subject = "test@example.com";
        QStringList features = {"feature_a"};
        QDateTime issuedAt = QDateTime::currentDateTimeUtc().addYears(-2);
        QDateTime expiresAt = issuedAt.addYears(1);  // Expired 1 year ago
        
        QJsonObject licenseJson = TestLicenseHelpers::createSignedLicense(
            privateKey, subject, features, issuedAt, expiresAt
        );
        
        QTemporaryDir tempDir;
        QString licensePath = TestLicenseHelpers::writeTempLicenseFile(licenseJson, tempDir);
        
        LicenseVerifier verifier(publicKey);
        auto license = verifier.loadAndVerify(licensePath);
        
        QVERIFY(license.has_value());  // Verification succeeds
        QVERIFY(license->expired());   // But license is expired
        QVERIFY(!license->isValid());  // So not valid
    }
    
    void testNoExpiryLicense() {
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        
        QString subject = "test@example.com";
        QStringList features = {"feature_a"};
        QDateTime issuedAt = QDateTime::currentDateTimeUtc();
        
        QJsonObject licenseJson = TestLicenseHelpers::createSignedLicense(
            privateKey, subject, features, issuedAt, std::nullopt
        );
        
        QTemporaryDir tempDir;
        QString licensePath = TestLicenseHelpers::writeTempLicenseFile(licenseJson, tempDir);
        
        LicenseVerifier verifier(publicKey);
        auto license = verifier.loadAndVerify(licensePath);
        
        QVERIFY(license.has_value());
        QVERIFY(!license->expired());
        QVERIFY(license->isValid());
    }
    
    void testMissingSignatureRejected() {
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        
        // Create license JSON without signature
        QJsonObject licenseJson;
        licenseJson["subject"] = "test@example.com";
        licenseJson["issued_at"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        licenseJson["features"] = QJsonArray();
        // No signature field
        
        QTemporaryDir tempDir;
        QString licensePath = TestLicenseHelpers::writeTempLicenseFile(licenseJson, tempDir);
        
        LicenseVerifier verifier(publicKey);
        auto license = verifier.loadAndVerify(licensePath);
        
        QVERIFY(!license.has_value());  // Should fail
    }
    
    void testInvalidJsonRejected() {
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        LicenseVerifier verifier(publicKey);
        
        // Test non-existent file
        auto license1 = verifier.loadAndVerify("/nonexistent/path/license.json");
        QVERIFY(!license1.has_value());
        
        // Test invalid JSON file
        QTemporaryDir tempDir;
        QString invalidPath = tempDir.path() + "/invalid.json";
        QFile invalidFile(invalidPath);
        invalidFile.open(QIODevice::WriteOnly);
        invalidFile.write("not valid json {");
        invalidFile.close();
        
        auto license2 = verifier.loadAndVerify(invalidPath);
        QVERIFY(!license2.has_value());
    }
    
    void testFeatureCheck() {
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        
        QStringList features = {"feature_a", "feature_b"};
        QJsonObject licenseJson = TestLicenseHelpers::createSignedLicense(
            privateKey, "test@example.com", features, QDateTime::currentDateTimeUtc()
        );
        
        QTemporaryDir tempDir;
        QString licensePath = TestLicenseHelpers::writeTempLicenseFile(licenseJson, tempDir);
        
        LicenseVerifier verifier(publicKey);
        auto license = verifier.loadAndVerify(licensePath);
        
        QVERIFY(license.has_value());
        QVERIFY(license->hasFeature("feature_a"));
        QVERIFY(license->hasFeature("feature_b"));
        QVERIFY(!license->hasFeature("feature_c"));
    }
    
    void testFromEnv() {
        // Test with valid key - set env var via qputenv
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        QString base64Key = QString::fromUtf8(publicKey.toBase64());
        
        qputenv("PHOENIX_LICENSE_PUBLIC_KEY", base64Key.toUtf8());
        
        auto verifier = LicenseVerifier::fromEnv();
        QVERIFY(verifier.has_value());
        QCOMPARE(verifier->publicKey(), publicKey);
        
        // Test with unset env var
        qunsetenv("PHOENIX_LICENSE_PUBLIC_KEY");
        
        auto verifier2 = LicenseVerifier::fromEnv();
        QVERIFY(!verifier2.has_value());
    }
};

QTEST_MAIN(LicenseVerificationTest)
#include "license_verification_tests.moc"

