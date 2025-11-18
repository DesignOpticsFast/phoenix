#pragma once

#include <QString>

class QWidget;

class IAnalysisView {
public:
    virtual ~IAnalysisView() = default;

    // Widget that can be embedded in an AnalysisWindow or other container
    virtual QWidget* widget() = 0;

    // Optional: simple API for title / reset; we can expand later
    virtual void setTitle(const QString& title) = 0;
    virtual QString title() const = 0;

    virtual void clear() = 0;
};

