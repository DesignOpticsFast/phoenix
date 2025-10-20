#pragma once

#include <QWidget>
#include <QString>
#include <QPen>
#include <QBrush>
#include <vector>

class QtGraphsPlotView : public QWidget
{
    Q_OBJECT

public:
    explicit QtGraphsPlotView(QWidget *parent = nullptr);
    ~QtGraphsPlotView() override;

    void setData(const std::vector<double>& xValues, const std::vector<double>& yValues);
    void setTitle(const QString& title);
    void setXLabel(const QString& label);
    void setYLabel(const QString& label);
    void clearData();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void setupPlot();
    void updatePlot();
    void applyDownsampling();
    
    QPen linePen_;
    QBrush backgroundBrush_;
    
    std::vector<double> xValues_;
    std::vector<double> yValues_;
    std::vector<double> xValuesDownsampled_;
    std::vector<double> yValuesDownsampled_;
    
    QString title_;
    QString xLabel_;
    QString yLabel_;
    
    double xMin_, xMax_, yMin_, yMax_;
    bool downsamplingEnabled_;
    static const int MAX_POINTS = 2000;
};




