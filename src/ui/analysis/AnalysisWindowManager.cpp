#include "ui/analysis/AnalysisWindowManager.hpp"
#include <QMainWindow>
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

