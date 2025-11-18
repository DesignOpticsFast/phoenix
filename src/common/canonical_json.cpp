#include "common/canonical_json.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QString>
#include <QByteArray>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace phoenix::json {

namespace {

// Convert CanonicalValue to QJsonValue
QJsonValue toQJsonValue(const CanonicalValue& value) {
    return std::visit([](const auto& arg) -> QJsonValue {
        using T = std::decay_t<decltype(arg)>;
        
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            return QJsonValue::Null;
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return static_cast<qint64>(arg);
        } else if constexpr (std::is_same_v<T, double>) {
            return arg;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return QString::fromStdString(arg);
        } else if constexpr (std::is_same_v<T, std::vector<CanonicalValue>>) {
            QJsonArray arr;
            for (const auto& item : arg) {
                arr.append(toQJsonValue(item));
            }
            return arr;
        } else if constexpr (std::is_same_v<T, std::map<std::string, CanonicalValue>>) {
            QJsonObject obj;
            // std::map is already sorted, so keys will be in order
            for (const auto& [key, val] : arg) {
                obj.insert(QString::fromStdString(key), toQJsonValue(val));
            }
            return obj;
        }
    }, value);
}

} // anonymous namespace

std::string to_canonical_json(const CanonicalValue& value) {
    QJsonValue jsonValue = toQJsonValue(value);
    QJsonDocument doc;
    
    if (jsonValue.isObject()) {
        doc = QJsonDocument(jsonValue.toObject());
    } else if (jsonValue.isArray()) {
        doc = QJsonDocument(jsonValue.toArray());
    } else {
        // For primitive values, create a QJsonDocument with just the value
        // QJsonDocument can handle this via QJsonDocument::fromVariant
        QJsonArray arr;
        arr.append(jsonValue);
        doc = QJsonDocument(arr);
        // Extract just the value part (remove array brackets)
        QByteArray bytes = doc.toJson(QJsonDocument::Compact);
        std::string result(bytes.constData(), bytes.size());
        // Remove [ and ] brackets
        if (result.size() >= 2 && result[0] == '[' && result[result.size()-1] == ']') {
            return result.substr(1, result.size() - 2);
        }
        return result;
    }
    
    // Use Compact format (no whitespace) for canonical JSON
    QByteArray jsonBytes = doc.toJson(QJsonDocument::Compact);
    return std::string(jsonBytes.constData(), jsonBytes.size());
}

std::string to_canonical_timestamp(std::chrono::system_clock::time_point tp) {
    // Convert to time_t
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    
    // Get milliseconds component
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        tp.time_since_epoch()
    ) % 1000;
    
    // Format as ISO 8601 UTC
    std::tm tm_buf;
    
    // Use portable gmtime (thread-safe version where available)
    #ifdef _WIN32
        gmtime_s(&tm_buf, &time_t);  // Windows
    #else
        gmtime_r(&time_t, &tm_buf);  // POSIX (Linux/macOS)
    #endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z";
    
    return oss.str();
}

} // namespace phoenix::json

