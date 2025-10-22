#include "ThemeManager.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QDebug>

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager* ThemeManager::instance()
{
    if (!s_instance) {
        s_instance = new ThemeManager();
    }
    return s_instance;
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
    , m_currentTheme(Theme::System)
    , m_settings(new QSettings("Phoenix", "Phoenix", this))
{
    loadSettings();
}

void ThemeManager::setTheme(Theme theme)
{
    if (m_currentTheme == theme) return;
    
    m_currentTheme = theme;
    applyTheme(theme);
    saveSettings();
    
    emit themeChanged(theme);
    emit darkModeChanged(isDarkMode());
}

void ThemeManager::setDarkMode(bool dark)
{
    Theme newTheme = dark ? Theme::Dark : Theme::Light;
    setTheme(newTheme);
}

ThemeManager::Theme ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

bool ThemeManager::isDarkMode() const
{
    switch (m_currentTheme) {
        case Theme::Light:
            return false;
        case Theme::Dark:
            return true;
        case Theme::System:
            return systemPrefersDark();
    }
    return false;
}

bool ThemeManager::systemPrefersDark() const
{
    // Check system theme preference
    // This is a simplified implementation - in practice you'd check
    // platform-specific APIs for system theme detection
    QPalette systemPalette = QApplication::style()->standardPalette();
    return systemPalette.color(QPalette::Window).lightness() < 128;
}

void ThemeManager::themeChanged()
{
    emit themeChanged(m_currentTheme);
    emit darkModeChanged(isDarkMode());
}

void ThemeManager::loadSettings()
{
    QString themeString = m_settings->value("theme", "system").toString();
    if (themeString == "light") {
        m_currentTheme = Theme::Light;
    } else if (themeString == "dark") {
        m_currentTheme = Theme::Dark;
    } else {
        m_currentTheme = Theme::System;
    }
    
    applyTheme(m_currentTheme);
}

void ThemeManager::saveSettings()
{
    QString themeString;
    switch (m_currentTheme) {
        case Theme::Light:
            themeString = "light";
            break;
        case Theme::Dark:
            themeString = "dark";
            break;
        case Theme::System:
            themeString = "system";
            break;
    }
    m_settings->setValue("theme", themeString);
}

void ThemeManager::applyTheme(Theme theme)
{
    switch (theme) {
        case Theme::Light:
            applyLightTheme();
            break;
        case Theme::Dark:
            applyDarkTheme();
            break;
        case Theme::System:
            applySystemTheme();
            break;
    }
}

void ThemeManager::applyLightTheme()
{
    // Apply light theme
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::Base, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    lightPalette.setColor(QPalette::Text, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    lightPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    lightPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    QApplication::setPalette(lightPalette);
}

void ThemeManager::applyDarkTheme()
{
    // Apply dark theme
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::Text, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    QApplication::setPalette(darkPalette);
}

void ThemeManager::applySystemTheme()
{
    if (systemPrefersDark()) {
        applyDarkTheme();
    } else {
        applyLightTheme();
    }
}

QString ThemeManager::loadStyleSheet(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open stylesheet:" << filename;
        return QString();
    }
    
    QTextStream stream(&file);
    return stream.readAll();
}
