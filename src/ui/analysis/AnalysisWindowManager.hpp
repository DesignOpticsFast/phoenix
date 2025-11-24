#pragma once

#include <QObject>
#include <QPointer>
#include <QList>

class QMainWindow;

class AnalysisWindowManager : public QObject {
    Q_OBJECT

public:
    static AnalysisWindowManager* instance();
    
    // Register/unregister analysis windows
    void registerWindow(QMainWindow* window);
    void unregisterWindow(QMainWindow* window);
    
    // Close all registered analysis windows
    void closeAll();
    
    // Get count of registered windows
    int windowCount() const { return m_windows.size(); }
    
    // Get list of all registered windows
    QList<QMainWindow*> windows() const;

private:
    AnalysisWindowManager(QObject* parent = nullptr);
    ~AnalysisWindowManager() override = default;
    
    QList<QPointer<QMainWindow>> m_windows;
    static AnalysisWindowManager* s_instance;
};

