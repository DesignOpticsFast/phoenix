#pragma once

#include <QObject>
#include <QPointer>
#include <QList>

class QMainWindow;
class QDockWidget;

class AnalysisWindowManager : public QObject {
    Q_OBJECT

public:
    static AnalysisWindowManager* instance();
    
    // Register/unregister analysis windows
    void registerWindow(QMainWindow* window);
    void unregisterWindow(QMainWindow* window);
    
    // Register/unregister tool windows (QDockWidget)
    void registerToolWindow(QDockWidget* dock);
    void unregisterToolWindow(QDockWidget* dock);
    
    // Close all registered analysis windows
    void closeAll();
    
    // Close all registered tool windows
    void closeAllTools();
    
    // Close everything (analysis + tool windows)
    void closeAllWindows();
    
    // Get count of registered windows
    int windowCount() const { return m_windows.size(); }
    int toolWindowCount() const { return m_toolWindows.size(); }

private:
    AnalysisWindowManager(QObject* parent = nullptr);
    ~AnalysisWindowManager() override = default;
    
    QList<QPointer<QMainWindow>> m_windows;
    QList<QPointer<QDockWidget>> m_toolWindows;
    bool m_closingAll = false;  // Guard flag to prevent re-entrancy issues
    bool m_closingTools = false;  // Guard flag for tool window closing
    static AnalysisWindowManager* s_instance;
};

