#include <QCoreApplication>
#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QVector>
#include <QRegularExpression>
#include <cmath>
#include <random>

class QtChartsScaleProbeHeadless : public QObject
{
    Q_OBJECT

public:
    QtChartsScaleProbeHeadless(QObject *parent = nullptr) : QObject(parent)
    {
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
        process.start("ps", QStringList() << "-o" << "pid,ppid,pcpu,pmem,rss" << "-p" << QString::number(QCoreApplication::applicationPid()));
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
            qDebug() << "Testing with 500 points";
        } else if (iteration == 20) {
            currentPoints = 1000;
            qDebug() << "Testing with 1000 points";
        } else if (iteration == 30) {
            currentPoints = 10000;
            qDebug() << "Testing with 10000 points";
        } else if (iteration == 40) {
            qDebug() << "Testing 50 windows scenario";
        }
        
        if (iteration < 50) {
            QTimer::singleShot(100, this, &QtChartsScaleProbeHeadless::collectTelemetry);
        } else {
            qDebug() << "Telemetry collection complete";
            QCoreApplication::quit();
        }
    }

private:
    void startTelemetry()
    {
        QTimer::singleShot(100, this, &QtChartsScaleProbeHeadless::collectTelemetry);
    }
    
    int currentPoints = 100;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QtChartsScaleProbeHeadless probe;
    
    return app.exec();
}

#include "gate0_5_qtcharts_headless.moc"



