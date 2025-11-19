# WP2.C Plan – Licensing Core in Phoenix

## Code Inspection Summary

### Existing Infrastructure

**phoenix_canonical_json:**
- Located in `src/common/canonical_json.{hpp,cpp}`
- Provides `to_canonical_json()` function in `phoenix::json` namespace
- Takes `CanonicalValue` (variant type built from `std::map<std::string, CanonicalValue>`), returns `std::string` (canonicalized JSON)
- Used for stable, deterministic JSON serialization with sorted keys
- Also provides `to_canonical_timestamp()` for ISO 8601 timestamps
- Already linked into phoenix_app
- Has test: `tests/canonical_json_tests.cpp`
- **Note:** Need to convert `QJsonObject` → `CanonicalValue` → canonicalize → `QByteArray` for signing

**phoenix_libsodium:**
- CMake target exists, wraps libsodium library (third_party/libsodium)
- No high-level wrapper API found - uses raw libsodium functions directly
- From sanity test (`tests/libsodium_ed25519_sanity.cpp`), uses:
  - `crypto_sign_keypair()` - generates Ed25519 keypair (public key: 32 bytes, private key: 64 bytes)
  - `crypto_sign_detached()` - signs a message with private key, returns signature (64 bytes)
  - `crypto_sign_verify_detached()` - verifies a signature with public key (returns 0 on success, -1 on failure)
- Requires `sodium_init()` before first use
- Already linked into phoenix_app
- Has sanity test: `tests/libsodium_ed25519_sanity.cpp`

**No Existing License Code Found:**
- No license-related classes or structs found
- No license file loading code
- No license verification logic
- Clean slate for implementation

### Test Infrastructure

- Uses QtTest framework (consistent with transport tests)
- Test files in `tests/` directory
- Existing pattern: `*_sanity.cpp` for basic functionality tests

---

## Proposed License Model

### JSON Schema

```json
{
  "subject": "user@example.com",
  "issued_at": "2024-01-15T10:30:00Z",
  "expires_at": "2025-01-15T10:30:00Z",  // null for no expiry
  "features": ["feature_a", "feature_b"],
  "signature": "base64_encoded_signature_here"
}
```

**Fields:**
- `subject` (string, required): License holder identifier
- `issued_at` (ISO 8601 string, required): When license was issued
- `expires_at` (ISO 8601 string or null, optional): Expiration date (null = no expiry)
- `features` (array of strings, required): List of licensed features
- `signature` (base64 string, required): Ed25519 signature of canonicalized JSON (excluding signature field)

**Signature Process:**
1. Create JSON object without `signature` field
2. Convert `QJsonObject` to `CanonicalValue` (std::map structure)
3. Canonicalize using `to_canonical_json()` → get `std::string`
4. Convert to `QByteArray` for signing
5. Sign canonical bytes with Ed25519 private key using `crypto_sign_detached()`
6. Base64-encode signature
7. Add `signature` field to original JSON
8. Save final JSON to file

**Verification Process:**
1. Load JSON file
2. Extract `signature` field (base64-decode)
3. Remove `signature` field from JSON
4. Convert `QJsonObject` to `CanonicalValue`
5. Canonicalize using `to_canonical_json()` → get `std::string` → `QByteArray`
6. Verify signature against canonical bytes using `crypto_sign_verify_detached()` with public key

### C++ License Model

```cpp
// src/license/License.hpp
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
    
    bool expired() const;
    bool hasFeature(const QString& feature) const;
    bool isValid() const;  // Not expired and has at least one feature
    
private:
    QString subject_;
    QStringList features_;
    QDateTime issuedAt_;
    std::optional<QDateTime> expiresAt_;
};
```

---

## File Discovery / Configuration

### Proposed Approach

**Priority Order:**
1. `PHOENIX_LICENSE_PATH` environment variable (if set)
2. Default path: `~/.config/Phoenix/license.json` (Linux/macOS)
3. For tests: Use test fixture directory (e.g., `tests/fixtures/licenses/`)

**Implementation:**
```cpp
// src/license/LicenseLoader.hpp
class LicenseLoader {
public:
    static QString defaultLicensePath();
    static QString findLicensePath();  // Checks env var, then default
};
```

**Test Strategy:**
- Tests create temporary license files in test fixture directory
- Use `QTemporaryDir` or `QDir::tempPath()` for test isolation
- Never read from system/user config directories in tests

---

## Licensing API Design

### Proposed API

```cpp
// src/license/LicenseVerifier.hpp
#pragma once

#include "License.hpp"
#include <QByteArray>
#include <QString>
#include <optional>

class LicenseVerifier {
public:
    // Construct with public key (for verification)
    explicit LicenseVerifier(const QByteArray& publicKey);
    
    // Load and verify license from file path
    std::optional<License> loadAndVerify(const QString& path);
    
    // Verify a JSON document with signature (lower-level API)
    bool verifyDocument(const QJsonObject& doc, const QByteArray& signature);
    
    // Get public key
    QByteArray publicKey() const { return publicKey_; }

private:
    QByteArray publicKey_;  // Ed25519 public key (32 bytes)
    
    // Helper: Extract signature from JSON and return document without signature
    std::pair<QJsonObject, QByteArray> extractSignature(const QJsonObject& doc);
    
    // Helper: Convert QJsonObject to CanonicalValue, then canonicalize
    QByteArray canonicalizeJsonObject(const QJsonObject& doc);
    
    // Helper: Verify signature against canonical JSON bytes
    bool verifyCanonicalized(const QByteArray& canonicalJson, const QByteArray& signature);
};
```

**Usage Pattern:**
```cpp
// Load public key (from config, env var, or hardcoded for tests)
QByteArray publicKey = loadPublicKey();  // Implementation TBD

LicenseVerifier verifier(publicKey);
auto license = verifier.loadAndVerify("/path/to/license.json");

if (license.has_value()) {
    if (license->expired()) {
        // Handle expired license
    }
    if (license->hasFeature("feature_a")) {
        // Enable feature
    }
}
```

**Public Key Management:**
- For WP2.C: Hardcode a test public key or load from env var `PHOENIX_LICENSE_PUBLIC_KEY`
- Future: Load from config file or embedded in binary
- Tests: Generate keypair on-the-fly for each test

---

## Test Design

### Test Structure

**File:** `tests/license_verification_tests.cpp`

**Test Cases:**

1. **testValidLicenseAccepted()**
   - Generate Ed25519 keypair
   - Create valid license JSON (without signature)
   - Canonicalize JSON
   - Sign canonical bytes with private key
   - Add signature to JSON
   - Write to temporary file
   - Load and verify with public key
   - Assert license is valid, not expired, has expected features

2. **testTamperedLicenseRejected()**
   - Create valid signed license file
   - Modify a field (e.g., change `subject` or add a `feature`)
   - Keep original signature
   - Attempt to verify
   - Assert verification fails

3. **testWrongKeyRejected()**
   - Generate two Ed25519 keypairs (keypair1, keypair2)
   - Sign license with keypair1 private key
   - Attempt to verify with keypair2 public key
   - Assert verification fails

4. **testExpiredLicense()**
   - Create license with `expires_at` in the past
   - Sign and verify successfully
   - Assert `license.expired() == true`
   - Assert `license.isValid() == false`

5. **testNoExpiryLicense()**
   - Create license with `expires_at: null`
   - Sign and verify successfully
   - Assert `license.expired() == false`
   - Assert `license.isValid() == true`

6. **testMissingSignatureRejected()**
   - Create license JSON without `signature` field
   - Attempt to verify
   - Assert verification fails (or returns nullopt)

7. **testInvalidJsonRejected()**
   - Attempt to load non-existent file
   - Attempt to load invalid JSON file
   - Assert appropriate error handling

8. **testFeatureCheck()**
   - Create license with features: ["feature_a", "feature_b"]
   - Verify `hasFeature("feature_a") == true`
   - Verify `hasFeature("feature_b") == true`
   - Verify `hasFeature("feature_c") == false`

### Test Utilities

```cpp
// Helper functions for tests
namespace TestLicenseHelpers {
    // Generate test keypair using libsodium
    // Returns (publicKey, privateKey) as QByteArray
    std::pair<QByteArray, QByteArray> generateTestKeypair();
    
    // Sign canonical JSON bytes with private key
    QByteArray signCanonicalJson(const QByteArray& canonicalJson, const QByteArray& privateKey);
    
    // Create and sign license JSON
    QJsonObject createSignedLicense(
        const QByteArray& privateKey,
        const QString& subject,
        const QStringList& features,
        const QDateTime& issuedAt,
        const std::optional<QDateTime>& expiresAt = std::nullopt
    );
    
    // Write JSON to temporary file
    QString writeTempLicenseFile(const QJsonObject& licenseJson);
}
```

---

## Implementation Plan

### Phase 1: License Model

1. **Create `src/license/License.hpp/.cpp`**
   - Implement `License` class with fields and methods
   - Implement `expired()`, `hasFeature()`, `isValid()` logic

### Phase 2: License Verifier

2. **Create `src/license/LicenseVerifier.hpp/.cpp`**
   - Implement `LicenseVerifier` class
   - Implement `loadAndVerify()` - file loading + verification
   - Implement `verifyDocument()` - lower-level verification API
   - Convert `QJsonObject` to `CanonicalValue`, then use `to_canonical_json()` for canonicalization
   - Use libsodium's `crypto_sign_verify_detached()` for Ed25519 verification

3. **Create `src/license/LicenseLoader.hpp/.cpp`**
   - Implement file path discovery logic
   - Support `PHOENIX_LICENSE_PATH` env var
   - Default path logic

### Phase 3: CMake Integration

4. **Update `CMakeLists.txt`**
   - Create `phoenix_license` library target
   - Link `phoenix_canonical_json` and `phoenix_libsodium`
   - Add license sources to build

### Phase 4: Tests

5. **Create `tests/license_verification_tests.cpp`**
   - Implement all test cases listed above
   - Use QtTest framework
   - Create test fixtures/utilities

6. **Update `tests/CMakeLists.txt`**
   - Add `license_verification_tests` executable
   - Link against `phoenix_license`, `phoenix_canonical_json`, `phoenix_libsodium`

### Phase 5: Build & Verify

7. **Build and test**
   - Ensure all tests pass
   - Verify no UI dependencies
   - Confirm clean API surface

---

## Open Questions

1. **Public Key Source:** Where should the public key come from for WP2.C?
   - Option A: Environment variable `PHOENIX_LICENSE_PUBLIC_KEY` (base64 or hex)
   - Option B: Hardcoded test key for now, config file later
   - Option C: Embedded in binary (for production)
   - **Recommendation:** Use env var for WP2.C, can add config file support later

2. **Signature Field Location:** Should signature be:
   - Option A: Inside JSON object (`"signature": "..."`)
   - Option B: Separate file (`license.json` + `license.json.sig`)
   - **Recommendation:** Option A (inside JSON) - simpler, single file

3. **Error Handling:** How should verification failures be communicated?
   - Option A: Return `std::optional<License>` (nullopt on failure)
   - Option B: Return `std::pair<bool, std::optional<License>>` with error details
   - Option C: Throw exceptions
   - **Recommendation:** Option A for WP2.C, can add error details later if needed

4. **License File Format:** Should we support:
   - Option A: Strict schema validation (reject unknown fields)
   - Option B: Permissive (ignore unknown fields, only validate required)
   - **Recommendation:** Option B for WP2.C - more flexible for future extensions

5. **Date Parsing:** ISO 8601 parsing - use QDateTime or custom parser?
   - **Recommendation:** Use `QDateTime::fromString()` with ISO 8601 format, or `QDateTime::fromString()` with Qt::ISODate

6. **Base64 Encoding:** Use Qt's `QByteArray::toBase64()` / `fromBase64()`?
   - **Recommendation:** Yes, Qt's built-in base64 support

---

## Next Steps

**Waiting for user clarification on:**
1. Public key source preference
2. Signature field location preference
3. Error handling approach
4. Any other design decisions

Once clarified, proceed with implementation.

