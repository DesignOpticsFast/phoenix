#include "ParamSpec.hpp"
#include <QDebug>

ParamSpec::ParamSpec()
    : m_type(Type::String)
{
}

ParamSpec::ParamSpec(const QString& name, const QString& label, Type type)
    : m_name(name)
    , m_label(label)
    , m_type(type)
{
}

ParamSpec& ParamSpec::setDefaultValue(const QVariant& value)
{
    m_defaultValue = value;
    return *this;
}

ParamSpec& ParamSpec::setMinValue(const QVariant& value)
{
    m_minValue = value;
    return *this;
}

ParamSpec& ParamSpec::setMaxValue(const QVariant& value)
{
    m_maxValue = value;
    return *this;
}

ParamSpec& ParamSpec::setEnumValues(const QStringList& values)
{
    m_enumValues = values;
    return *this;
}

bool ParamSpec::isValid(const QVariant& value) const
{
    if (!value.isValid()) {
        return false;
    }
    
    // Type checking
    switch (m_type) {
        case Type::Int:
            if (!value.canConvert<int>()) {
                return false;
            }
            break;
        case Type::Double:
            if (!value.canConvert<double>()) {
                return false;
            }
            break;
        case Type::Bool:
            if (!value.canConvert<bool>()) {
                return false;
            }
            break;
        case Type::String:
            if (!value.canConvert<QString>()) {
                return false;
            }
            break;
        case Type::Enum:
            if (!value.canConvert<QString>()) {
                return false;
            }
            break;
    }
    
    // Range checking for numeric types
    if (m_type == Type::Int || m_type == Type::Double) {
        bool ok = false;
        double numValue = value.toDouble(&ok);
        if (!ok) {
            return false;
        }
        
        if (m_minValue.isValid()) {
            double min = m_minValue.toDouble(&ok);
            if (ok && numValue < min) {
                return false;
            }
        }
        
        if (m_maxValue.isValid()) {
            double max = m_maxValue.toDouble(&ok);
            if (ok && numValue > max) {
                return false;
            }
        }
    }
    
    // Enum value checking
    if (m_type == Type::Enum) {
        QString strValue = value.toString();
        if (!m_enumValues.contains(strValue)) {
            return false;
        }
    }
    
    return true;
}

QString ParamSpec::displayName() const
{
    if (!m_label.isEmpty()) {
        return m_label;
    }
    if (!m_name.isEmpty()) {
        return m_name;
    }
    return QString("parameter");
}

QString ParamSpec::validationError(const QVariant& value) const
{
    const QString display = displayName();
    
    if (!value.isValid()) {
        return QString("Invalid value for parameter '%1'").arg(display);
    }
    
    // Type checking
    switch (m_type) {
        case Type::Int:
            if (!value.canConvert<int>()) {
                return QString("Parameter '%1' must be an integer").arg(display);
            }
            break;
        case Type::Double:
            if (!value.canConvert<double>()) {
                return QString("Parameter '%1' must be a number").arg(display);
            }
            break;
        case Type::Bool:
            if (!value.canConvert<bool>()) {
                return QString("Parameter '%1' must be a boolean").arg(display);
            }
            break;
        case Type::String:
            if (!value.canConvert<QString>()) {
                return QString("Parameter '%1' must be a string").arg(display);
            }
            break;
        case Type::Enum:
            if (!value.canConvert<QString>()) {
                return QString("Parameter '%1' must be a string").arg(display);
            }
            break;
    }
    
    // Range checking for numeric types
    if (m_type == Type::Int || m_type == Type::Double) {
        bool ok = false;
        double numValue = value.toDouble(&ok);
        if (!ok) {
            return QString("Parameter '%1' must be a number").arg(display);
        }
        
        if (m_minValue.isValid()) {
            double min = m_minValue.toDouble(&ok);
            if (ok && numValue < min) {
                return QString("Parameter '%1' must be >= %2").arg(display).arg(min);
            }
        }
        
        if (m_maxValue.isValid()) {
            double max = m_maxValue.toDouble(&ok);
            if (ok && numValue > max) {
                return QString("Parameter '%1' must be <= %2").arg(display).arg(max);
            }
        }
    }
    
    // Enum value checking
    if (m_type == Type::Enum) {
        QString strValue = value.toString();
        if (!m_enumValues.contains(strValue)) {
            return QString("Parameter '%1' must be one of: %2")
                .arg(display)
                .arg(m_enumValues.join(", "));
        }
    }
    
    return QString(); // No error
}

QString ParamSpec::valueToString(const QVariant& value) const
{
    switch (m_type) {
        case Type::Int:
            return QString::number(value.toInt());
        case Type::Double:
            return QString::number(value.toDouble());
        case Type::Bool:
            return value.toBool() ? "true" : "false";
        case Type::String:
        case Type::Enum:
            return value.toString();
    }
    return QString();
}

QVariant ParamSpec::stringToValue(const QString& str) const
{
    switch (m_type) {
        case Type::Int: {
            bool ok;
            int val = str.toInt(&ok);
            return ok ? QVariant(val) : QVariant();
        }
        case Type::Double: {
            bool ok;
            double val = str.toDouble(&ok);
            return ok ? QVariant(val) : QVariant();
        }
        case Type::Bool:
            return QVariant(str.toLower() == "true" || str == "1");
        case Type::String:
        case Type::Enum:
            return QVariant(str);
    }
    return QVariant();
}

