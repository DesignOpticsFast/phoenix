#include <QtTest/QtTest>
#include "app/LicenseManager.h"
#include "license/LicenseVerifier.hpp"
#include "license/License.hpp"
#include "common/canonical_json.hpp"
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <sodium.h>

using namespace phoenix::json;

// Reuse test helpers from license_verification_tests
namespace TestLicenseHelpers {
    std::pair<QByteArray, QByteArray> generateTestKeypair()
    {
        std::array<unsigned char, crypto_sign_PUBLICKEYBYTES> pk{};
        std::array<unsigned char, crypto_sign_SECRETKEYBYTES> sk{};
        
        if (crypto_sign_keypair(pk.data(), sk.data()) != 0) {
            return {QByteArray(), QByteArray()};
        }
        
        QByteArray publicKey(reinterpret_cast<const char*>(pk.data()), crypto_sign_PUBLICKEYBYTES);
        QByteArray privateKey(reinterpret_cast<const char*>(sk.data()), crypto_sign_SECRETKEYBYTES);
        
        return {publicKey, privateKey};
    }
    
    // Convert QJsonValue to CanonicalValue
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
    
    QByteArray signCanonicalJson(const QByteArray& canonicalJson, const QByteArray& privateKey)
    {
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
    
    QJsonObject createSignedLicense(const QByteArray& privateKey,
                                    const QString& subject,
                                    const QStringList& features,
                                    const QDateTime& issuedAt,
                                    const std::optional<QDateTime>& expiresAt = std::nullopt)
    {
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
        
        // Convert to canonical JSON (without signature)
        CanonicalValue canonicalValue = jsonValueToCanonical(QJsonValue(license));
        std::string canonicalJsonStr = to_canonical_json(canonicalValue);
        QByteArray canonicalJson = QByteArray::fromStdString(canonicalJsonStr);
        
        // Sign canonical JSON
        QByteArray signature = signCanonicalJson(canonicalJson, privateKey);
        license["signature"] = QString::fromLatin1(signature.toBase64());
        
        return license;
    }
    
    QString writeTempLicenseFile(const QJsonObject& licenseJson, QTemporaryDir& tempDir)
    {
        QString path = tempDir.filePath("license.json");
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            return {};
        }
        QJsonDocument doc(licenseJson);
        file.write(doc.toJson(QJsonDocument::Compact));
        file.close();
        return path;
    }
}

class LicenseManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        if (sodium_init() < 0) {
            QSKIP("libsodium initialization failed");
        }
    }
    
    void testLicenseManagerSingleton()
    {
        LicenseManager* mgr1 = LicenseManager::instance();
        LicenseManager* mgr2 = LicenseManager::instance();
        
        QVERIFY(mgr1 != nullptr);
        QCOMPARE(mgr1, mgr2);
    }
    
    void testNotConfiguredState()
    {
        // Ensure no env var is set
        qunsetenv("PHOENIX_LICENSE_PUBLIC_KEY");
        
        LicenseManager* mgr = LicenseManager::instance();
        mgr->initialize();
        
        QCOMPARE(mgr->currentState(), LicenseManager::LicenseState::NotConfigured);
        QVERIFY(!mgr->currentLicense().has_value());
        QVERIFY(!mgr->hasFeature("feature_xy_plot"));
    }
    
    void testNoLicenseFile()
    {
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        QVERIFY(!publicKey.isEmpty());
        
        QString base64Key = QString::fromUtf8(publicKey.toBase64());
        qputenv("PHOENIX_LICENSE_PUBLIC_KEY", base64Key.toUtf8());
        qunsetenv("PHOENIX_LICENSE_PATH"); // Use default path
        
        LicenseManager* mgr = LicenseManager::instance();
        mgr->initialize();
        
        QCOMPARE(mgr->currentState(), LicenseManager::LicenseState::NoLicense);
        QVERIFY(!mgr->currentLicense().has_value());
        
        qunsetenv("PHOENIX_LICENSE_PUBLIC_KEY");
    }
    
    void testValidLicense()
    {
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        QVERIFY(!publicKey.isEmpty());
        
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        
        QDateTime issuedAt = QDateTime::currentDateTimeUtc().addDays(-30);
        QDateTime expiresAt = QDateTime::currentDateTimeUtc().addDays(30);
        
        QJsonObject licenseJson = TestLicenseHelpers::createSignedLicense(
            privateKey, "test@example.com", {"feature_xy_plot", "feature_advanced"}, issuedAt, expiresAt);
        
        QString licensePath = TestLicenseHelpers::writeTempLicenseFile(licenseJson, tempDir);
        QVERIFY(!licensePath.isEmpty());
        
        QString base64Key = QString::fromUtf8(publicKey.toBase64());
        qputenv("PHOENIX_LICENSE_PUBLIC_KEY", base64Key.toUtf8());
        qputenv("PHOENIX_LICENSE_PATH", licensePath.toUtf8());
        
        LicenseManager* mgr = LicenseManager::instance();
        mgr->initialize();
        
        QCOMPARE(mgr->currentState(), LicenseManager::LicenseState::Valid);
        QVERIFY(mgr->currentLicense().has_value());
        QCOMPARE(mgr->currentLicense()->subject(), QString("test@example.com"));
        QVERIFY(mgr->hasFeature("feature_xy_plot"));
        QVERIFY(mgr->hasFeature("feature_advanced"));
        QVERIFY(!mgr->hasFeature("feature_nonexistent"));
        
        qunsetenv("PHOENIX_LICENSE_PUBLIC_KEY");
        qunsetenv("PHOENIX_LICENSE_PATH");
    }
    
    void testExpiredLicense()
    {
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        QVERIFY(!publicKey.isEmpty());
        
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        
        QDateTime issuedAt = QDateTime::currentDateTimeUtc().addDays(-60);
        QDateTime expiresAt = QDateTime::currentDateTimeUtc().addDays(-1); // Expired yesterday
        
        QJsonObject licenseJson = TestLicenseHelpers::createSignedLicense(
            privateKey, "expired@example.com", {"feature_xy_plot"}, issuedAt, expiresAt);
        
        QString licensePath = TestLicenseHelpers::writeTempLicenseFile(licenseJson, tempDir);
        QVERIFY(!licensePath.isEmpty());
        
        QString base64Key = QString::fromUtf8(publicKey.toBase64());
        qputenv("PHOENIX_LICENSE_PUBLIC_KEY", base64Key.toUtf8());
        qputenv("PHOENIX_LICENSE_PATH", licensePath.toUtf8());
        
        LicenseManager* mgr = LicenseManager::instance();
        mgr->initialize();
        
        QCOMPARE(mgr->currentState(), LicenseManager::LicenseState::Expired);
        QVERIFY(mgr->currentLicense().has_value()); // License object exists but is expired
        QVERIFY(!mgr->hasFeature("feature_xy_plot")); // Features not available when expired
        
        qunsetenv("PHOENIX_LICENSE_PUBLIC_KEY");
        qunsetenv("PHOENIX_LICENSE_PATH");
    }
    
    void testInvalidLicense()
    {
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        QVERIFY(!publicKey.isEmpty());
        
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        
        // Create a license with wrong signature (use different key)
        auto [wrongPublicKey, wrongPrivateKey] = TestLicenseHelpers::generateTestKeypair();
        
        QDateTime issuedAt = QDateTime::currentDateTimeUtc().addDays(-30);
        QDateTime expiresAt = QDateTime::currentDateTimeUtc().addDays(30);
        
        QJsonObject licenseJson = TestLicenseHelpers::createSignedLicense(
            wrongPrivateKey, "invalid@example.com", {"feature_xy_plot"}, issuedAt, expiresAt);
        
        QString licensePath = TestLicenseHelpers::writeTempLicenseFile(licenseJson, tempDir);
        QVERIFY(!licensePath.isEmpty());
        
        // Use correct public key but license signed with wrong private key
        QString base64Key = QString::fromUtf8(publicKey.toBase64());
        qputenv("PHOENIX_LICENSE_PUBLIC_KEY", base64Key.toUtf8());
        qputenv("PHOENIX_LICENSE_PATH", licensePath.toUtf8());
        
        LicenseManager* mgr = LicenseManager::instance();
        mgr->initialize();
        
        QCOMPARE(mgr->currentState(), LicenseManager::LicenseState::Invalid);
        QVERIFY(!mgr->currentLicense().has_value());
        QVERIFY(!mgr->hasFeature("feature_xy_plot"));
        
        qunsetenv("PHOENIX_LICENSE_PUBLIC_KEY");
        qunsetenv("PHOENIX_LICENSE_PATH");
    }
    
    void testFeatureCheck()
    {
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        QVERIFY(!publicKey.isEmpty());
        
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        
        QDateTime issuedAt = QDateTime::currentDateTimeUtc().addDays(-30);
        QDateTime expiresAt = QDateTime::currentDateTimeUtc().addDays(30);
        
        QJsonObject licenseJson = TestLicenseHelpers::createSignedLicense(
            privateKey, "featuretest@example.com", {"feature_xy_plot"}, issuedAt, expiresAt);
        
        QString licensePath = TestLicenseHelpers::writeTempLicenseFile(licenseJson, tempDir);
        QVERIFY(!licensePath.isEmpty());
        
        QString base64Key = QString::fromUtf8(publicKey.toBase64());
        qputenv("PHOENIX_LICENSE_PUBLIC_KEY", base64Key.toUtf8());
        qputenv("PHOENIX_LICENSE_PATH", licensePath.toUtf8());
        
        LicenseManager* mgr = LicenseManager::instance();
        mgr->initialize();
        
        QCOMPARE(mgr->currentState(), LicenseManager::LicenseState::Valid);
        QVERIFY(mgr->hasFeature("feature_xy_plot"));
        QVERIFY(!mgr->hasFeature("feature_advanced"));
        QVERIFY(!mgr->hasFeature("feature_nonexistent"));
        
        qunsetenv("PHOENIX_LICENSE_PUBLIC_KEY");
        qunsetenv("PHOENIX_LICENSE_PATH");
    }
    
    void testRefresh()
    {
        auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
        QVERIFY(!publicKey.isEmpty());
        
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        
        QDateTime issuedAt = QDateTime::currentDateTimeUtc().addDays(-30);
        QDateTime expiresAt = QDateTime::currentDateTimeUtc().addDays(30);
        
        QJsonObject licenseJson1 = TestLicenseHelpers::createSignedLicense(
            privateKey, "refresh1@example.com", {"feature_xy_plot"}, issuedAt, expiresAt);
        
        QString licensePath = TestLicenseHelpers::writeTempLicenseFile(licenseJson1, tempDir);
        QVERIFY(!licensePath.isEmpty());
        
        QString base64Key = QString::fromUtf8(publicKey.toBase64());
        qputenv("PHOENIX_LICENSE_PUBLIC_KEY", base64Key.toUtf8());
        qputenv("PHOENIX_LICENSE_PATH", licensePath.toUtf8());
        
        LicenseManager* mgr = LicenseManager::instance();
        mgr->initialize();
        
        QCOMPARE(mgr->currentState(), LicenseManager::LicenseState::Valid);
        QCOMPARE(mgr->currentLicense()->subject(), QString("refresh1@example.com"));
        
        // Update license file with new subject
        QJsonObject licenseJson2 = TestLicenseHelpers::createSignedLicense(
            privateKey, "refresh2@example.com", {"feature_xy_plot"}, issuedAt, expiresAt);
        
        QFile file(licensePath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QJsonDocument doc(licenseJson2);
        file.write(doc.toJson(QJsonDocument::Compact));
        file.close();
        
        // Refresh and verify new license is loaded
        mgr->refresh();
        
        QCOMPARE(mgr->currentState(), LicenseManager::LicenseState::Valid);
        QCOMPARE(mgr->currentLicense()->subject(), QString("refresh2@example.com"));
        
        qunsetenv("PHOENIX_LICENSE_PUBLIC_KEY");
        qunsetenv("PHOENIX_LICENSE_PATH");
    }
};

QTEST_MAIN(LicenseManagerTest)
#include "license_manager_tests.moc"

