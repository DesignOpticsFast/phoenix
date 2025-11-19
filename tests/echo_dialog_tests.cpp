#include <QtTest/QtTest>
#include "transport/TransportFactory.hpp"
#include "transport/GrpcUdsChannel.hpp"
#include "transport/LocalSocketChannel.hpp"
#include "app/LicenseManager.h"
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <sodium.h>
#include "common/canonical_json.hpp"
#include "license/LicenseVerifier.hpp"
#include "license/License.hpp"

using namespace phoenix::json;

// Reuse test helpers from license_manager_tests
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
        QString licensePath = tempDir.filePath("license.json");
        QFile licenseFile(licensePath);
        if (!licenseFile.open(QIODevice::WriteOnly)) {
            return QString();
        }
        QJsonDocument doc(licenseJson);
        licenseFile.write(doc.toJson());
        licenseFile.close();
        return licensePath;
    }
}

class EchoDialogTests : public QObject {
    Q_OBJECT

private slots:
    void testBackendDetectionGrpc();
    void testBackendDetectionLocalSocket();
    void testBackendDetectionAuto();
    void testLicenseGating();
    void testLicenseGatingNotConfigured();

private:
    void setupTestLicense(const QString& subject, const QByteArray& publicKey, const QByteArray& privateKey, 
                          const QStringList& features, bool expired = false);
    void cleanupTestLicense();
    QTemporaryDir* m_tempDir = nullptr;
};

void EchoDialogTests::testBackendDetectionGrpc()
{
    // Set environment to grpc
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PHOENIX_TRANSPORT", "grpc");
    
    // Note: We can't easily override the environment in makeTransportClientFromEnv()
    // without modifying the factory, so we test the factory directly
    auto client = makeTransportClient(TransportBackend::Grpc);
    QVERIFY(client != nullptr);
    
    QString backendName = client->backendName();
    QVERIFY(backendName.contains("gRPC", Qt::CaseInsensitive));
}

void EchoDialogTests::testBackendDetectionLocalSocket()
{
    auto client = makeTransportClient(TransportBackend::LocalSocket);
    QVERIFY(client != nullptr);
    
    QString backendName = client->backendName();
    QVERIFY(backendName.contains("LocalSocket", Qt::CaseInsensitive));
}

void EchoDialogTests::testBackendDetectionAuto()
{
    // Auto should default to gRPC
    auto client = makeTransportClient(TransportBackend::Auto);
    QVERIFY(client != nullptr);
    
    QString backendName = client->backendName();
    QVERIFY(backendName.contains("gRPC", Qt::CaseInsensitive));
}

void EchoDialogTests::testLicenseGating()
{
    // Setup test license with feature_echo_test
    auto [publicKey, privateKey] = TestLicenseHelpers::generateTestKeypair();
    QVERIFY(!publicKey.isEmpty() && !privateKey.isEmpty());
    
    setupTestLicense("test@example.com", publicKey, privateKey, {"feature_echo_test"});
    
    LicenseManager* mgr = LicenseManager::instance();
    LicenseManager::LicenseState state = mgr->currentState();
    
    // Should be valid
    QCOMPARE(state, LicenseManager::LicenseState::Valid);
    
    // Should have feature
    QVERIFY(mgr->hasFeature("feature_echo_test"));
    
    cleanupTestLicense();
    
    // Now setup license without feature_echo_test (use same keypair)
    setupTestLicense("test2@example.com", publicKey, privateKey, {"feature_xy_plot"});
    
    state = mgr->currentState();
    QCOMPARE(state, LicenseManager::LicenseState::Valid);
    
    // Should NOT have feature_echo_test
    QVERIFY(!mgr->hasFeature("feature_echo_test"));
    
    cleanupTestLicense();
}

void EchoDialogTests::testLicenseGatingNotConfigured()
{
    // Clean up any existing license
    cleanupTestLicense();
    
    LicenseManager* mgr = LicenseManager::instance();
    LicenseManager::LicenseState state = mgr->currentState();
    
    // Should be NotConfigured
    QCOMPARE(state, LicenseManager::LicenseState::NotConfigured);
    
    // In NotConfigured state, feature check should return false,
    // but the dialog should enable the button (graceful degradation)
    // This is tested at the UI level, but we verify the state here
    QVERIFY(!mgr->hasFeature("feature_echo_test"));
}

void EchoDialogTests::setupTestLicense(const QString& subject, const QByteArray& publicKey, const QByteArray& privateKey,
                                       const QStringList& features, bool expired)
{
    cleanupTestLicense();
    
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    QDateTime issuedAt = QDateTime::currentDateTimeUtc().addDays(-1);
    QDateTime expiresAt = expired 
        ? QDateTime::currentDateTimeUtc().addDays(-1)
        : QDateTime::currentDateTimeUtc().addDays(30);
    
    // Use helper to create signed license (privateKey signs the license)
    QJsonObject licenseJson = TestLicenseHelpers::createSignedLicense(
        privateKey, subject, features, issuedAt, expiresAt);
    
    QString licensePath = TestLicenseHelpers::writeTempLicenseFile(licenseJson, *m_tempDir);
    QVERIFY(!licensePath.isEmpty());
    
    // Set public key (matching the privateKey used to sign) and license path in environment
    QString base64Key = QString::fromUtf8(publicKey.toBase64());
    qputenv("PHOENIX_LICENSE_PUBLIC_KEY", base64Key.toUtf8());
    qputenv("PHOENIX_LICENSE_PATH", licensePath.toUtf8());
    
    // Initialize license manager
    LicenseManager* mgr = LicenseManager::instance();
    mgr->initialize();
}

void EchoDialogTests::cleanupTestLicense()
{
    if (m_tempDir) {
        delete m_tempDir;
        m_tempDir = nullptr;
    }
    
    qunsetenv("PHOENIX_LICENSE_PATH");
    qunsetenv("PHOENIX_LICENSE_PUBLIC_KEY");
    
    LicenseManager* mgr = LicenseManager::instance();
    mgr->initialize(); // Reinitialize to clear state
}

QTEST_MAIN(EchoDialogTests)
#include "echo_dialog_tests.moc"

