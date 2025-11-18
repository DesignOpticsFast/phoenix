#include <QtTest/QtTest>
#include "plot/XYPlotViewGraphs.hpp"
#include <QElapsedTimer>
#include <QPointF>
#include <vector>
#include <cmath>

class GraphsPerfSanityTests : public QObject {
    Q_OBJECT

private slots:
    void test10kPointPerformance() {
        constexpr int pointCount = 10000;
        
        // Generate 10k-point sine wave
        std::vector<QPointF> points;
        points.reserve(pointCount);
        for (int i = 0; i < pointCount; ++i) {
            points.emplace_back(i, std::sin(i * 0.01));
        }
        
        XYPlotViewGraphs view;
        
        // Warm-up: one setData() call
        view.setData(points);
        view.clear();
        
        // Measure performance
        QElapsedTimer timer;
        timer.start();
        view.setData(points);
        qint64 elapsedMs = timer.elapsed();
        
        qDebug() << "[PERF] 10k-point update time:" << elapsedMs << "ms";
        
        // Assert reasonable performance (< 100 ms implies ≥10 updates/sec)
        QVERIFY(elapsedMs < 100);
    }
};

QTEST_MAIN(GraphsPerfSanityTests)
#include "graphs_perf_sanity.moc"

