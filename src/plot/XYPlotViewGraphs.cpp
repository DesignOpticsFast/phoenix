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
#include <QOpenGLContext>
#include <QSurfaceFormat>

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
    
    // Ensure QQuickWidget has a minimum size for proper rendering
    m_quickWidget->setMinimumSize(400, 300);
    
    // Runtime diagnostics: Check OpenGL context availability
    QOpenGLContext* glContext = QOpenGLContext::currentContext();
    if (glContext) {
        qDebug() << "XYPlotViewGraphs: OpenGL context available, version:" 
                 << glContext->format().majorVersion() << "." << glContext->format().minorVersion();
    } else {
        qWarning() << "XYPlotViewGraphs: No OpenGL context available - QQuickWidget may use software rendering";
    }
    
    // Runtime diagnostics: Check QQuickWidget initial state
    qDebug() << "XYPlotViewGraphs: QQuickWidget created, initial status:" << m_quickWidget->status();
    qDebug() << "XYPlotViewGraphs: QQuickWidget size:" << m_quickWidget->size();
    qDebug() << "XYPlotViewGraphs: Container size:" << m_container->size();
    
    // Check QML file availability
    QUrl qmlUrl("qrc:/qml/XYPlotView.qml");
    qDebug() << "XYPlotViewGraphs: Loading QML from:" << qmlUrl;
    m_quickWidget->setSource(qmlUrl);
    
    // Runtime diagnostics: Check QML loading status immediately after setSource
    qDebug() << "XYPlotViewGraphs: QML status immediately after setSource:" << m_quickWidget->status();
    
    // Check for QML errors and verify loading succeeded
    if (m_quickWidget->status() == QQuickWidget::Error) {
        qCritical() << "XYPlotViewGraphs: QML failed to load:" << m_quickWidget->errors();
        // QML failed - don't proceed with null root object
        // Widget will be non-functional but won't crash
        layout->addWidget(m_quickWidget);
        m_container->setLayout(layout);
        return; // Early return - QML failed to load
    } else if (m_quickWidget->status() == QQuickWidget::Ready) {
        qDebug() << "XYPlotViewGraphs: QML loaded successfully";
    } else {
        qDebug() << "XYPlotViewGraphs: QML loading status:" << m_quickWidget->status();
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
        qWarning() << "XYPlotViewGraphs: Failed to load QML root object. Status:" << m_quickWidget->status();
        qWarning() << "XYPlotViewGraphs: QML errors:" << m_quickWidget->errors();
    } else {
        qDebug() << "XYPlotViewGraphs: QML root object loaded successfully";
        qDebug() << "XYPlotViewGraphs: Root object type:" << m_rootItem->metaObject()->className();
        
        // Find and store the mainSeries LineSeries object
        m_mainSeries = m_rootItem->findChild<QObject*>("mainSeries", Qt::FindChildrenRecursively);
        
        if (!m_mainSeries) {
            qWarning() << "XYPlotViewGraphs: mainSeries not found in QML";
            qWarning() << "XYPlotViewGraphs: Available child objects:" << m_rootItem->findChildren<QObject*>();
#ifndef NDEBUG
            Q_ASSERT(false);
#endif
        } else {
            qDebug() << "XYPlotViewGraphs: mainSeries found successfully";
        }
    }
    
    layout->addWidget(m_quickWidget);
    m_container->setLayout(layout);
    
    // Runtime diagnostics: Final state
    qDebug() << "XYPlotViewGraphs: Constructor complete. QQuickWidget status:" << m_quickWidget->status();
    qDebug() << "XYPlotViewGraphs: QQuickWidget visible:" << m_quickWidget->isVisible();
    qDebug() << "XYPlotViewGraphs: Container visible:" << m_container->isVisible();
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
        qWarning() << "XYPlotViewGraphs::setData: mainSeries not available";
        qWarning() << "XYPlotViewGraphs::setData: QQuickWidget status:" << (m_quickWidget ? m_quickWidget->status() : -1);
        qWarning() << "XYPlotViewGraphs::setData: Root item:" << (m_rootItem ? "exists" : "null");
        return;
    }
    
    qDebug() << "XYPlotViewGraphs::setData: Setting" << points.size() << "data points";
    
    // Convert std::vector<QPointF> to QList<QPointF> for bulk replace
    QList<QPointF> pointList;
    pointList.reserve(static_cast<int>(points.size()));
    for (const QPointF& point : points) {
        pointList.append(point);
    }
    
    // Use replace() for bulk update (more efficient than individual appends)
    bool success = QMetaObject::invokeMethod(m_mainSeries, "replace", 
                               Q_ARG(QList<QPointF>, pointList));
    
    if (!success) {
        qWarning() << "XYPlotViewGraphs::setData: Failed to invoke replace() on mainSeries";
    } else {
        qDebug() << "XYPlotViewGraphs::setData: Data set successfully";
    }
}

