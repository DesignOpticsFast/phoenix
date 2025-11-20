#include "plot/XYPlotViewGraphs.hpp"

#include <QWidget>
#include <QVBoxLayout>
#include <QQuickWidget>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QDebug>
#include <QObject>
#include <QList>
#include <algorithm>
#include <cmath>

// Initialize QML resource
void initQmlResources() {
    Q_INIT_RESOURCE(phoenix_qml);
}

XYPlotViewGraphs::XYPlotViewGraphs()
    : m_container(new QWidget)
    , m_quickWidget(nullptr)
    , m_rootItem(nullptr)
    , m_mainSeries(nullptr)
    , m_axisX(nullptr)
    , m_axisY(nullptr)
{
    // Ensure QML resources are initialized
    static bool resourcesInitialized = false;
    if (!resourcesInitialized) {
        initQmlResources();
        resourcesInitialized = true;
    }
    
    auto* layout = new QVBoxLayout(m_container);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Create QQuickWidget for Qt Graphs QML
    m_quickWidget = new QQuickWidget(m_container);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    
    // Check QML file availability
    QUrl qmlUrl("qrc:/qml/XYPlotView.qml");
    qDebug() << "XYPlotViewGraphs: Loading QML from:" << qmlUrl;
    m_quickWidget->setSource(qmlUrl);
    
    // Check for QML errors immediately after setSource
    QQuickWidget::Status status = m_quickWidget->status();
    if (status == QQuickWidget::Error) {
        qWarning() << "XYPlotViewGraphs: QML load failed. Status: Error";
        qWarning() << "XYPlotViewGraphs: QML errors:" << m_quickWidget->errors();
        qWarning() << "XYPlotViewGraphs: QML URL was:" << qmlUrl;
    } else if (status == QQuickWidget::Ready) {
        qInfo() << "XYPlotViewGraphs: QML load succeeded";
    }
    
    // Get root item for accessing QML properties
    // Wait for QML to be ready (StatusRootObjectCreated means QML is loaded)
    if (status == QQuickWidget::Ready || status == QQuickWidget::Error) {
        m_rootItem = m_quickWidget->rootObject();
    }
    
    // If not ready yet, try to get it anyway (for cases where status hasn't updated)
    if (!m_rootItem) {
        m_rootItem = m_quickWidget->rootObject();
        status = m_quickWidget->status(); // Re-check status
    }
    
    if (!m_rootItem) {
        qWarning() << "XYPlotViewGraphs: Failed to load QML root object.";
        qWarning() << "XYPlotViewGraphs: Status:" << status;
        qWarning() << "XYPlotViewGraphs: QML URL:" << qmlUrl;
        qWarning() << "XYPlotViewGraphs: QML errors:" << m_quickWidget->errors();
    } else {
        qDebug() << "XYPlotViewGraphs: QML root object loaded successfully";
        
        // Find and store the mainSeries LineSeries object
        m_mainSeries = m_rootItem->findChild<QObject*>("mainSeries", Qt::FindChildrenRecursively);
        
        if (!m_mainSeries) {
            qWarning() << "XYPlotViewGraphs: mainSeries not found in QML";
            qWarning() << "XYPlotViewGraphs: Root object type:" << m_rootItem->metaObject()->className();
            qWarning() << "XYPlotViewGraphs: This may indicate a QML structure issue";
#ifndef NDEBUG
            Q_ASSERT(false);
#endif
        } else {
            qDebug() << "XYPlotViewGraphs: mainSeries found successfully";
        }
        
        // Find and cache axis objects for autoscaling
        m_axisX = m_rootItem->findChild<QObject*>("axisX", Qt::FindChildrenRecursively);
        m_axisY = m_rootItem->findChild<QObject*>("axisY", Qt::FindChildrenRecursively);
        
        if (!m_axisX) {
            qWarning() << "XYPlotViewGraphs: axisX not found in QML - autoscaling will be disabled";
        }
        if (!m_axisY) {
            qWarning() << "XYPlotViewGraphs: axisY not found in QML - autoscaling will be disabled";
        }
    }
    
    layout->addWidget(m_quickWidget);
    m_container->setLayout(layout);
}

XYPlotViewGraphs::~XYPlotViewGraphs() = default;

QWidget* XYPlotViewGraphs::widget() {
    return m_container;
}

void XYPlotViewGraphs::setTitle(const QString& title) {
    m_title = title;
    // TODO: Set title on Qt Graphs chart if QML API supports it
    // For now, just store the title
}

QString XYPlotViewGraphs::title() const {
    return m_title;
}

void XYPlotViewGraphs::clear() {
    if (!m_mainSeries) {
        qWarning() << "Cannot clear: mainSeries not available";
        return;
    }
    
    QMetaObject::invokeMethod(m_mainSeries, "clear");
}

void XYPlotViewGraphs::setData(const std::vector<QPointF>& points) {
    if (!m_mainSeries) {
        qWarning() << "Cannot set data: mainSeries not available";
        return;
    }
    
    // Convert std::vector<QPointF> to QList<QPointF> for bulk replace
    QList<QPointF> pointList;
    pointList.reserve(static_cast<int>(points.size()));
    for (const QPointF& point : points) {
        pointList.append(point);
    }
    
    // Use replace() for bulk update (more efficient than individual appends)
    QMetaObject::invokeMethod(m_mainSeries, "replace", 
                               Q_ARG(QList<QPointF>, pointList));
    
    // Update axis ranges to fit the data
    updateAxisRanges(points);
}

void XYPlotViewGraphs::updateAxisRanges(const std::vector<QPointF>& points) {
    // If no data, leave axes as-is (don't reset)
    if (points.empty()) {
        return;
    }
    
    // If axes aren't available, skip autoscaling
    if (!m_axisX || !m_axisY) {
        return;
    }
    
    // Compute min/max X and Y from data
    double minX = points[0].x();
    double maxX = points[0].x();
    double minY = points[0].y();
    double maxY = points[0].y();
    
    for (const QPointF& point : points) {
        minX = std::min(minX, point.x());
        maxX = std::max(maxX, point.x());
        minY = std::min(minY, point.y());
        maxY = std::max(maxY, point.y());
    }
    
    // Handle single point or constant X case
    if (minX == maxX) {
        const double dx = (minX == 0.0 ? 1.0 : std::abs(minX) * 0.1);
        minX -= dx;
        maxX += dx;
    }
    
    // Apply 10% padding to Y-axis
    double rangeY = maxY - minY;
    if (rangeY <= 0.0) {
        // Constant Y values - use 10% of absolute value or default to 1.0
        rangeY = (maxY == 0.0 ? 1.0 : std::abs(maxY) * 0.1);
    }
    
    const double pad = rangeY * 0.1;  // 10% padding
    const double newMinY = minY - pad;
    const double newMaxY = maxY + pad;
    
    // Set axis ranges using QObject property system
    // Try both "min"/"max" and "minimum"/"maximum" property names
    bool xSet = false;
    bool ySet = false;
    
    // Try "min"/"max" first (most common in Qt Graphs)
    if (m_axisX->property("min").isValid()) {
        m_axisX->setProperty("min", minX);
        m_axisX->setProperty("max", maxX);
        xSet = true;
    } else if (m_axisX->property("minimum").isValid()) {
        m_axisX->setProperty("minimum", minX);
        m_axisX->setProperty("maximum", maxX);
        xSet = true;
    }
    
    if (m_axisY->property("min").isValid()) {
        m_axisY->setProperty("min", newMinY);
        m_axisY->setProperty("max", newMaxY);
        ySet = true;
    } else if (m_axisY->property("minimum").isValid()) {
        m_axisY->setProperty("minimum", newMinY);
        m_axisY->setProperty("maximum", newMaxY);
        ySet = true;
    }
    
    if (!xSet) {
        qWarning() << "XYPlotViewGraphs: Could not set X axis range - property 'min'/'max' or 'minimum'/'maximum' not found";
    }
    if (!ySet) {
        qWarning() << "XYPlotViewGraphs: Could not set Y axis range - property 'min'/'max' or 'minimum'/'maximum' not found";
    }
}

