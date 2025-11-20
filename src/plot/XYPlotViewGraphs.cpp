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
#include <QFile>
#include <QFileInfo>
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
    qInfo() << "XYPlotViewGraphs: QML resource URL:" << qmlUrl;
    qInfo() << "XYPlotViewGraphs: QML engine search paths:" << m_quickWidget->engine()->importPathList();
    qInfo() << "XYPlotViewGraphs: QML engine base URL:" << m_quickWidget->engine()->baseUrl();
    
    // Verify resource exists before loading
    if (QFile::exists(":/qml/XYPlotView.qml")) {
        QFileInfo fileInfo(":/qml/XYPlotView.qml");
        qInfo() << "XYPlotViewGraphs: QML resource file exists, size:" << fileInfo.size() << "bytes";
    } else {
        qWarning() << "XYPlotViewGraphs: QML resource file NOT found at :/qml/XYPlotView.qml";
    }
    
    qDebug() << "XYPlotViewGraphs: Loading QML from:" << qmlUrl;
    m_quickWidget->setSource(qmlUrl);
    
    // Check for QML errors immediately after setSource
    QQuickWidget::Status status = m_quickWidget->status();
    if (status == QQuickWidget::Error) {
        qCritical() << "XYPlotViewGraphs: QML load failed, url=" << qmlUrl;
        const auto errors = m_quickWidget->errors();
        for (const auto &err : errors) {
            qCritical() << "  QML error:" << err.toString();
        }
    } else if (status == QQuickWidget::Ready) {
        qInfo() << "XYPlotViewGraphs: QML loaded successfully";
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
        qCritical() << "XYPlotViewGraphs: FATAL - Failed to load QML root object.";
        qCritical() << "XYPlotViewGraphs: Status:" << status;
        qCritical() << "XYPlotViewGraphs: QML URL:" << qmlUrl;
        qCritical() << "XYPlotViewGraphs: QML errors:" << m_quickWidget->errors();
        qCritical() << "XYPlotViewGraphs: Cannot proceed without QML root object - aborting initialization";
        return; // Abort initialization - widget will be unusable
    }
    
    if (status != QQuickWidget::Ready) {
        qCritical() << "XYPlotViewGraphs: FATAL - QML not in Ready state, status:" << status;
        qCritical() << "XYPlotViewGraphs: Cannot proceed - aborting initialization";
        return; // Abort initialization
    }
    
    qInfo() << "XYPlotViewGraphs: QML root object loaded successfully, type:" << m_rootItem->metaObject()->className();
    
    // ============================================================================
    // BINDING VERIFICATION: Verify QML structure matches C++ expectations
    // ============================================================================
    // Find GraphsView (should exist as child of root Item)
    QObject* graphsView = m_rootItem->findChild<QObject*>("graphsView", Qt::FindChildrenRecursively);
    if (!graphsView) {
        qCritical() << "XYPlotViewGraphs: FATAL - QML binding mismatch: graphsView not found";
        qCritical() << "XYPlotViewGraphs: Expected QML structure: Item { GraphsView { objectName: \"graphsView\" ... } }";
        qCritical() << "XYPlotViewGraphs: Root object type:" << m_rootItem->metaObject()->className();
        qCritical() << "XYPlotViewGraphs: Aborting initialization - QML structure does not match C++ expectations";
        return; // Abort initialization
    }
    qDebug() << "XYPlotViewGraphs: graphsView found, type:" << graphsView->metaObject()->className();
    
    // Find and verify mainSeries LineSeries object
    m_mainSeries = m_rootItem->findChild<QObject*>("mainSeries", Qt::FindChildrenRecursively);
    if (!m_mainSeries) {
        qCritical() << "XYPlotViewGraphs: FATAL - QML binding mismatch: mainSeries not found";
        qCritical() << "XYPlotViewGraphs: Expected QML: LineSeries { objectName: \"mainSeries\" ... }";
        qCritical() << "XYPlotViewGraphs: Root object type:" << m_rootItem->metaObject()->className();
        qCritical() << "XYPlotViewGraphs: Aborting initialization - cannot bind to series";
        return; // Abort initialization
    }
    
    // Verify mainSeries is a LineSeries (check metaObject className)
    QString seriesType = m_mainSeries->metaObject()->className();
    if (!seriesType.contains("LineSeries") && !seriesType.contains("QXYSeries")) {
        qCritical() << "XYPlotViewGraphs: FATAL - QML binding type mismatch: mainSeries is not LineSeries";
        qCritical() << "XYPlotViewGraphs: Found type:" << seriesType;
        qCritical() << "XYPlotViewGraphs: Expected: QLineSeries or QXYSeries";
        qCritical() << "XYPlotViewGraphs: Aborting initialization - incorrect QML type";
        return; // Abort initialization
    }
    qInfo() << "XYPlotViewGraphs: mainSeries verified, type:" << seriesType;
    
    // Verify mainSeries has required methods
    const QMetaObject* seriesMeta = m_mainSeries->metaObject();
    bool hasClear = false;
    bool hasReplace = false;
    for (int i = 0; i < seriesMeta->methodCount(); ++i) {
        QMetaMethod method = seriesMeta->method(i);
        if (method.name() == QByteArray("clear")) {
            hasClear = true;
        } else if (method.name() == QByteArray("replace")) {
            hasReplace = true;
        }
    }
    if (!hasClear) {
        qCritical() << "XYPlotViewGraphs: FATAL - mainSeries missing required method: clear()";
        qCritical() << "XYPlotViewGraphs: Aborting initialization - QML binding incomplete";
        return;
    }
    if (!hasReplace) {
        qCritical() << "XYPlotViewGraphs: FATAL - mainSeries missing required method: replace(QList<QPointF>)";
        qCritical() << "XYPlotViewGraphs: Aborting initialization - QML binding incomplete";
        return;
    }
    qDebug() << "XYPlotViewGraphs: mainSeries methods verified (clear, replace)";
    
    // Find and verify axis objects
    m_axisX = m_rootItem->findChild<QObject*>("axisX", Qt::FindChildrenRecursively);
    m_axisY = m_rootItem->findChild<QObject*>("axisY", Qt::FindChildrenRecursively);
    
    if (!m_axisX) {
        qCritical() << "XYPlotViewGraphs: FATAL - QML binding mismatch: axisX not found";
        qCritical() << "XYPlotViewGraphs: Expected QML: ValueAxis { objectName: \"axisX\" ... }";
        qCritical() << "XYPlotViewGraphs: Aborting initialization - cannot bind to X axis";
        return; // Abort initialization
    }
    
    if (!m_axisY) {
        qCritical() << "XYPlotViewGraphs: FATAL - QML binding mismatch: axisY not found";
        qCritical() << "XYPlotViewGraphs: Expected QML: ValueAxis { objectName: \"axisY\" ... }";
        qCritical() << "XYPlotViewGraphs: Aborting initialization - cannot bind to Y axis";
        return; // Abort initialization
    }
    
    // Verify axes are ValueAxis types (check metaObject className)
    QString axisXType = m_axisX->metaObject()->className();
    QString axisYType = m_axisY->metaObject()->className();
    if (!axisXType.contains("ValueAxis") && !axisXType.contains("AbstractAxis")) {
        qCritical() << "XYPlotViewGraphs: FATAL - QML binding type mismatch: axisX is not ValueAxis";
        qCritical() << "XYPlotViewGraphs: Found type:" << axisXType;
        qCritical() << "XYPlotViewGraphs: Expected: QValueAxis or QAbstractAxis";
        qCritical() << "XYPlotViewGraphs: Aborting initialization - incorrect QML type";
        return;
    }
    if (!axisYType.contains("ValueAxis") && !axisYType.contains("AbstractAxis")) {
        qCritical() << "XYPlotViewGraphs: FATAL - QML binding type mismatch: axisY is not ValueAxis";
        qCritical() << "XYPlotViewGraphs: Found type:" << axisYType;
        qCritical() << "XYPlotViewGraphs: Expected: QValueAxis or QAbstractAxis";
        qCritical() << "XYPlotViewGraphs: Aborting initialization - incorrect QML type";
        return;
    }
    qInfo() << "XYPlotViewGraphs: axisX verified, type:" << axisXType;
    qInfo() << "XYPlotViewGraphs: axisY verified, type:" << axisYType;
    
    // Verify axes have required properties (min/max)
    if (!m_axisX->property("min").isValid() && !m_axisX->property("minimum").isValid()) {
        qCritical() << "XYPlotViewGraphs: FATAL - axisX missing required properties: min/max or minimum/maximum";
        qCritical() << "XYPlotViewGraphs: Aborting initialization - cannot set axis ranges";
        return;
    }
    if (!m_axisY->property("min").isValid() && !m_axisY->property("minimum").isValid()) {
        qCritical() << "XYPlotViewGraphs: FATAL - axisY missing required properties: min/max or minimum/maximum";
        qCritical() << "XYPlotViewGraphs: Aborting initialization - cannot set axis ranges";
        return;
    }
    qDebug() << "XYPlotViewGraphs: Axis properties verified (min/max available)";
    
    qInfo() << "XYPlotViewGraphs: QML binding verification complete - all required objects found and verified";
    
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
    // Runtime binding gates: fail fast if QML binding is broken
    if (m_quickWidget->status() != QQuickWidget::Ready) {
        qCritical() << "XYPlotViewGraphs::clear - FATAL: QML not ready, status=" << m_quickWidget->status();
        qCritical() << "XYPlotViewGraphs::clear - Cannot clear - QML binding broken";
        return;
    }
    
    if (!m_quickWidget->rootObject()) {
        qCritical() << "XYPlotViewGraphs::clear - FATAL: rootObject is null";
        qCritical() << "XYPlotViewGraphs::clear - Cannot clear - QML root missing";
        return;
    }
    
    if (!m_mainSeries) {
        qCritical() << "XYPlotViewGraphs::clear - FATAL: mainSeries binding is null";
        qCritical() << "XYPlotViewGraphs::clear - QML binding mismatch - mainSeries not found during initialization";
        qCritical() << "XYPlotViewGraphs::clear - Cannot clear - aborting operation";
        return;
    }
    
    // Verify mainSeries still exists and is valid
    if (!m_mainSeries->metaObject()) {
        qCritical() << "XYPlotViewGraphs::clear - FATAL: mainSeries metaObject is null";
        qCritical() << "XYPlotViewGraphs::clear - QML object destroyed - aborting operation";
        return;
    }
    
    QMetaObject::invokeMethod(m_mainSeries, "clear");
}

void XYPlotViewGraphs::setData(const std::vector<QPointF>& points) {
    // Runtime binding gates: fail fast if QML binding is broken
    if (m_quickWidget->status() != QQuickWidget::Ready) {
        qCritical() << "XYPlotViewGraphs::setData - FATAL: QML not ready, status=" << m_quickWidget->status();
        qCritical() << "XYPlotViewGraphs::setData - Cannot set data - QML binding broken";
        return;
    }
    
    if (!m_quickWidget->rootObject()) {
        qCritical() << "XYPlotViewGraphs::setData - FATAL: rootObject is null";
        qCritical() << "XYPlotViewGraphs::setData - Cannot set data - QML root missing";
        return;
    }
    
    if (!m_mainSeries) {
        qCritical() << "XYPlotViewGraphs::setData - FATAL: mainSeries binding is null";
        qCritical() << "XYPlotViewGraphs::setData - QML binding mismatch - mainSeries not found during initialization";
        qCritical() << "XYPlotViewGraphs::setData - Cannot set data - aborting operation";
        return;
    }
    
    // Verify mainSeries still exists and is valid
    if (!m_mainSeries->metaObject()) {
        qCritical() << "XYPlotViewGraphs::setData - FATAL: mainSeries metaObject is null";
        qCritical() << "XYPlotViewGraphs::setData - QML object destroyed - aborting operation";
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
    // Lightweight guards: silent returns if QML not ready (no logging to avoid noise)
    if (m_quickWidget->status() != QQuickWidget::Ready) {
        return;
    }
    
    if (!m_quickWidget->rootObject()) {
        return;
    }
    
    // If no data, leave axes as-is (don't reset)
    if (points.empty()) {
        return;
    }
    
    // Runtime binding gates: verify axes are available
    if (!m_axisX || !m_axisY) {
        // Silent return - autoscaling is optional, but log at debug level
        qDebug() << "XYPlotViewGraphs::updateAxisRanges - Axes not available, skipping autoscaling";
        return;
    }
    
    // Verify axes are still valid
    if (!m_axisX->metaObject() || !m_axisY->metaObject()) {
        qDebug() << "XYPlotViewGraphs::updateAxisRanges - Axis objects invalid, skipping autoscaling";
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

