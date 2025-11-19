#pragma once

#include "features/FeatureDescriptor.hpp"
#include <QWidget>
#include <QMap>
#include <QVariant>
#include <QStringList>

class QFormLayout;
class QWidget;

class FeatureParameterPanel : public QWidget {
    Q_OBJECT

public:
    explicit FeatureParameterPanel(const FeatureDescriptor& descriptor, QWidget* parent = nullptr);
    
    // Get current parameter values
    QMap<QString, QVariant> parameters() const;
    
    // Set parameter values (validates and applies defaults for missing params)
    void setParameters(const QMap<QString, QVariant>& values);
    
    // Get validation errors (empty if all valid)
    QStringList validationErrors() const;
    
    // Check if all parameters are valid
    bool isValid() const;

signals:
    void parametersChanged(const QMap<QString, QVariant>& values);

private slots:
    void onParameterChanged();

private:
    void setupUI();
    QWidget* createEditorForParam(const ParamSpec& spec);
    QVariant getValueFromWidget(QWidget* widget, const ParamSpec& spec) const;
    void setValueToWidget(QWidget* widget, const ParamSpec& spec, const QVariant& value) const;
    
    FeatureDescriptor m_descriptor;
    QFormLayout* m_formLayout;
    QMap<QString, QWidget*> m_editors;  // param name -> editor widget
    QMap<QString, const ParamSpec*> m_paramSpecs;  // param name -> spec
};

