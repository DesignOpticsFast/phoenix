#include <QtTest/QtTest>

#ifdef PHX_WITH_TRANSPORT_DEPS
#include "transport/LocalSocketChannel.hpp"
#include "palantir/error.pb.h"

using namespace phoenix::transport;
#endif

class ErrorMappingTest : public QObject {
    Q_OBJECT

private slots:
    void testMapErrorResponseInternalError();
    void testMapErrorResponseMessageTooLarge();
    void testMapErrorResponseInvalidMessageFormat();
    void testMapErrorResponseProtobufParseError();
    void testMapErrorResponseUnknownMessageType();
    void testMapErrorResponseInvalidParameterValue();
    void testMapErrorResponseInvalidArgument();
    void testMapErrorResponseMissingRequiredField();
    void testMapErrorResponseServiceUnavailable();
    void testMapErrorResponseTimeout();
    void testMapErrorResponseConnectionClosed();
    void testMapErrorResponseConnectionTimeout();
    void testMapErrorResponseMessageTooLargeSpecific();
    void testMapErrorResponseUnknownCode();
    void testMapErrorResponseEmptyMessage();
    void testMapErrorResponseWithDetails();
};

#ifdef PHX_WITH_TRANSPORT_DEPS

void ErrorMappingTest::testMapErrorResponseInternalError()
{
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::INTERNAL_ERROR);
    error.set_message("Server encountered an error");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    QVERIFY(result.contains("Server error"));
    QVERIFY(result.contains("Server encountered an error"));
}

void ErrorMappingTest::testMapErrorResponseMessageTooLarge()
{
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::MESSAGE_TOO_LARGE);
    error.set_message("Message size 11MB exceeds limit 10MB");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    QVERIFY(result.contains("Message too large"));
    QVERIFY(result.contains("Message size 11MB exceeds limit 10MB"));
}

void ErrorMappingTest::testMapErrorResponseInvalidMessageFormat()
{
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::INVALID_MESSAGE_FORMAT);
    error.set_message("Malformed envelope");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    QVERIFY(result.contains("Invalid message format"));
    QVERIFY(result.contains("Malformed envelope"));
}

void ErrorMappingTest::testMapErrorResponseProtobufParseError()
{
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::PROTOBUF_PARSE_ERROR);
    error.set_message("Failed to parse CapabilitiesRequest");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    QVERIFY(result.contains("Protocol error"));
    QVERIFY(result.contains("Failed to parse CapabilitiesRequest"));
}

void ErrorMappingTest::testMapErrorResponseUnknownMessageType()
{
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::UNKNOWN_MESSAGE_TYPE);
    error.set_message("Unknown type: 999");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    QVERIFY(result.contains("Unknown message type"));
    QVERIFY(result.contains("Unknown type: 999"));
}

void ErrorMappingTest::testMapErrorResponseInvalidParameterValue()
{
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::INVALID_PARAMETER_VALUE);
    error.set_message("samples must be between 2 and 10000000");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    QVERIFY(result.contains("Invalid parameter"));
    QVERIFY(result.contains("samples must be between 2 and 10000000"));
}

void ErrorMappingTest::testMapErrorResponseInvalidArgument()
{
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::INVALID_ARGUMENT);
    error.set_message("Invalid argument provided");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    QVERIFY(result.contains("Invalid argument"));
    QVERIFY(result.contains("Invalid argument provided"));
}

void ErrorMappingTest::testMapErrorResponseMissingRequiredField()
{
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::MISSING_REQUIRED_FIELD);
    error.set_message("Required field 'samples' is missing");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    QVERIFY(result.contains("Missing required field"));
    QVERIFY(result.contains("Required field 'samples' is missing"));
}

void ErrorMappingTest::testMapErrorResponseServiceUnavailable()
{
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::SERVICE_UNAVAILABLE);
    error.set_message("Service temporarily unavailable");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    QVERIFY(result.contains("Service unavailable"));
    QVERIFY(result.contains("Service temporarily unavailable"));
}

void ErrorMappingTest::testMapErrorResponseTimeout()
{
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::TIMEOUT);
    error.set_message("Request timed out after 30s");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    QVERIFY(result.contains("Request timeout"));
    QVERIFY(result.contains("Request timed out after 30s"));
}

void ErrorMappingTest::testMapErrorResponseConnectionClosed()
{
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::CONNECTION_CLOSED);
    error.set_message("Connection closed by server");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    QVERIFY(result.contains("Connection closed"));
    QVERIFY(result.contains("Connection closed by server"));
}

void ErrorMappingTest::testMapErrorResponseConnectionTimeout()
{
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::CONNECTION_TIMEOUT);
    error.set_message("Connection timeout");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    QVERIFY(result.contains("Connection timeout"));
    QVERIFY(result.contains("Connection timeout"));
}

void ErrorMappingTest::testMapErrorResponseMessageTooLargeSpecific()
{
    // Specific test for MESSAGE_TOO_LARGE as requested
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::MESSAGE_TOO_LARGE);
    error.set_message("Message size 10485761 bytes exceeds limit 10485760 bytes (10MB)");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    // Verify it's a clear, size-related message
    QVERIFY(result.startsWith("Message too large:"));
    QVERIFY(result.contains("10485761"));
    QVERIFY(result.contains("10485760"));
    QVERIFY(result.contains("10MB"));
}

void ErrorMappingTest::testMapErrorResponseUnknownCode()
{
    // Test with ERROR_CODE_UNSPECIFIED (unknown/unexpected code)
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::ERROR_CODE_UNSPECIFIED);
    error.set_message("Some error occurred");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    // Should fall back to base message (default case)
    QCOMPARE(result, QString("Some error occurred"));
    
    // Test with empty message - should show "Unknown error (code 0)"
    palantir::ErrorResponse error2;
    error2.set_error_code(palantir::ErrorCode::ERROR_CODE_UNSPECIFIED);
    error2.set_message("");
    
    QString result2 = LocalSocketChannel::mapErrorResponse(error2);
    QVERIFY(result2.contains("Unknown error"));
    QVERIFY(result2.contains("code 0"));
}

void ErrorMappingTest::testMapErrorResponseEmptyMessage()
{
    // Test with empty message field
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::INTERNAL_ERROR);
    error.set_message(""); // Empty message
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    // Should handle gracefully - prefix should still be present
    QVERIFY(result.contains("Server error"));
    // Message part may be empty, but function should not crash
    QVERIFY(!result.isEmpty());
}

void ErrorMappingTest::testMapErrorResponseWithDetails()
{
    // Test that details field is appended to the message
    palantir::ErrorResponse error;
    error.set_error_code(palantir::ErrorCode::INVALID_PARAMETER_VALUE);
    error.set_message("Invalid parameter");
    error.set_details("Debug details: samples=0, expected range [2, 10000000]");
    
    QString result = LocalSocketChannel::mapErrorResponse(error);
    
    // Details should be appended in parentheses
    QVERIFY(result.contains("Invalid parameter"));
    QVERIFY(result.contains("Debug details: samples=0, expected range [2, 10000000]"));
    QVERIFY(result.contains("(") && result.contains(")"));
}

#else
// Stub implementations when transport deps are disabled
void ErrorMappingTest::testMapErrorResponseInternalError() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseMessageTooLarge() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseInvalidMessageFormat() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseProtobufParseError() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseUnknownMessageType() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseInvalidParameterValue() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseInvalidArgument() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseMissingRequiredField() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseServiceUnavailable() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseTimeout() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseConnectionClosed() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseConnectionTimeout() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseMessageTooLargeSpecific() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseUnknownCode() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseEmptyMessage() { QSKIP("Transport deps not enabled"); }
void ErrorMappingTest::testMapErrorResponseWithDetails() { QSKIP("Transport deps not enabled"); }
#endif

#include "ErrorMapping_test.moc"
QTEST_MAIN(ErrorMappingTest)

