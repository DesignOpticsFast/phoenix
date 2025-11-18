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

// Initialize QML resource
void initQmlResources() {
    Q_INIT_RESOURCE(phoenix_qml);
}

XYPlotViewGraphs::XYPlotViewGraphs()
    : m_container(new QWidget)
    , m_quickWidget(nullptr)
    , m_rootItem(nullptr)
    , m_mainSeries(nullptr)
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
    qDebug() << "Loading QML from:" << qmlUrl;
    m_quickWidget->setSource(qmlUrl);
    
    // Check for QML errors
    if (m_quickWidget->status() == QQuickWidget::Error) {
        qWarning() << "QML errors:" << m_quickWidget->errors();
    }
    
    // Get root item for accessing QML properties
    // Wait for QML to be ready (StatusRootObjectCreated means QML is loaded)
    if (m_quickWidget->status() == QQuickWidget::Ready || m_quickWidget->status() == QQuickWidget::Error) {
        m_rootItem = m_quickWidget->rootObject();
    }
    
    // If not ready yet, try to get it anyway (for cases where status hasn't updated)
    if (!m_rootItem) {
        m_rootItem = m_quickWidget->rootObject();
    }
    
    if (!m_rootItem) {
        qWarning() << "Failed to load QML root object. Status:" << m_quickWidget->status();
        qWarning() << "QML errors:" << m_quickWidget->errors();
    } else {
        // Find and store the mainSeries LineSeries object
        m_mainSeries = m_rootItem->findChild<QObject*>("mainSeries", Qt::FindChildrenRecursively);
        
        if (!m_mainSeries) {
            qWarning() << "XYPlotViewGraphs: mainSeries not found in QML";
#ifndef NDEBUG
            Q_ASSERT(false);
#endif
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
}

