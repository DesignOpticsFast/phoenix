#include "FeatureParameterPanel.hpp"
#include "features/ParamSpec.hpp"
#include "features/FeatureDescriptor.hpp"
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QDebug>

FeatureParameterPanel::FeatureParameterPanel(const FeatureDescriptor& descriptor, QWidget* parent)
    : QWidget(parent)
    , m_descriptor(descriptor)
    , m_formLayout(nullptr)
{
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        qInfo() << "[PANEL] Creating FeatureParameterPanel for feature:" << descriptor.id()
                << "displayName:" << descriptor.displayName()
                << "paramCount:" << descriptor.params().size();
    }
#endif
    setupUI();
#ifndef NDEBUG
    if (qEnvironmentVariableIsSet("PHOENIX_DEBUG_UI_LOG")) {
        QWidget* parentWidget = QWidget::parentWidget();
        qInfo() << "[PANEL] FeatureParameterPanel created - visible:" << isVisible()
                << "size:" << size()
                << "minSize:" << minimumSize()
                << "parent:" << (parentWidget ? parentWidget->metaObject()->className() : "nullptr");
    }
#endif
}

void FeatureParameterPanel::setupUI()
{
    m_formLayout = new QFormLayout(this);
    m_formLayout->setContentsMargins(10, 10, 10, 10);
    m_formLayout->setSpacing(8);
    
    // Create editor for each parameter
    // Use findParam() to get stable pointers to ParamSpecs stored in m_descriptor
    for (const ParamSpec& param : m_descriptor.params()) {
        QWidget* editor = createEditorForParam(param);
        if (editor) {
            m_formLayout->addRow(new QLabel(param.label() + ":", this), editor);
            m_editors.insert(param.name(), editor);
            // Use findParam() to get a stable pointer to the ParamSpec stored in m_descriptor
            const ParamSpec* stableSpec = m_descriptor.findParam(param.name());
            if (stableSpec) {
                m_paramSpecs.insert(param.name(), stableSpec);
            } else {
                qWarning() << "FeatureParameterPanel::setupUI: ParamSpec not found for" << param.name();
            }
        }
    }
    
    setLayout(m_formLayout);
}

QWidget* FeatureParameterPanel::createEditorForParam(const ParamSpec& spec)
{
    switch (spec.type()) {
        case ParamSpec::Type::Int: {
            QSpinBox* spinBox = new QSpinBox(this);
            if (spec.minValue().isValid()) {
                spinBox->setMinimum(spec.minValue().toInt());
            } else {
                spinBox->setMinimum(-999999);
            }
            if (spec.maxValue().isValid()) {
                spinBox->setMaximum(spec.maxValue().toInt());
            } else {
                spinBox->setMaximum(999999);
            }
            if (spec.defaultValue().isValid()) {
                spinBox->setValue(spec.defaultValue().toInt());
            }
            connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                    this, &FeatureParameterPanel::onParameterChanged);
            return spinBox;
        }
        case ParamSpec::Type::Double: {
            QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
            spinBox->setDecimals(6);  // Reasonable precision
            if (spec.minValue().isValid()) {
                spinBox->setMinimum(spec.minValue().toDouble());
            } else {
                spinBox->setMinimum(-999999.0);
            }
            if (spec.maxValue().isValid()) {
                spinBox->setMaximum(spec.maxValue().toDouble());
            } else {
                spinBox->setMaximum(999999.0);
            }
            if (spec.defaultValue().isValid()) {
                spinBox->setValue(spec.defaultValue().toDouble());
            }
            connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    this, &FeatureParameterPanel::onParameterChanged);
            return spinBox;
        }
        case ParamSpec::Type::Bool: {
            QCheckBox* checkBox = new QCheckBox(this);
            if (spec.defaultValue().isValid()) {
                checkBox->setChecked(spec.defaultValue().toBool());
            }
            connect(checkBox, &QCheckBox::toggled,
                    this, &FeatureParameterPanel::onParameterChanged);
            return checkBox;
        }
        case ParamSpec::Type::String: {
            QLineEdit* lineEdit = new QLineEdit(this);
            if (spec.defaultValue().isValid()) {
                lineEdit->setText(spec.defaultValue().toString());
            }
            connect(lineEdit, &QLineEdit::textChanged,
                    this, &FeatureParameterPanel::onParameterChanged);
            return lineEdit;
        }
        case ParamSpec::Type::Enum:
            // Enum support deferred to later chunk
            qWarning() << "FeatureParameterPanel: Enum type not yet supported for parameter" << spec.name();
            return nullptr;
    }
    return nullptr;
}

QVariant FeatureParameterPanel::getValueFromWidget(QWidget* widget, const ParamSpec& spec) const
{
    QVariant widgetValue;
    
    switch (spec.type()) {
        case ParamSpec::Type::Int: {
            QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget);
            if (spinBox) {
                widgetValue = QVariant(spinBox->value());
            }
            break;
        }
        case ParamSpec::Type::Double: {
            QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox*>(widget);
            if (spinBox) {
                widgetValue = QVariant(spinBox->value());
            }
            break;
        }
        case ParamSpec::Type::Bool: {
            QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget);
            if (checkBox) {
                widgetValue = QVariant(checkBox->isChecked());
            }
            break;
        }
        case ParamSpec::Type::String: {
            QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
            if (lineEdit) {
                widgetValue = QVariant(lineEdit->text());
            }
            break;
        }
        case ParamSpec::Type::Enum:
            // Enum support deferred
            break;
    }
    
    // Fall back to default if widget value is invalid
    if (!widgetValue.isValid() && spec.defaultValue().isValid()) {
        return spec.defaultValue();
    }
    
    return widgetValue;
}

void FeatureParameterPanel::setValueToWidget(QWidget* widget, const ParamSpec& spec, const QVariant& value) const
{
    if (!value.isValid()) {
        // Use default if value is invalid
        if (spec.defaultValue().isValid()) {
            setValueToWidget(widget, spec, spec.defaultValue());
        }
        return;
    }
    
    switch (spec.type()) {
        case ParamSpec::Type::Int: {
            QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget);
            if (spinBox) {
                spinBox->setValue(value.toInt());
            }
            break;
        }
        case ParamSpec::Type::Double: {
            QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox*>(widget);
            if (spinBox) {
                spinBox->setValue(value.toDouble());
            }
            break;
        }
        case ParamSpec::Type::Bool: {
            QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget);
            if (checkBox) {
                checkBox->setChecked(value.toBool());
            }
            break;
        }
        case ParamSpec::Type::String: {
            QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
            if (lineEdit) {
                lineEdit->setText(value.toString());
            }
            break;
        }
        case ParamSpec::Type::Enum:
            // Enum support deferred
            break;
    }
}

QMap<QString, QVariant> FeatureParameterPanel::parameters() const
{
    QMap<QString, QVariant> result;
    
    // First, collect values from widgets
    for (auto it = m_editors.begin(); it != m_editors.end(); ++it) {
        const QString& paramName = it.key();
        QWidget* widget = it.value();
        const ParamSpec* spec = m_paramSpecs.value(paramName);
        
        if (widget && spec) {
            QVariant value = getValueFromWidget(widget, *spec);
            result.insert(paramName, value);
        }
    }
    
    // Ensure all params have values (use defaults for missing)
    for (const ParamSpec& spec : m_descriptor.params()) {
        if (!result.contains(spec.name())) {
            if (spec.defaultValue().isValid()) {
                result.insert(spec.name(), spec.defaultValue());
            }
        }
    }
    
    return result;
}

void FeatureParameterPanel::setParameters(const QMap<QString, QVariant>& values)
{
    // Set provided values
    for (auto it = values.begin(); it != values.end(); ++it) {
        const QString& paramName = it.key();
        const QVariant& value = it.value();
        
        QWidget* widget = m_editors.value(paramName);
        const ParamSpec* spec = m_paramSpecs.value(paramName);
        
        if (widget && spec) {
            if (spec->isValid(value)) {
                setValueToWidget(widget, *spec, value);
            } else {
                // Invalid value - use default instead
                if (spec->defaultValue().isValid()) {
                    setValueToWidget(widget, *spec, spec->defaultValue());
                }
            }
        }
    }
    
    // Ensure all params have values (use defaults for missing ones)
    for (auto it = m_paramSpecs.begin(); it != m_paramSpecs.end(); ++it) {
        const QString& paramName = it.key();
        if (!values.contains(paramName)) {
            QWidget* widget = m_editors.value(paramName);
            const ParamSpec* spec = it.value();
            if (widget && spec && spec->defaultValue().isValid()) {
                setValueToWidget(widget, *spec, spec->defaultValue());
            }
        }
    }
}

QStringList FeatureParameterPanel::validationErrors() const
{
    QStringList errors;
    QMap<QString, QVariant> params = parameters();
    
    for (auto it = params.begin(); it != params.end(); ++it) {
        const QString& paramName = it.key();
        const QVariant& value = it.value();
        
        // Try to find spec from m_paramSpecs first (for params with widgets)
        const ParamSpec* spec = m_paramSpecs.value(paramName);
        
        // If not found in m_paramSpecs, search descriptor params (for params without widgets)
        // Use findParam() to get a stable pointer to the ParamSpec stored in m_descriptor
        if (!spec) {
            spec = m_descriptor.findParam(paramName);
        }
        
        if (spec) {
            if (!spec->isValid(value)) {
                QString error = spec->validationError(value);
                if (!error.isEmpty()) {
                    errors.append(error);
                }
            }
        } else {
            qWarning() << "FeatureParameterPanel::validationErrors: No spec found for param" << paramName;
        }
    }
    
    return errors;
}

bool FeatureParameterPanel::isValid() const
{
    return validationErrors().isEmpty();
}

void FeatureParameterPanel::onParameterChanged()
{
    emit parametersChanged(parameters());
}

