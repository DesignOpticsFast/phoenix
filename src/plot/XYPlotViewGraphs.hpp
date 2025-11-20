#pragma once

#include "ui/analysis/IAnalysisView.hpp"
#include <QString>
#include <QPointF>
#include <vector>

class QWidget;
class QQuickWidget;
class QQuickItem;
class QObject;

class XYPlotViewGraphs : public IAnalysisView {
public:
    XYPlotViewGraphs();
    ~XYPlotViewGraphs() override;

    QWidget* widget() override;

    void setTitle(const QString& title) override;
    QString title() const override;

    void clear() override;

    // Public API for setting XY data (for tests and future data integration)
    void setData(const std::vector<QPointF>& points);

private:
    void updateAxisRanges(const std::vector<QPointF>& points);
    
    QString m_title;
    QWidget* m_container;   // parent widget container
    QQuickWidget* m_quickWidget;  // QML container for Qt Graphs
    QQuickItem* m_rootItem;  // Root QML item
    QObject* m_mainSeries;  // QML LineSeries object for data updates
    QObject* m_axisX;        // QML ValueAxis object for X axis
    QObject* m_axisY;        // QML ValueAxis object for Y axis
};

