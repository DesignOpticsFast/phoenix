#pragma once

#include <string>
#include <map>
#include <variant>
#include <vector>
#include <chrono>

namespace phoenix::json {

// Forward declare for recursive type
struct CanonicalValue;

// Canonical JSON value type
using CanonicalValueBase = std::variant<
    std::nullptr_t,
    bool,
    int64_t,
    double,
    std::string,
    std::vector<CanonicalValue>,
    std::map<std::string, CanonicalValue>
>;

// Wrap in struct to enable recursive definition
struct CanonicalValue : CanonicalValueBase {
    using CanonicalValueBase::variant;
    using CanonicalValueBase::operator=;
    
    template<typename T>
    CanonicalValue(T&& arg) : CanonicalValueBase(std::forward<T>(arg)) {}
};

/**
 * Serialize a CanonicalValue to canonical JSON string.
 * 
 * Canonical JSON means:
 * - Object keys are sorted lexicographically
 * - Nested objects also have sorted keys
 * - Compact format (no whitespace)
 * - Deterministic output
 */
std::string to_canonical_json(const CanonicalValue& value);

/**
 * Convert a time point to ISO 8601 UTC timestamp string with millisecond precision.
 * 
 * Format: YYYY-MM-DDTHH:MM:SS.sssZ
 * Example: 2025-11-17T13:45:12.123Z
 * 
 * @param tp Time point to convert
 * @return ISO 8601 UTC timestamp string
 */
std::string to_canonical_timestamp(std::chrono::system_clock::time_point tp);

} // namespace phoenix::json

