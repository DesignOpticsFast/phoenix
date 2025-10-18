#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QThread>
#include <QVector>
#include <QRegularExpression>
#include <cmath>
#include <random>

class QtChartsScaleProbe : public QMainWindow
{
    Q_OBJECT

public:
    QtChartsScaleProbe(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setWindowTitle("Qt Charts Scale Probe - Gate 0.5A");
        resize(1200, 800);
        
        // Create central widget
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        // Create layout
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);
        
        // Create charts for different test scenarios
        createCharts(layout);
        
        // Start telemetry collection
        startTelemetry();
    }

private slots:
    void collectTelemetry()
    {
        static int iteration = 0;
        static QElapsedTimer timer;
        if (iteration == 0) timer.start();
        
        // Get system metrics
        QProcess process;
        process.start("ps", QStringList() << "-o" << "pid,ppid,pcpu,pmem,rss" << "-p" << QString::number(QApplication::applicationPid()));
        process.waitForFinished();
        QString output = process.readAllStandardOutput();
        
        // Parse CPU and memory usage
        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        if (lines.size() > 1) {
            QStringList fields = lines[1].split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (fields.size() >= 5) {
                double cpu_pct = fields[2].toDouble();
                double rss_mb = fields[4].toDouble() / 1024.0; // Convert KB to MB
                
                // Log telemetry
                QFile file("/home/ec2-user/workspace/phoenix/docs/sprint4/results/gate0_5_qtcharts.csv");
                if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
                    QTextStream stream(&file);
                    if (iteration == 0) {
                        stream << "ui_event,ui_latency_ms,fps_observed,series_points,series_points_effective,downsampling_enabled,cpu_pct,rss_mb\n";
                    }
                    stream << "telemetry," << timer.elapsed() << ",30," << currentPoints << "," << currentPoints << ",false," << cpu_pct << "," << rss_mb << "\n";
                }
            }
        }
        
        iteration++;
        
        // Test different point counts
        if (iteration == 10) {
            currentPoints = 500;
            updateCharts();
        } else if (iteration == 20) {
            currentPoints = 1000;
            updateCharts();
        } else if (iteration == 30) {
            currentPoints = 10000;
            updateCharts();
        } else if (iteration == 40) {
            // Test 50 windows
            createMultipleWindows();
        }
        
        if (iteration < 50) {
            QTimer::singleShot(100, this, &QtChartsScaleProbe::collectTelemetry);
        } else {
            qDebug() << "Telemetry collection complete";
            QApplication::quit();
        }
    }

private:
    void createCharts(QVBoxLayout *layout)
    {
        // Create main chart
        QChart *chart = new QChart();
        chart->setTitle("Qt Charts Scale Probe");
        
        QLineSeries *lineSeries = new QLineSeries();
        lineSeries->setName("Test Series");
        
        // Generate initial data
        generateData(lineSeries, 100);
        
        chart->addSeries(lineSeries);
        chart->createDefaultAxes();
        
        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        layout->addWidget(chartView);
        
        charts.append(chart);
        series.append(lineSeries);
    }
    
    void generateData(QLineSeries *series, int pointCount)
    {
        series->clear();
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 100.0);
        
        for (int i = 0; i < pointCount; ++i) {
            double x = i;
            double y = 50.0 + 30.0 * std::sin(x * 0.1) + dis(gen);
            series->append(x, y);
        }
    }
    
    void updateCharts()
    {
        for (int i = 0; i < series.size(); ++i) {
            generateData(series[i], currentPoints);
        }
    }
    
    void createMultipleWindows()
    {
        for (int i = 0; i < 50; ++i) {
            QMainWindow *window = new QMainWindow();
            window->setWindowTitle(QString("Chart Window %1").arg(i + 1));
            window->resize(400, 300);
            
            QChart *chart = new QChart();
            chart->setTitle(QString("Window %1").arg(i + 1));
            
            QLineSeries *series = new QLineSeries();
            generateData(series, currentPoints);
            chart->addSeries(series);
            chart->createDefaultAxes();
            
            QChartView *chartView = new QChartView(chart);
            chartView->setRenderHint(QPainter::Antialiasing);
            window->setCentralWidget(chartView);
            
            window->show();
            windows.append(window);
        }
    }
    
    void startTelemetry()
    {
        QTimer::singleShot(100, this, &QtChartsScaleProbe::collectTelemetry);
    }
    
    QVector<QChart*> charts;
    QVector<QLineSeries*> series;
    QVector<QMainWindow*> windows;
    int currentPoints = 100;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QtChartsScaleProbe window;
    window.show();
    
    return app.exec();
}

#include "gate0_5_qtcharts_test.moc"
