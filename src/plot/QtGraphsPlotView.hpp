#pragma once

#include <QWidget>
#include <QChartView>
#include <QLineSeries>
#include <QChart>
#include <QValueAxis>
#include <QString>
#include <vector>

class QtChartsPlotView : public QChartView
{
    Q_OBJECT

public:
    explicit QtChartsPlotView(QWidget *parent = nullptr);
    ~QtChartsPlotView() override;

    void setData(const std::vector<double>& xValues, const std::vector<double>& yValues);
    void setTitle(const QString& title);
    void setXLabel(const QString& label);
    void setYLabel(const QString& label);
    void clearData();

private:
    void setupChart();
    void updateChart();
    void applyDownsampling();
    
    QChart* chart_;
    QLineSeries* series_;
    QValueAxis* xAxis_;
    QValueAxis* yAxis_;
    
    std::vector<double> xValues_;
    std::vector<double> yValues_;
    std::vector<double> xValuesDownsampled_;
    std::vector<double> yValuesDownsampled_;
    
    QString title_;
    QString xLabel_;
    QString yLabel_;
    
    bool downsamplingEnabled_;
    static const int MAX_POINTS = 2000;
};




