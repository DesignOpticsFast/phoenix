#include <QtTest/QtTest>
#include "ui/analysis/AnalysisWindowManager.hpp"
#include <QMainWindow>
#include <QDockWidget>
#include <QApplication>
#include <QSignalSpy>
#include <QCloseEvent>

class TestWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit TestWindow(QWidget* parent = nullptr) : QMainWindow(parent), m_closeEventReceived(false) {}
    
    bool closeEventReceived() const { return m_closeEventReceived; }
    
protected:
    void closeEvent(QCloseEvent* event) override {
        m_closeEventReceived = true;
        QMainWindow::closeEvent(event);
    }

private:
    bool m_closeEventReceived;
};

class AnalysisWindowManagerTests : public QObject {
    Q_OBJECT

private slots:
    void testSingleton();
    void testRegisterWindow();
    void testUnregisterWindow();
    void testCloseAll();
    void testCloseAllWithMultipleWindows();
    void testWindowCount();
    void testToolWindowRegistration();
    void testCloseAllTools();
    void testCloseAllWindows();
};

void AnalysisWindowManagerTests::testSingleton()
{
    AnalysisWindowManager* mgr1 = AnalysisWindowManager::instance();
    AnalysisWindowManager* mgr2 = AnalysisWindowManager::instance();
    
    // Should return the same instance
    QVERIFY(mgr1 == mgr2);
    QVERIFY(mgr1 != nullptr);
}

void AnalysisWindowManagerTests::testRegisterWindow()
{
    AnalysisWindowManager* mgr = AnalysisWindowManager::instance();
    int initialCount = mgr->windowCount();
    
    TestWindow* window = new TestWindow();
    mgr->registerWindow(window);
    
    QCOMPARE(mgr->windowCount(), initialCount + 1);
    
    // Cleanup
    window->deleteLater();
    QApplication::processEvents();
}

void AnalysisWindowManagerTests::testUnregisterWindow()
{
    AnalysisWindowManager* mgr = AnalysisWindowManager::instance();
    
    TestWindow* window = new TestWindow();
    mgr->registerWindow(window);
    int countAfterRegister = mgr->windowCount();
    
    mgr->unregisterWindow(window);
    
    QCOMPARE(mgr->windowCount(), countAfterRegister - 1);
    
    // Cleanup
    window->deleteLater();
    QApplication::processEvents();
}

void AnalysisWindowManagerTests::testCloseAll()
{
    AnalysisWindowManager* mgr = AnalysisWindowManager::instance();
    
    // Create multiple test windows
    TestWindow* window1 = new TestWindow();
    TestWindow* window2 = new TestWindow();
    TestWindow* window3 = new TestWindow();
    
    mgr->registerWindow(window1);
    mgr->registerWindow(window2);
    mgr->registerWindow(window3);
    
    int countBeforeClose = mgr->windowCount();
    QVERIFY(countBeforeClose >= 3);
    
    // Close all windows
    mgr->closeAll();
    
    // Process events to ensure close events are processed
    QApplication::processEvents();
    
    // Verify windows received close events
    QVERIFY(window1->closeEventReceived());
    QVERIFY(window2->closeEventReceived());
    QVERIFY(window3->closeEventReceived());
    
    // Verify windows are no longer visible
    QVERIFY(!window1->isVisible());
    QVERIFY(!window2->isVisible());
    QVERIFY(!window3->isVisible());
    
    // Verify manager doesn't retain stale pointers
    // Calling closeAll() again should be a no-op (no windows registered)
    int countAfterClose = mgr->windowCount();
    QVERIFY(countAfterClose < countBeforeClose);
    
    // Cleanup
    window1->deleteLater();
    window2->deleteLater();
    window3->deleteLater();
    QApplication::processEvents();
}

void AnalysisWindowManagerTests::testWindowCount()
{
    AnalysisWindowManager* mgr = AnalysisWindowManager::instance();
    int initialCount = mgr->windowCount();
    
    TestWindow* window1 = new TestWindow();
    TestWindow* window2 = new TestWindow();
    
    mgr->registerWindow(window1);
    QCOMPARE(mgr->windowCount(), initialCount + 1);
    
    mgr->registerWindow(window2);
    QCOMPARE(mgr->windowCount(), initialCount + 2);
    
    mgr->unregisterWindow(window1);
    QCOMPARE(mgr->windowCount(), initialCount + 1);
    
    mgr->unregisterWindow(window2);
    QCOMPARE(mgr->windowCount(), initialCount);
    
    // Cleanup
    window1->deleteLater();
    window2->deleteLater();
    QApplication::processEvents();
}

void AnalysisWindowManagerTests::testCloseAllWithMultipleWindows()
{
    AnalysisWindowManager* mgr = AnalysisWindowManager::instance();
    
    // Create multiple windows
    TestWindow* w1 = new TestWindow();
    TestWindow* w2 = new TestWindow();
    TestWindow* w3 = new TestWindow();
    
    // Show them (so isVisible() check works)
    w1->show();
    w2->show();
    w3->show();
    QApplication::processEvents();
    
    mgr->registerWindow(w1);
    mgr->registerWindow(w2);
    mgr->registerWindow(w3);
    
    QCOMPARE(mgr->windowCount(), 3);
    
    // Verify all are visible before close
    QVERIFY(w1->isVisible());
    QVERIFY(w2->isVisible());
    QVERIFY(w3->isVisible());
    
    // Close all
    mgr->closeAll();
    QApplication::processEvents();
    
    // Verify all windows are closed
    QVERIFY(!w1->isVisible());
    QVERIFY(!w2->isVisible());
    QVERIFY(!w3->isVisible());
    
    // Verify all received close events
    QVERIFY(w1->closeEventReceived());
    QVERIFY(w2->closeEventReceived());
    QVERIFY(w3->closeEventReceived());
    
    // Verify manager is clean (no stale pointers)
    // Calling closeAll() again should be safe
    mgr->closeAll();
    QApplication::processEvents();
    
    // Cleanup
    w1->deleteLater();
    w2->deleteLater();
    w3->deleteLater();
    QApplication::processEvents();
}

void AnalysisWindowManagerTests::testToolWindowRegistration()
{
    AnalysisWindowManager* mgr = AnalysisWindowManager::instance();
    int initialCount = mgr->toolWindowCount();
    
    QMainWindow* parent = new QMainWindow();
    QDockWidget* dock1 = new QDockWidget("Test Dock 1", parent);
    QDockWidget* dock2 = new QDockWidget("Test Dock 2", parent);
    
    mgr->registerToolWindow(dock1);
    QCOMPARE(mgr->toolWindowCount(), initialCount + 1);
    
    mgr->registerToolWindow(dock2);
    QCOMPARE(mgr->toolWindowCount(), initialCount + 2);
    
    mgr->unregisterToolWindow(dock1);
    QCOMPARE(mgr->toolWindowCount(), initialCount + 1);
    
    mgr->unregisterToolWindow(dock2);
    QCOMPARE(mgr->toolWindowCount(), initialCount);
    
    // Cleanup
    parent->deleteLater();
    QApplication::processEvents();
}

void AnalysisWindowManagerTests::testCloseAllTools()
{
    AnalysisWindowManager* mgr = AnalysisWindowManager::instance();
    
    QMainWindow* parent = new QMainWindow();
    QDockWidget* dock1 = new QDockWidget("Test Dock 1", parent);
    QDockWidget* dock2 = new QDockWidget("Test Dock 2", parent);
    
    dock1->show();
    dock2->show();
    QApplication::processEvents();
    
    mgr->registerToolWindow(dock1);
    mgr->registerToolWindow(dock2);
    
    QVERIFY(dock1->isVisible());
    QVERIFY(dock2->isVisible());
    
    mgr->closeAllTools();
    QApplication::processEvents();
    
    QVERIFY(!dock1->isVisible());
    QVERIFY(!dock2->isVisible());
    
    // Cleanup
    parent->deleteLater();
    QApplication::processEvents();
}

void AnalysisWindowManagerTests::testCloseAllWindows()
{
    AnalysisWindowManager* mgr = AnalysisWindowManager::instance();
    
    // Create analysis windows
    TestWindow* w1 = new TestWindow();
    TestWindow* w2 = new TestWindow();
    
    // Create tool windows
    QMainWindow* parent = new QMainWindow();
    QDockWidget* dock1 = new QDockWidget("Test Dock", parent);
    
    // Show all
    w1->show();
    w2->show();
    dock1->show();
    QApplication::processEvents();
    
    // Register all
    mgr->registerWindow(w1);
    mgr->registerWindow(w2);
    mgr->registerToolWindow(dock1);
    
    QVERIFY(w1->isVisible());
    QVERIFY(w2->isVisible());
    QVERIFY(dock1->isVisible());
    
    // Close everything
    mgr->closeAllWindows();
    QApplication::processEvents();
    
    // Verify all closed
    QVERIFY(!w1->isVisible());
    QVERIFY(!w2->isVisible());
    QVERIFY(!dock1->isVisible());
    
    // Cleanup
    w1->deleteLater();
    w2->deleteLater();
    parent->deleteLater();
    QApplication::processEvents();
}

QTEST_MAIN(AnalysisWindowManagerTests)
#include "test_analysis_window_manager.moc"

