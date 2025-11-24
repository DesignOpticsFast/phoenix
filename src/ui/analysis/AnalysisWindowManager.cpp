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
    
    // Remove all pointers to this window (including null pointers from deleted windows)
    m_windows.removeAll(QPointer<QMainWindow>(window));
    
    // Also clean up any null pointers (from deleted windows)
    m_windows.removeAll(QPointer<QMainWindow>());
    
    qDebug() << "AnalysisWindowManager: Unregistered window, total count:" << m_windows.size();
}

void AnalysisWindowManager::closeAll()
{
    qDebug() << "AnalysisWindowManager: Closing all" << m_windows.size() << "analysis windows";
    
    // Close all windows (this will trigger their closeEvent)
    for (const QPointer<QMainWindow>& ptr : m_windows) {
        if (ptr) {
            ptr->close();
        }
    }
    
    // Process events to ensure close events are processed
    QCoreApplication::processEvents();
    
    // Clean up null pointers
    m_windows.removeAll(QPointer<QMainWindow>());
    
    qDebug() << "AnalysisWindowManager: Closed all windows, remaining count:" << m_windows.size();
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

