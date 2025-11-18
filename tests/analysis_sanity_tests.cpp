#include <QtTest/QtTest>
#include "ui/analysis/AnalysisWindow.hpp"
#include "plot/XYPlotViewGraphs.hpp"
#include <QPointF>
#include <QCoreApplication>
#include <vector>

class AnalysisSanityTests : public QObject {
    Q_OBJECT

private slots:
    void testAttachXYPlotView() {
        AnalysisWindow window;
        auto view = std::make_unique<XYPlotViewGraphs>();
        IAnalysisView* raw = view.get();
        
        // Process events to ensure QML is initialized
        QCoreApplication::processEvents();
        
        window.setView(std::move(view));
        
        // Process events again after setting view
        QCoreApplication::processEvents();
        
        QVERIFY(window.view() == raw);
        QVERIFY(window.view()->widget() != nullptr);
    }

    void testClearDoesNotCrash() {
        XYPlotViewGraphs view;
        view.clear(); // Should not crash or throw
        QVERIFY(true);
    }
    
    void testXYPlotViewGraphsWidget() {
        XYPlotViewGraphs view;
        QVERIFY(view.widget() != nullptr);
        // Verify QQuickWidget is created (widget should be non-null)
    }
    
    void testXYPlotViewGraphsSetData() {
        XYPlotViewGraphs view;
        std::vector<QPointF> points = {{0, 0}, {1, 1}, {2, 4}};
        view.setData(points);
        // Should not crash
        QVERIFY(true);
    }
    
    void testXYPlotViewGraphsClear() {
        XYPlotViewGraphs view;
        std::vector<QPointF> points = {{0, 0}, {1, 1}};
        view.setData(points);
        view.clear();
        // Should not crash
        QVERIFY(true);
    }
};

QTEST_MAIN(AnalysisSanityTests)
#include "analysis_sanity_tests.moc"

