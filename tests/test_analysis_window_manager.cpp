#include <QtTest/QtTest>
#include "ui/analysis/AnalysisWindowManager.hpp"
#include <QMainWindow>
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
    void testWindowCount();
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

QTEST_MAIN(AnalysisWindowManagerTests)
#include "test_analysis_window_manager.moc"

