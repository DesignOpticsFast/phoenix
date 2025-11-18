#include <QtTest/QtTest>
#include "common/canonical_json.hpp"
#include <map>
#include <vector>
#include <chrono>

using namespace phoenix::json;

class CanonicalJsonTest : public QObject
{
    Q_OBJECT

private slots:
    void testKeyOrdering();
    void testNestedObjects();
    void testTimestampFormat();
    void testGoldenJson();
    void testPrimitiveTypes();
};

void CanonicalJsonTest::testKeyOrdering()
{
    // Create a map with keys out of order
    std::map<std::string, CanonicalValue> obj;
    obj["z"] = CanonicalValue{int64_t{3}};
    obj["a"] = CanonicalValue{int64_t{1}};
    obj["m"] = CanonicalValue{int64_t{2}};
    
    CanonicalValue value = obj;
    std::string json = to_canonical_json(value);
    
    // Verify keys are in sorted order: a, m, z
    QVERIFY(json.find("\"a\"") < json.find("\"m\""));
    QVERIFY(json.find("\"m\"") < json.find("\"z\""));
    
    // Verify exact format
    QCOMPARE(json, std::string("{\"a\":1,\"m\":2,\"z\":3}"));
}

void CanonicalJsonTest::testNestedObjects()
{
    // Create nested objects with unsorted keys
    std::map<std::string, CanonicalValue> inner;
    inner["y"] = CanonicalValue{int64_t{2}};
    inner["x"] = CanonicalValue{int64_t{1}};
    
    std::map<std::string, CanonicalValue> outer;
    outer["b"] = CanonicalValue{inner};
    outer["a"] = CanonicalValue{int64_t{0}};
    
    CanonicalValue value = outer;
    std::string json = to_canonical_json(value);
    
    // Verify outer keys are sorted: a, b
    QVERIFY(json.find("\"a\"") < json.find("\"b\""));
    
    // Verify inner keys are sorted: x, y
    size_t innerStart = json.find("\"b\"");
    size_t xPos = json.find("\"x\"", innerStart);
    size_t yPos = json.find("\"y\"", innerStart);
    QVERIFY(xPos < yPos);
    
    // Verify exact format
    QCOMPARE(json, std::string("{\"a\":0,\"b\":{\"x\":1,\"y\":2}}"));
}

void CanonicalJsonTest::testTimestampFormat()
{
    // Test with a known time point: 2025-11-17 13:45:12.123 UTC
    std::tm tm = {};
    tm.tm_year = 125;  // 2025 - 1900
    tm.tm_mon = 10;    // November (0-indexed)
    tm.tm_mday = 17;
    tm.tm_hour = 13;
    tm.tm_min = 45;
    tm.tm_sec = 12;
    
    std::time_t time_t = timegm(&tm);  // UTC time
    auto tp = std::chrono::system_clock::from_time_t(time_t);
    tp += std::chrono::milliseconds(123);  // Add 123 milliseconds
    
    std::string timestamp = to_canonical_timestamp(tp);
    
    // Verify format: 2025-11-17T13:45:12.123Z
    QCOMPARE(timestamp, std::string("2025-11-17T13:45:12.123Z"));
}

void CanonicalJsonTest::testGoldenJson()
{
    // Build a canonical structure
    std::map<std::string, CanonicalValue> obj;
    obj["timestamp"] = CanonicalValue{std::string{"2025-11-17T13:45:12.123Z"}};
    obj["version"] = CanonicalValue{int64_t{1}};
    obj["enabled"] = CanonicalValue{true};
    obj["name"] = CanonicalValue{std::string{"test"}};
    
    std::vector<CanonicalValue> tags;
    tags.push_back(CanonicalValue{std::string{"tag1"}});
    tags.push_back(CanonicalValue{std::string{"tag2"}});
    obj["tags"] = CanonicalValue{tags};
    
    CanonicalValue value = obj;
    std::string json = to_canonical_json(value);
    
    // Golden JSON string (keys sorted: enabled, name, tags, timestamp, version)
    std::string expected = R"({"enabled":true,"name":"test","tags":["tag1","tag2"],"timestamp":"2025-11-17T13:45:12.123Z","version":1})";
    
    QCOMPARE(json, expected);
}

void CanonicalJsonTest::testPrimitiveTypes()
{
    // Test null - wrap in object for proper JSON structure
    std::map<std::string, CanonicalValue> nullObj;
    nullObj["value"] = CanonicalValue{nullptr};
    std::string nullJson = to_canonical_json(CanonicalValue{nullObj});
    QCOMPARE(nullJson, std::string("{\"value\":null}"));
    
    // Test boolean
    std::map<std::string, CanonicalValue> boolObj;
    boolObj["flag"] = CanonicalValue{true};
    std::string boolJson = to_canonical_json(CanonicalValue{boolObj});
    QCOMPARE(boolJson, std::string("{\"flag\":true}"));
    
    // Test number
    std::map<std::string, CanonicalValue> numObj;
    numObj["value"] = CanonicalValue{int64_t{42}};
    std::map<std::string, CanonicalValue> doubleObj;
    doubleObj["pi"] = CanonicalValue{3.14159};
    std::string numJson = to_canonical_json(CanonicalValue{numObj});
    std::string doubleJson = to_canonical_json(CanonicalValue{doubleObj});
    QCOMPARE(numJson, std::string("{\"value\":42}"));
    QCOMPARE(doubleJson, std::string("{\"pi\":3.14159}"));
    
    // Test string
    std::map<std::string, CanonicalValue> strObj;
    strObj["text"] = CanonicalValue{std::string{"hello"}};
    std::string strJson = to_canonical_json(CanonicalValue{strObj});
    QCOMPARE(strJson, std::string("{\"text\":\"hello\"}"));
}

QTEST_MAIN(CanonicalJsonTest)
#include "canonical_json_tests.moc"

