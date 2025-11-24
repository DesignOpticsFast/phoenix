#include "ui/analysis/AnalysisWindowManager.hpp"
#include <QMainWindow>
#include <QDockWidget>
#include <QCoreApplication>
#include <QDebug>

AnalysisWindowManager* AnalysisWindowManager::s_instance = nullptr;

AnalysisWindowManager::AnalysisWindowManager(QObject* parent)
    : QObject(parent)
{
}

AnalysisWindowManager* AnalysisWindowManager::instance()
{
    if (!s_instance) {
        s_instance = new AnalysisWindowManager();
    }
    return s_instance;
}

void AnalysisWindowManager::registerWindow(QMainWindow* window)
{
    if (!window) {
        qWarning() << "AnalysisWindowManager: Attempted to register null window";
        return;
    }
    
    // Check if already registered
    for (const QPointer<QMainWindow>& ptr : m_windows) {
        if (ptr.data() == window) {
            qDebug() << "AnalysisWindowManager: Window already registered";
            return;
        }
    }
    
    m_windows.append(QPointer<QMainWindow>(window));
    qDebug() << "AnalysisWindowManager: Registered window, total count:" << m_windows.size();
}

void AnalysisWindowManager::unregisterWindow(QMainWindow* window)
{
    if (!window) {
        return;
    }
    
    // If we're in the middle of closeAll(), just mark the pointer as null
    // Don't modify the container structure to avoid iterator invalidation
    if (m_closingAll) {
        // Find and null out the pointer without removing it from the list
        for (QPointer<QMainWindow>& ptr : m_windows) {
            if (ptr.data() == window) {
                ptr.clear();  // Null out the pointer
                break;
            }
        }
        return;
    }
    
    // Normal unregistration: remove from list
    m_windows.removeAll(QPointer<QMainWindow>(window));
    
    // Also clean up any null pointers (from deleted windows)
    m_windows.removeAll(QPointer<QMainWindow>());
    
    qDebug() << "AnalysisWindowManager: Unregistered window, total count:" << m_windows.size();
}

void AnalysisWindowManager::closeAll()
{
    qDebug() << "AnalysisWindowManager: Closing all" << m_windows.size() << "analysis windows";
    
    // Set guard flag to prevent unregisterWindow() from modifying container during iteration
    m_closingAll = true;
    
    // Take a snapshot copy to iterate over (prevents iterator invalidation)
    const QList<QPointer<QMainWindow>> windowsSnapshot = m_windows;
    
    // Close all windows (this will trigger their closeEvent, which calls unregisterWindow)
    for (const QPointer<QMainWindow>& ptr : windowsSnapshot) {
        if (ptr) {
            ptr->close();
        }
    }
    
    // Process events to ensure close events are processed
    QCoreApplication::processEvents();
    
    // Reset guard flag
    m_closingAll = false;
    
    // Clean up null pointers and any remaining entries
    m_windows.removeAll(QPointer<QMainWindow>());
    
    qDebug() << "AnalysisWindowManager: Closed all windows, remaining count:" << m_windows.size();
}

void AnalysisWindowManager::registerToolWindow(QDockWidget* dock)
{
    if (!dock) {
        qWarning() << "AnalysisWindowManager: Attempted to register null tool window";
        return;
    }
    
    // Check if already registered
    for (const QPointer<QDockWidget>& ptr : m_toolWindows) {
        if (ptr.data() == dock) {
            qDebug() << "AnalysisWindowManager: Tool window already registered";
            return;
        }
    }
    
    m_toolWindows.append(QPointer<QDockWidget>(dock));
    qDebug() << "AnalysisWindowManager: Registered tool window, total count:" << m_toolWindows.size();
}

void AnalysisWindowManager::unregisterToolWindow(QDockWidget* dock)
{
    if (!dock) {
        return;
    }
    
    // If we're in the middle of closeAllTools(), just mark the pointer as null
    if (m_closingTools) {
        for (QPointer<QDockWidget>& ptr : m_toolWindows) {
            if (ptr.data() == dock) {
                ptr.clear();
                break;
            }
        }
        return;
    }
    
    // Normal unregistration: remove from list
    m_toolWindows.removeAll(QPointer<QDockWidget>(dock));
    m_toolWindows.removeAll(QPointer<QDockWidget>());
    
    qDebug() << "AnalysisWindowManager: Unregistered tool window, total count:" << m_toolWindows.size();
}

void AnalysisWindowManager::closeAllTools()
{
    qDebug() << "AnalysisWindowManager: Closing all" << m_toolWindows.size() << "tool windows";
    
    m_closingTools = true;
    
    // Take snapshot to prevent iterator invalidation
    const QList<QPointer<QDockWidget>> toolsSnapshot = m_toolWindows;
    
    // Close all tool windows
    for (const QPointer<QDockWidget>& ptr : toolsSnapshot) {
        if (ptr) {
            ptr->close();
        }
    }
    
    // Process events
    QCoreApplication::processEvents();
    
    m_closingTools = false;
    
    // Clean up null pointers
    m_toolWindows.removeAll(QPointer<QDockWidget>());
    
    qDebug() << "AnalysisWindowManager: Closed all tool windows, remaining count:" << m_toolWindows.size();
}

void AnalysisWindowManager::closeAllWindows()
{
    // Close analysis windows first, then tool windows
    closeAll();
    closeAllTools();
}

void AnalysisWindowManager::raiseAllAnalysisWindows()
{
    // Take snapshot to avoid iterator invalidation
    const QList<QPointer<QMainWindow>> snapshot = m_windows;
    
    // Raise all visible analysis windows
    for (const QPointer<QMainWindow>& ptr : snapshot) {
        if (!ptr) {
            continue;
        }
        if (!ptr->isVisible()) {
            continue;
        }
        ptr->raise();
        // Do NOT call activateWindow() to avoid stealing focus from MainWindow
    }
}

QList<QMainWindow*> AnalysisWindowManager::windows() const
{
    QList<QMainWindow*> result;
    for (const QPointer<QMainWindow>& ptr : m_windows) {
        if (ptr) {
            result.append(ptr.data());
        }
    }
    return result;
}

