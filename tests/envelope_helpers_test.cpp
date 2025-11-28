#include <QtTest/QtTest>

#ifdef PHX_WITH_TRANSPORT_DEPS
#include "transport/EnvelopeHelpers.hpp"
#include "palantir/capabilities.pb.h"
#include "palantir/xysine.pb.h"
#include "palantir/envelope.pb.h"

using namespace phoenix::transport;
#endif

class EnvelopeHelpersTest : public QObject {
    Q_OBJECT

private slots:
    void testMakeEnvelopeCapabilitiesRequest();
    void testMakeEnvelopeCapabilitiesResponse();
    void testMakeEnvelopeXYSineRequest();
    void testMakeEnvelopeXYSineResponse();
    void testMakeEnvelopeWithMetadata();
    
    void testParseEnvelopeValid();
    void testParseEnvelopeInvalidVersion();
    void testParseEnvelopeInvalidType();
    void testParseEnvelopeTruncated();
    void testParseEnvelopeEmptyBuffer();
    void testParseEnvelopeUnspecifiedType();
    
    void testRoundTripCapabilitiesRequest();
    void testRoundTripCapabilitiesResponse();
    void testRoundTripXYSineRequest();
    void testRoundTripXYSineResponse();
    
    void testParseEnvelopeVersionZero();
    void testParseEnvelopeCompletelyMalformed();
    void testMetadataRoundTripWithSpecialCharacters();
    void testMetadataRoundTripEmpty();
};

#ifdef PHX_WITH_TRANSPORT_DEPS
void EnvelopeHelpersTest::testMakeEnvelopeCapabilitiesRequest()
{
    palantir::CapabilitiesRequest request;
    QString error;
    
    auto envelope = makeEnvelope(palantir::MessageType::CAPABILITIES_REQUEST, request, {}, &error);
    
    QVERIFY(envelope.has_value());
    QCOMPARE(envelope->version(), 1u);
    QCOMPARE(envelope->type(), palantir::MessageType::CAPABILITIES_REQUEST);
    // Payload can be empty for empty messages (CapabilitiesRequest has no fields)
    QVERIFY(error.isEmpty());
}

void EnvelopeHelpersTest::testMakeEnvelopeCapabilitiesResponse()
{
    palantir::CapabilitiesResponse response;
    auto* caps = response.mutable_capabilities();
    caps->set_server_version("test-1.0");
    caps->add_supported_features("xy_sine");
    
    QString error;
    auto envelope = makeEnvelope(palantir::MessageType::CAPABILITIES_RESPONSE, response, {}, &error);
    
    QVERIFY(envelope.has_value());
    QCOMPARE(envelope->version(), 1u);
    QCOMPARE(envelope->type(), palantir::MessageType::CAPABILITIES_RESPONSE);
    QVERIFY(!envelope->payload().empty());
    QVERIFY(error.isEmpty());
}

void EnvelopeHelpersTest::testMakeEnvelopeXYSineRequest()
{
    palantir::XYSineRequest request;
    request.set_frequency(2.0);
    request.set_amplitude(1.5);
    request.set_phase(0.5);
    request.set_samples(100);
    
    QString error;
    auto envelope = makeEnvelope(palantir::MessageType::XY_SINE_REQUEST, request, {}, &error);
    
    QVERIFY(envelope.has_value());
    QCOMPARE(envelope->version(), 1u);
    QCOMPARE(envelope->type(), palantir::MessageType::XY_SINE_REQUEST);
    QVERIFY(!envelope->payload().empty());
    QVERIFY(error.isEmpty());
}

void EnvelopeHelpersTest::testMakeEnvelopeXYSineResponse()
{
    palantir::XYSineResponse response;
    response.add_x(0.0);
    response.add_x(1.0);
    response.add_y(0.0);
    response.add_y(1.0);
    response.set_status("OK");
    
    QString error;
    auto envelope = makeEnvelope(palantir::MessageType::XY_SINE_RESPONSE, response, {}, &error);
    
    QVERIFY(envelope.has_value());
    QCOMPARE(envelope->version(), 1u);
    QCOMPARE(envelope->type(), palantir::MessageType::XY_SINE_RESPONSE);
    QVERIFY(!envelope->payload().empty());
    QVERIFY(error.isEmpty());
}

void EnvelopeHelpersTest::testMakeEnvelopeWithMetadata()
{
    palantir::CapabilitiesRequest request;
    std::map<std::string, std::string> metadata;
    metadata["trace_id"] = "abc123";
    metadata["client_version"] = "phoenix-0.0.4";
    
    QString error;
    auto envelope = makeEnvelope(palantir::MessageType::CAPABILITIES_REQUEST, request, metadata, &error);
    
    QVERIFY(envelope.has_value());
    QCOMPARE(envelope->metadata().size(), 2u);
    QCOMPARE(envelope->metadata().at("trace_id"), "abc123");
    QCOMPARE(envelope->metadata().at("client_version"), "phoenix-0.0.4");
    QVERIFY(error.isEmpty());
}

void EnvelopeHelpersTest::testParseEnvelopeValid()
{
    // Create a valid envelope
    palantir::CapabilitiesRequest request;
    auto envelope = makeEnvelope(palantir::MessageType::CAPABILITIES_REQUEST, request);
    QVERIFY(envelope.has_value());
    
    // Serialize it
    std::string serialized;
    QVERIFY(envelope->SerializeToString(&serialized));
    
    // Parse it back
    QByteArray buffer(serialized.data(), static_cast<int>(serialized.size()));
    palantir::MessageEnvelope parsed;
    QString error;
    
    QVERIFY(parseEnvelope(buffer, parsed, &error));
    QCOMPARE(parsed.version(), 1u);
    QCOMPARE(parsed.type(), palantir::MessageType::CAPABILITIES_REQUEST);
    QVERIFY(error.isEmpty());
}

void EnvelopeHelpersTest::testParseEnvelopeInvalidVersion()
{
    // Create envelope with wrong version
    palantir::MessageEnvelope envelope;
    envelope.set_version(999); // Invalid version
    envelope.set_type(palantir::MessageType::CAPABILITIES_REQUEST);
    envelope.set_payload("test");
    
    std::string serialized;
    QVERIFY(envelope.SerializeToString(&serialized));
    
    QByteArray buffer(serialized.data(), static_cast<int>(serialized.size()));
    palantir::MessageEnvelope parsed;
    QString error;
    
    QVERIFY(!parseEnvelope(buffer, parsed, &error));
    QVERIFY(!error.isEmpty());
    QVERIFY(error.contains("Invalid protocol version"));
}

void EnvelopeHelpersTest::testParseEnvelopeInvalidType()
{
    // Create envelope with UNSPECIFIED type (invalid)
    palantir::MessageEnvelope envelope;
    envelope.set_version(1);
    envelope.set_type(palantir::MessageType::MESSAGE_TYPE_UNSPECIFIED);
    envelope.set_payload("test");
    
    std::string serialized;
    QVERIFY(envelope.SerializeToString(&serialized));
    
    QByteArray buffer(serialized.data(), static_cast<int>(serialized.size()));
    palantir::MessageEnvelope parsed;
    QString error;
    
    QVERIFY(!parseEnvelope(buffer, parsed, &error));
    QVERIFY(!error.isEmpty());
    QVERIFY(error.contains("UNSPECIFIED"));
}

void EnvelopeHelpersTest::testParseEnvelopeTruncated()
{
    // Create valid envelope
    palantir::CapabilitiesRequest request;
    auto envelope = makeEnvelope(palantir::MessageType::CAPABILITIES_REQUEST, request);
    QVERIFY(envelope.has_value());
    
    std::string serialized;
    QVERIFY(envelope->SerializeToString(&serialized));
    
    // Truncate the buffer (remove last few bytes) - need to remove enough to break parsing
    // Protobuf is lenient, so remove more bytes to ensure parse failure
    int truncatedSize = qMax(1, static_cast<int>(serialized.size() - 10));
    QByteArray buffer(serialized.data(), truncatedSize);
    palantir::MessageEnvelope parsed;
    QString error;
    
    // parseEnvelope may succeed on partial data (protobuf is lenient), but the envelope will be malformed
    // Check that either parsing fails OR the parsed envelope is invalid
    bool parseResult = parseEnvelope(buffer, parsed, &error);
    if (parseResult) {
        // If parsing succeeded, verify the envelope is malformed (version check should catch it)
        QVERIFY(parsed.version() != 1 || parsed.type() == palantir::MessageType::MESSAGE_TYPE_UNSPECIFIED);
    } else {
        // Parsing failed as expected
        QVERIFY(!error.isEmpty());
    }
}

void EnvelopeHelpersTest::testParseEnvelopeEmptyBuffer()
{
    QByteArray buffer;
    palantir::MessageEnvelope parsed;
    QString error;
    
    QVERIFY(!parseEnvelope(buffer, parsed, &error));
    QVERIFY(!error.isEmpty());
    QVERIFY(error.contains("Empty buffer"));
}

void EnvelopeHelpersTest::testParseEnvelopeUnspecifiedType()
{
    // Test that UNSPECIFIED type is rejected
    palantir::MessageEnvelope envelope;
    envelope.set_version(1);
    envelope.set_type(palantir::MessageType::MESSAGE_TYPE_UNSPECIFIED);
    envelope.set_payload("");
    
    std::string serialized;
    QVERIFY(envelope.SerializeToString(&serialized));
    
    QByteArray buffer(serialized.data(), static_cast<int>(serialized.size()));
    palantir::MessageEnvelope parsed;
    QString error;
    
    QVERIFY(!parseEnvelope(buffer, parsed, &error));
    QVERIFY(error.contains("UNSPECIFIED"));
}

void EnvelopeHelpersTest::testRoundTripCapabilitiesRequest()
{
    palantir::CapabilitiesRequest original;
    
    // Encode
    auto envelope = makeEnvelope(palantir::MessageType::CAPABILITIES_REQUEST, original);
    QVERIFY(envelope.has_value());
    
    // Serialize envelope
    std::string serialized;
    QVERIFY(envelope->SerializeToString(&serialized));
    
    // Parse envelope
    QByteArray buffer(serialized.data(), static_cast<int>(serialized.size()));
    palantir::MessageEnvelope parsed;
    QVERIFY(parseEnvelope(buffer, parsed));
    
    // Extract payload
    palantir::CapabilitiesRequest decoded;
    QVERIFY(decoded.ParseFromString(parsed.payload()));
    
    // Verify round-trip (CapabilitiesRequest is empty, so just verify it parses)
    QVERIFY(decoded.IsInitialized());
}

void EnvelopeHelpersTest::testRoundTripCapabilitiesResponse()
{
    palantir::CapabilitiesResponse original;
    auto* caps = original.mutable_capabilities();
    caps->set_server_version("test-1.0");
    caps->add_supported_features("xy_sine");
    caps->add_supported_features("heat_diffusion");
    
    // Encode
    auto envelope = makeEnvelope(palantir::MessageType::CAPABILITIES_RESPONSE, original);
    QVERIFY(envelope.has_value());
    
    // Serialize and parse
    std::string serialized;
    QVERIFY(envelope->SerializeToString(&serialized));
    QByteArray buffer(serialized.data(), static_cast<int>(serialized.size()));
    palantir::MessageEnvelope parsed;
    QVERIFY(parseEnvelope(buffer, parsed));
    
    // Extract and verify
    palantir::CapabilitiesResponse decoded;
    const std::string& payload = parsed.payload();
    QVERIFY(decoded.ParseFromArray(payload.data(), static_cast<int>(payload.size())));
    QCOMPARE(decoded.capabilities().server_version(), "test-1.0");
    QCOMPARE(decoded.capabilities().supported_features_size(), 2);
    QCOMPARE(decoded.capabilities().supported_features(0), "xy_sine");
    QCOMPARE(decoded.capabilities().supported_features(1), "heat_diffusion");
}

void EnvelopeHelpersTest::testRoundTripXYSineRequest()
{
    palantir::XYSineRequest original;
    original.set_frequency(2.5);
    original.set_amplitude(1.8);
    original.set_phase(0.3);
    original.set_samples(500);
    
    // Encode
    auto envelope = makeEnvelope(palantir::MessageType::XY_SINE_REQUEST, original);
    QVERIFY(envelope.has_value());
    
    // Serialize and parse
    std::string serialized;
    QVERIFY(envelope->SerializeToString(&serialized));
    QByteArray buffer(serialized.data(), static_cast<int>(serialized.size()));
    palantir::MessageEnvelope parsed;
    QVERIFY(parseEnvelope(buffer, parsed));
    
    // Extract and verify
    palantir::XYSineRequest decoded;
    QVERIFY(decoded.ParseFromString(parsed.payload()));
    QCOMPARE(decoded.frequency(), 2.5);
    QCOMPARE(decoded.amplitude(), 1.8);
    QCOMPARE(decoded.phase(), 0.3);
    QCOMPARE(decoded.samples(), 500);
}

void EnvelopeHelpersTest::testRoundTripXYSineResponse()
{
    palantir::XYSineResponse original;
    original.add_x(0.0);
    original.add_x(1.57);
    original.add_x(3.14);
    original.add_y(0.0);
    original.add_y(1.0);
    original.add_y(0.0);
    original.set_status("OK");
    
    // Encode
    auto envelope = makeEnvelope(palantir::MessageType::XY_SINE_RESPONSE, original);
    QVERIFY(envelope.has_value());
    
    // Serialize and parse
    std::string serialized;
    QVERIFY(envelope->SerializeToString(&serialized));
    QByteArray buffer(serialized.data(), static_cast<int>(serialized.size()));
    palantir::MessageEnvelope parsed;
    QVERIFY(parseEnvelope(buffer, parsed));
    
    // Extract and verify
    palantir::XYSineResponse decoded;
    const std::string& payload = parsed.payload();
    QVERIFY(decoded.ParseFromArray(payload.data(), static_cast<int>(payload.size())));
    QCOMPARE(decoded.x_size(), 3);
    QCOMPARE(decoded.y_size(), 3);
    QCOMPARE(decoded.x(0), 0.0);
    QCOMPARE(decoded.x(1), 1.57);
    QCOMPARE(decoded.y(0), 0.0);
    QCOMPARE(decoded.y(1), 1.0);
    QCOMPARE(decoded.status(), "OK");
}

void EnvelopeHelpersTest::testParseEnvelopeVersionZero()
{
    // Test that version 0 is rejected
    palantir::MessageEnvelope envelope;
    envelope.set_version(0); // Invalid version
    envelope.set_type(palantir::MessageType::CAPABILITIES_REQUEST);
    envelope.set_payload("test");
    
    std::string serialized;
    QVERIFY(envelope.SerializeToString(&serialized));
    
    QByteArray buffer(serialized.data(), static_cast<int>(serialized.size()));
    palantir::MessageEnvelope parsed;
    QString error;
    
    QVERIFY(!parseEnvelope(buffer, parsed, &error));
    QVERIFY(!error.isEmpty());
    QVERIFY(error.contains("Invalid protocol version"));
}

void EnvelopeHelpersTest::testParseEnvelopeCompletelyMalformed()
{
    // Test with completely random/invalid data
    QByteArray malformed("\x00\x01\x02\x03\xFF\xFE\xFD\xFC", 8);
    
    palantir::MessageEnvelope parsed;
    QString error;
    
    // parseEnvelope should fail on completely malformed data
    QVERIFY(!parseEnvelope(malformed, parsed, &error));
    QVERIFY(!error.isEmpty());
}

void EnvelopeHelpersTest::testMetadataRoundTripWithSpecialCharacters()
{
    // Test metadata with special characters and empty values
    palantir::CapabilitiesRequest request;
    std::map<std::string, std::string> metadata;
    metadata["trace_id"] = "abc-123_xyz";
    metadata["client_version"] = "phoenix-0.0.4";
    metadata["empty_value"] = "";
    metadata["special_chars"] = "test@example.com:8080/path?query=value";
    
    auto envelope = makeEnvelope(palantir::MessageType::CAPABILITIES_REQUEST, request, metadata);
    QVERIFY(envelope.has_value());
    
    // Serialize and parse
    std::string serialized;
    QVERIFY(envelope->SerializeToString(&serialized));
    QByteArray buffer(serialized.data(), static_cast<int>(serialized.size()));
    palantir::MessageEnvelope parsed;
    QVERIFY(parseEnvelope(buffer, parsed));
    
    // Verify metadata round-trip
    QCOMPARE(parsed.metadata().size(), 4u);
    QCOMPARE(parsed.metadata().at("trace_id"), "abc-123_xyz");
    QCOMPARE(parsed.metadata().at("client_version"), "phoenix-0.0.4");
    QCOMPARE(parsed.metadata().at("empty_value"), "");
    QCOMPARE(parsed.metadata().at("special_chars"), "test@example.com:8080/path?query=value");
}

void EnvelopeHelpersTest::testMetadataRoundTripEmpty()
{
    // Test metadata with empty map
    palantir::CapabilitiesRequest request;
    std::map<std::string, std::string> emptyMetadata;
    
    auto envelope = makeEnvelope(palantir::MessageType::CAPABILITIES_REQUEST, request, emptyMetadata);
    QVERIFY(envelope.has_value());
    
    // Serialize and parse
    std::string serialized;
    QVERIFY(envelope->SerializeToString(&serialized));
    QByteArray buffer(serialized.data(), static_cast<int>(serialized.size()));
    palantir::MessageEnvelope parsed;
    QVERIFY(parseEnvelope(buffer, parsed));
    
    // Verify metadata is empty
    QCOMPARE(parsed.metadata().size(), 0u);
}
#else
// Stub implementations when transport deps are disabled
void EnvelopeHelpersTest::testMakeEnvelopeCapabilitiesRequest() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testMakeEnvelopeCapabilitiesResponse() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testMakeEnvelopeXYSineRequest() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testMakeEnvelopeXYSineResponse() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testMakeEnvelopeWithMetadata() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testParseEnvelopeValid() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testParseEnvelopeInvalidVersion() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testParseEnvelopeInvalidType() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testParseEnvelopeTruncated() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testParseEnvelopeEmptyBuffer() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testParseEnvelopeUnspecifiedType() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testRoundTripCapabilitiesRequest() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testRoundTripCapabilitiesResponse() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testRoundTripXYSineRequest() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testRoundTripXYSineResponse() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testParseEnvelopeVersionZero() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testParseEnvelopeCompletelyMalformed() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testMetadataRoundTripWithSpecialCharacters() { QSKIP("Transport deps not enabled"); }
void EnvelopeHelpersTest::testMetadataRoundTripEmpty() { QSKIP("Transport deps not enabled"); }
#endif

#include "envelope_helpers_test.moc"
QTEST_MAIN(EnvelopeHelpersTest)
