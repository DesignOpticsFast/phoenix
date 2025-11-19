#pragma once

#include "License.hpp"
#include <QByteArray>
#include <QString>
#include <QJsonObject>
#include <optional>
#include <array>

// Ed25519 public key size (32 bytes)
constexpr size_t ED25519_PUBLIC_KEY_SIZE = 32;

class LicenseVerifier {
public:
    // Construct with public key (raw bytes)
    explicit LicenseVerifier(const QByteArray& publicKey);
    explicit LicenseVerifier(const std::array<unsigned char, ED25519_PUBLIC_KEY_SIZE>& publicKey);
    
    // Load and verify license from file path
    std::optional<License> loadAndVerify(const QString& path);
    
    // Verify a JSON document with signature (lower-level API)
    // Returns License if verification succeeds, nullopt otherwise
    std::optional<License> verifyDocument(const QJsonObject& doc, const QByteArray& signature);
    
    // Get public key
    QByteArray publicKey() const { return publicKey_; }
    
    // Create LicenseVerifier from environment variable PHOENIX_LICENSE_PUBLIC_KEY
    // Returns nullopt if env var not set or invalid
    static std::optional<LicenseVerifier> fromEnv();

private:
    QByteArray publicKey_;  // Ed25519 public key (32 bytes)
    
    // Helper: Extract signature from JSON and return document without signature
    std::pair<QJsonObject, QByteArray> extractSignature(const QJsonObject& doc);
    
    // Helper: Convert QJsonObject to CanonicalValue, then canonicalize
    QByteArray canonicalizeJsonObject(const QJsonObject& doc);
    
    // Helper: Verify signature against canonical JSON bytes
    bool verifyCanonicalized(const QByteArray& canonicalJson, const QByteArray& signature);
    
    // Helper: Parse License from verified JSON document
    std::optional<License> parseLicense(const QJsonObject& doc);
    
    // Helper: Validate JSON schema (required fields, types)
    bool validateSchema(const QJsonObject& doc);
};

