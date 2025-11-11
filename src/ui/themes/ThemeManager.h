#pragma once

#include <QObject>
#include <QApplication>
#include <QSettings>
#include <QPointer>
#include <QString>

class SettingsProvider;

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
    static void setSettingsProvider(SettingsProvider* sp);
    
    // Theme management
    void setTheme(Theme theme);
    void setDarkMode(bool dark);
    Theme currentTheme() const;
    bool isDarkMode() const;
    
    // System theme detection
    bool systemPrefersDark() const;
    void applySystemTheme();
    
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
    static QPointer<SettingsProvider> provider_;
    
    Theme m_currentTheme;
    Theme m_activeTheme;
    bool m_paletteInitialized = false;
    
    void applyTheme(Theme theme);
    void applyLightTheme();
    void applyDarkTheme();
    Theme resolveSystemTheme() const;
    bool applyResolvedTheme(Theme resolvedTheme);
    void emitThemeSignals();
    QString loadStyleSheet(const QString& filename);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
};
