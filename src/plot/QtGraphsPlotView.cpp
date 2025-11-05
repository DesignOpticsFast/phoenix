#include "QtGraphsPlotView.hpp"
#include "app/PhxConstants.h"

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QBrush>
#include <QString>
#include <QDebug>
#include <algorithm>
#include <cmath>

QtGraphsPlotView::QtGraphsPlotView(QWidget *parent)
    : QWidget(parent)
    , xMin_(0.0), xMax_(1.0), yMin_(0.0), yMax_(1.0)
    , downsamplingEnabled_(false)
{
    setupPlot();
}

QtGraphsPlotView::~QtGraphsPlotView() = default;

void QtGraphsPlotView::setupPlot()
{
    // Set up pen and brush
    linePen_ = QPen(Qt::blue, 2);
    backgroundBrush_ = QBrush(Qt::white);
    
    // Set default properties
    setTitle("XY Sine Wave");
    setXLabel("X");
    setYLabel("Y");
    
    // Set initial axis ranges
    xMin_ = 0.0; xMax_ = 1.0;
    yMin_ = 0.0; yMax_ = 1.0;
}

void QtGraphsPlotView::setData(const std::vector<double>& xValues, const std::vector<double>& yValues)
{
    Q_UNUSED(xValues);
    Q_UNUSED(yValues);
    
    qWarning() << "[Plot] Deferred: plotting disabled this sprint; target points"
               << phx::plot::kTargetPoints;
    
    // Stub implementation - plotting deferred to Sprint 5
    // TODO: Implement actual plotting when Sprint 5 begins
}

void QtGraphsPlotView::setTitle(const QString& title)
{
    title_ = title;
    update();
}

void QtGraphsPlotView::setXLabel(const QString& label)
{
    xLabel_ = label;
    update();
}

void QtGraphsPlotView::setYLabel(const QString& label)
{
    yLabel_ = label;
    update();
}

void QtGraphsPlotView::clearData()
{
    xValues_.clear();
    yValues_.clear();
    xValuesDownsampled_.clear();
    yValuesDownsampled_.clear();
    update();
}

void QtGraphsPlotView::updatePlot()
{
    // Use downsampled data if available, otherwise original data
    const std::vector<double>& xData = downsamplingEnabled_ ? xValuesDownsampled_ : xValues_;
    const std::vector<double>& yData = downsamplingEnabled_ ? yValuesDownsampled_ : yValues_;
    
    if (xData.empty() || yData.empty()) {
        return;
    }
    
    // Update axis ranges
    if (!xData.empty() && !yData.empty()) {
        xMin_ = *std::min_element(xData.begin(), xData.end());
        xMax_ = *std::max_element(xData.begin(), xData.end());
        yMin_ = *std::min_element(yData.begin(), yData.end());
        yMax_ = *std::max_element(yData.begin(), yData.end());
        
        // Add some padding
        double xPadding = (xMax_ - xMin_) * 0.05;
        double yPadding = (yMax_ - yMin_) * 0.05;
        
        xMin_ -= xPadding;
        xMax_ += xPadding;
        yMin_ -= yPadding;
        yMax_ += yPadding;
    }
    
    update();
}

void QtGraphsPlotView::applyDownsampling()
{
    if (xValues_.size() <= MAX_POINTS) {
        downsamplingEnabled_ = false;
        return;
    }
    
    downsamplingEnabled_ = true;
    
    // Simple downsampling: take every nth point
    int step = xValues_.size() / MAX_POINTS;
    xValuesDownsampled_.clear();
    yValuesDownsampled_.clear();
    
    for (size_t i = 0; i < xValues_.size(); i += step) {
        xValuesDownsampled_.push_back(xValues_[i]);
        yValuesDownsampled_.push_back(yValues_[i]);
    }
    
    // Always include the last point
    if (xValuesDownsampled_.back() != xValues_.back()) {
        xValuesDownsampled_.push_back(xValues_.back());
        yValuesDownsampled_.push_back(yValues_.back());
    }
}

void QtGraphsPlotView::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    painter.fillRect(rect(), backgroundBrush_);
    
    // Draw title
    if (!title_.isEmpty()) {
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(rect().adjusted(10, 10, -10, -10), Qt::AlignTop | Qt::AlignHCenter, title_);
    }
    
    // Get data to plot
    const std::vector<double>& xData = downsamplingEnabled_ ? xValuesDownsampled_ : xValues_;
    const std::vector<double>& yData = downsamplingEnabled_ ? yValuesDownsampled_ : yValues_;
    
    if (xData.empty() || yData.empty()) {
        return;
    }
    
    // Calculate plot area (leave space for labels)
    QRect plotArea = rect().adjusted(60, 40, -20, -40);
    
    // Draw axes
    painter.setPen(Qt::black);
    painter.drawLine(plotArea.left(), plotArea.bottom(), plotArea.right(), plotArea.bottom());
    painter.drawLine(plotArea.left(), plotArea.top(), plotArea.left(), plotArea.bottom());
    
    // Draw axis labels
    painter.setFont(QFont("Arial", 10));
    painter.drawText(plotArea.adjusted(0, -30, 0, 0), Qt::AlignCenter, xLabel_);
    
    // Rotate for Y-axis label
    painter.save();
    painter.translate(20, plotArea.center().y());
    painter.rotate(-90);
    painter.drawText(QRect(-100, -50, 200, 100), Qt::AlignCenter, yLabel_);
    painter.restore();
    
    // Draw grid lines
    painter.setPen(QPen(Qt::lightGray, 1, Qt::DotLine));
    int numGridLines = 5;
    for (int i = 1; i < numGridLines; ++i) {
        int x = plotArea.left() + (plotArea.width() * i) / numGridLines;
        int y = plotArea.top() + (plotArea.height() * i) / numGridLines;
        painter.drawLine(x, plotArea.top(), x, plotArea.bottom());
        painter.drawLine(plotArea.left(), y, plotArea.right(), y);
    }
    
    // Draw data points
    painter.setPen(linePen_);
    QPointF prevPoint;
    bool firstPoint = true;
    
    for (size_t i = 0; i < xData.size(); ++i) {
        // Convert data coordinates to screen coordinates
        double x = plotArea.left() + (xData[i] - xMin_) / (xMax_ - xMin_) * plotArea.width();
        double y = plotArea.bottom() - (yData[i] - yMin_) / (yMax_ - yMin_) * plotArea.height();
        
        QPointF currentPoint(x, y);
        
        if (!firstPoint) {
            painter.drawLine(prevPoint, currentPoint);
        }
        
        prevPoint = currentPoint;
        firstPoint = false;
    }
}

#include "QtGraphsPlotView.moc"