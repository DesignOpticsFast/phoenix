#pragma once

#include <QString>
#include <QStringList>
#include <QVariant>

class ParamSpec {
public:
    enum class Type {
        Int,
        Double,
        Bool,
        String,
        Enum
    };

    ParamSpec();
    ParamSpec(const QString& name, const QString& label, Type type);
    
    // Getters
    QString name() const { return m_name; }
    QString label() const { return m_label; }
    Type type() const { return m_type; }
    QVariant defaultValue() const { return m_defaultValue; }
    QVariant minValue() const { return m_minValue; }
    QVariant maxValue() const { return m_maxValue; }
    QStringList enumValues() const { return m_enumValues; }
    
    // Setters (fluent API for building)
    ParamSpec& setDefaultValue(const QVariant& value);
    ParamSpec& setMinValue(const QVariant& value);
    ParamSpec& setMaxValue(const QVariant& value);
    ParamSpec& setEnumValues(const QStringList& values);
    
    // Validation
    bool isValid(const QVariant& value) const;
    QString validationError(const QVariant& value) const;
    
    // Conversion to/from string (for proto map<string, string>)
    QString valueToString(const QVariant& value) const;
    QVariant stringToValue(const QString& str) const;

private:
    QString m_name;           // e.g., "frequency"
    QString m_label;          // e.g., "Frequency"
    Type m_type;              // Int, Double, Bool, String, Enum
    QVariant m_defaultValue;  // Default value
    QVariant m_minValue;      // Min (for numeric types)
    QVariant m_maxValue;      // Max (for numeric types)
    QStringList m_enumValues; // For Enum type
};

