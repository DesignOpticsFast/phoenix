#pragma once

#include "ui/analysis/IAnalysisView.hpp"
#include <QString>
#include <QPointF>
#include <vector>

class QWidget;
class QQuickWidget;
class QQuickItem;
class QObject;
class QTimer;

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
    void initializeAxisRanges(const std::vector<QPointF>& points);
    void clampZoom();  // Clamp zoom values to limits
    
    QString m_title;
    QWidget* m_container;   // parent widget container
    QQuickWidget* m_quickWidget;  // QML container for Qt Graphs
    QQuickItem* m_rootItem;  // Root QML item
    QObject* m_mainSeries;  // QML LineSeries object for data updates
    QObject* m_axisX;        // QML ValueAxis object for X axis
    QObject* m_axisY;        // QML ValueAxis object for Y axis
    QTimer* m_zoomCheckTimer;  // Timer to periodically check and clamp zoom
    
    // Data bounding box tracking for zoom limits
    double m_dataMinX = 0.0;
    double m_dataMaxX = 0.0;
    double m_dataMinY = 0.0;
    double m_dataMaxY = 0.0;
    double m_baseSpanX = 0.0;  // Padded span used for zoom limit calculation
    double m_baseSpanY = 0.0;  // Padded span used for zoom limit calculation
    double m_minZoom = 0.5;     // Minimum zoom (allows 2x zoom-out from base)
    double m_maxZoom = 100.0;   // Maximum zoom (reasonable upper bound)
};

