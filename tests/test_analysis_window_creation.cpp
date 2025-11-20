#include <QtTest/QtTest>
#include "ui/analysis/XYAnalysisWindow.hpp"
#include "plot/XYPlotViewGraphs.hpp"
#include <QApplication>
#include <QWidget>
#include <QPointF>
#include <vector>

class AnalysisWindowCreationTests : public QObject {
    Q_OBJECT

private slots:
    void testWindowCreation();
    void testWindowLoadsQML();
    void testWindowWithFeature();
};

void AnalysisWindowCreationTests::testWindowCreation()
{
    // Create window
    XYAnalysisWindow* window = new XYAnalysisWindow();
    
    // Verify window exists
    QVERIFY(window != nullptr);
    
    // Verify window has plot view
    QVERIFY(window->plotView() != nullptr);
    
    // Verify window title
    QCOMPARE(window->windowTitle(), QString("XY Plot Analysis"));
    
    // Cleanup
    window->deleteLater();
    QApplication::processEvents();
}

void AnalysisWindowCreationTests::testWindowLoadsQML()
{
    // Create window
    XYAnalysisWindow* window = new XYAnalysisWindow();
    
    // Set some test data
    std::vector<QPointF> points;
    points.emplace_back(0.0, 0.0);
    points.emplace_back(1.0, 1.0);
    points.emplace_back(2.0, 4.0);
    
    window->plotView()->setData(points);
    window->plotView()->setTitle("Test Plot");
    
    // Verify plot view widget exists
    QWidget* plotWidget = window->plotView()->widget();
    QVERIFY(plotWidget != nullptr);
    
    // Show window (this will trigger QML loading)
    window->show();
    QApplication::processEvents();
    
    // Give QML a moment to load
    QTest::qWait(100);
    QApplication::processEvents();
    
    // Verify window is visible
    QVERIFY(window->isVisible());
    
    // Cleanup
    window->close();
    window->deleteLater();
    QApplication::processEvents();
}

void AnalysisWindowCreationTests::testWindowWithFeature()
{
    // Create window
    XYAnalysisWindow* window = new XYAnalysisWindow();
    
    // Set feature (this should create parameter panel)
    window->setFeature("xy_sine");
    
    // Verify window still exists
    QVERIFY(window != nullptr);
    
    // Show window
    window->show();
    QApplication::processEvents();
    QTest::qWait(100);
    
    // Verify window is visible
    QVERIFY(window->isVisible());
    
    // Cleanup
    window->close();
    window->deleteLater();
    QApplication::processEvents();
}

QTEST_MAIN(AnalysisWindowCreationTests)
#include "test_analysis_window_creation.moc"

