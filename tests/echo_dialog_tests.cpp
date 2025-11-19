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
    void setupTestLicense(const QString& subject, const QByteArray& privateKey, 
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
    
    setupTestLicense("test@example.com", privateKey, {"feature_echo_test"});
    
    LicenseManager* mgr = LicenseManager::instance();
    LicenseManager::LicenseState state = mgr->currentState();
    
    // Should be valid
    QCOMPARE(state, LicenseManager::LicenseState::Valid);
    
    // Should have feature
    QVERIFY(mgr->hasFeature("feature_echo_test"));
    
    cleanupTestLicense();
    
    // Now setup license without feature_echo_test
    setupTestLicense("test2@example.com", privateKey, {"feature_xy_plot"});
    
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

void EchoDialogTests::setupTestLicense(const QString& subject, const QByteArray& privateKey,
                                       const QStringList& features, bool expired)
{
    cleanupTestLicense();
    
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    QDateTime issuedAt = QDateTime::currentDateTimeUtc().addDays(-1);
    QDateTime expiresAt = expired 
        ? QDateTime::currentDateTimeUtc().addDays(-1)
        : QDateTime::currentDateTimeUtc().addDays(30);
    
    License license(subject, features, issuedAt, expiresAt);
    
    QJsonObject licenseObj;
    licenseObj["subject"] = subject;
    licenseObj["issued_at"] = issuedAt.toUTC().toString(Qt::ISODate);
    licenseObj["expires_at"] = expiresAt.toUTC().toString(Qt::ISODate);
    
    QJsonArray featuresArray;
    for (const QString& feature : features) {
        featuresArray.append(feature);
    }
    licenseObj["features"] = featuresArray;
    
    QJsonDocument doc(licenseObj);
    QByteArray canonicalJson = canonicalize(doc.toJson(QJsonDocument::Compact));
    
    // Sign the license
    std::array<unsigned char, crypto_sign_BYTES> signature{};
    if (crypto_sign_detached(signature.data(), nullptr,
                             reinterpret_cast<const unsigned char*>(canonicalJson.constData()),
                             canonicalJson.size(),
                             privateKey.constData()) != 0) {
        QFAIL("Failed to sign license");
    }
    
    QByteArray signatureBytes(reinterpret_cast<const char*>(signature.data()), crypto_sign_BYTES);
    QString signatureBase64 = signatureBytes.toBase64();
    
    QJsonObject signedLicense;
    signedLicense["license"] = licenseObj;
    signedLicense["signature"] = signatureBase64;
    
    QJsonDocument signedDoc(signedLicense);
    
    // Write license file
    QString licensePath = m_tempDir->filePath("license.json");
    QFile licenseFile(licensePath);
    QVERIFY(licenseFile.open(QIODevice::WriteOnly));
    licenseFile.write(signedDoc.toJson());
    licenseFile.close();
    
    // Set license path in environment
    qputenv("PHOENIX_LICENSE_PATH", licensePath.toLocal8Bit());
    
    // Refresh license manager
    LicenseManager* mgr = LicenseManager::instance();
    mgr->refresh();
}

void EchoDialogTests::cleanupTestLicense()
{
    if (m_tempDir) {
        delete m_tempDir;
        m_tempDir = nullptr;
    }
    
    qunsetenv("PHOENIX_LICENSE_PATH");
    
    LicenseManager* mgr = LicenseManager::instance();
    mgr->refresh();
}

QTEST_MAIN(EchoDialogTests)
#include "echo_dialog_tests.moc"

