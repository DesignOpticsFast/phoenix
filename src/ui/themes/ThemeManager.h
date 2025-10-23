#pragma once

#include <QObject>
#include <QApplication>
#include <QSettings>
#include <QStyle>
#include <QPalette>
#include <QString>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum class Theme {
        Light,
        Dark,
        System
    };

    static ThemeManager* instance();
    
    // Theme management
    void setTheme(Theme theme);
    void setDarkMode(bool dark);
    Theme currentTheme() const;
    bool isDarkMode() const;
    
    // System theme detection
    bool systemPrefersDark() const;
    
    // Theme change signals
    void themeChanged();
    
    // Settings
    void loadSettings();
    void saveSettings();

signals:
    void themeChanged(Theme theme);
    void darkModeChanged(bool isDark);

private:
    explicit ThemeManager(QObject* parent = nullptr);
    static ThemeManager* s_instance;
    
    Theme m_currentTheme;
    QSettings* m_settings;
    
    void applyTheme(Theme theme);
    void applyLightTheme();
    void applyDarkTheme();
    void applySystemTheme();
    QString loadStyleSheet(const QString& filename);
};
