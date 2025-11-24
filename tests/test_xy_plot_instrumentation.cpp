// Minimal test to programmatically trigger XY Plot and capture instrumentation logs
// This allows us to capture logs without manual GUI interaction

#include <QApplication>
#include <QTimer>
#include "ui/analysis/XYAnalysisWindow.hpp"
#include "features/FeatureRegistry.hpp"
#include <QDebug>
#include <iostream>

int main(int argc, char** argv)
{
    // Set up environment for debug logging
    qputenv("PHOENIX_DEBUG_UI_LOG", "1");
    
    QApplication app(argc, argv);
    
    // Initialize FeatureRegistry
    FeatureRegistry::instance().registerDefaultFeatures();
    
    qInfo() << "[TEST] Creating XYAnalysisWindow programmatically...";
    
    // Create XYAnalysisWindow (no parent, like MainWindow does)
    XYAnalysisWindow* win = new XYAnalysisWindow(nullptr);
    
    qInfo() << "[TEST] Window created, calling setFeature(\"xy_sine\")...";
    
    // Trigger parameter panel setup
    win->setFeature("xy_sine");
    
    qInfo() << "[TEST] Dumping widget tree after setFeature():";
    win->dumpWidgetTree();
    
    // Show the window (this will trigger showEvent and more logging)
    qInfo() << "[TEST] Showing window...";
    win->show();
    
    // Process events to ensure showEvent is called
    QApplication::processEvents();
    QApplication::processEvents();
    
    qInfo() << "[TEST] Dumping widget tree after show():";
    win->dumpWidgetTree();
    
    // Wait a moment for any async operations
    QTimer::singleShot(100, &app, &QApplication::quit);
    
    qInfo() << "[TEST] Running event loop for 100ms...";
    int result = app.exec();
    
    qInfo() << "[TEST] Test complete, exiting.";
    
    delete win;
    return result;
}

