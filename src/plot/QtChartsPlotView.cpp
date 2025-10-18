#include "QtChartsPlotView.hpp"

#include <QWidget>
#include <QChartView>
#include <QLineSeries>
#include <QChart>
#include <QValueAxis>
#include <QString>
#include <QDebug>
#include <algorithm>
#include <cmath>

QtChartsPlotView::QtChartsPlotView(QWidget *parent)
    : QChartView(parent)
    , chart_(nullptr)
    , series_(nullptr)
    , xAxis_(nullptr)
    , yAxis_(nullptr)
    , downsamplingEnabled_(false)
{
    setupChart();
}

QtChartsPlotView::~QtChartsPlotView() = default;

void QtChartsPlotView::setupChart()
{
    // Create chart
    chart_ = new QChart();
    setChart(chart_);
    
    // Create series
    series_ = new QLineSeries();
    chart_->addSeries(series_);
    
    // Create axes
    xAxis_ = new QValueAxis();
    yAxis_ = new QValueAxis();
    
    chart_->addAxis(xAxis_, Qt::AlignBottom);
    chart_->addAxis(yAxis_, Qt::AlignLeft);
    
    series_->attachAxis(xAxis_);
    series_->attachAxis(yAxis_);
    
    // Set default properties
    chart_->setTitle("XY Sine Wave");
    xAxis_->setTitleText("X");
    yAxis_->setTitleText("Y");
    
    // Enable animations
    chart_->setAnimationOptions(QChart::SeriesAnimations);
    
    // Set theme
    chart_->setTheme(QChart::ChartThemeLight);
}

void QtChartsPlotView::setData(const std::vector<double>& xValues, const std::vector<double>& yValues)
{
    xValues_ = xValues;
    yValues_ = yValues;
    
    // Apply downsampling if needed
    applyDownsampling();
    
    updateChart();
}

void QtChartsPlotView::setTitle(const QString& title)
{
    title_ = title;
    if (chart_) {
        chart_->setTitle(title);
    }
}

void QtChartsPlotView::setXLabel(const QString& label)
{
    xLabel_ = label;
    if (xAxis_) {
        xAxis_->setTitleText(label);
    }
}

void QtChartsPlotView::setYLabel(const QString& label)
{
    yLabel_ = label;
    if (yAxis_) {
        yAxis_->setTitleText(label);
    }
}

void QtChartsPlotView::clearData()
{
    xValues_.clear();
    yValues_.clear();
    xValuesDownsampled_.clear();
    yValuesDownsampled_.clear();
    
    if (series_) {
        series_->clear();
    }
}

void QtChartsPlotView::updateChart()
{
    if (!series_) {
        return;
    }
    
    series_->clear();
    
    // Use downsampled data if available, otherwise original data
    const std::vector<double>& xData = downsamplingEnabled_ ? xValuesDownsampled_ : xValues_;
    const std::vector<double>& yData = downsamplingEnabled_ ? yValuesDownsampled_ : yValues_;
    
    if (xData.empty() || yData.empty()) {
        return;
    }
    
    // Add points to series
    for (size_t i = 0; i < xData.size(); ++i) {
        series_->append(xData[i], yData[i]);
    }
    
    // Update axis ranges
    if (!xData.empty() && !yData.empty()) {
        double xMin = *std::min_element(xData.begin(), xData.end());
        double xMax = *std::max_element(xData.begin(), xData.end());
        double yMin = *std::min_element(yData.begin(), yData.end());
        double yMax = *std::max_element(yData.begin(), yData.end());
        
        // Add some padding
        double xPadding = (xMax - xMin) * 0.05;
        double yPadding = (yMax - yMin) * 0.05;
        
        xAxis_->setRange(xMin - xPadding, xMax + xPadding);
        yAxis_->setRange(yMin - yPadding, yMax + yPadding);
    }
}

void QtChartsPlotView::applyDownsampling()
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

#include "QtChartsPlotView.moc"



