#include "ThemeManager.h"
#include "app/SettingsProvider.h"
#include "app/SettingsKeys.h"
#include <QApplication>
#include <QCoreApplication>
#include <QEvent>
#include <QGuiApplication>
#include <QStyleHints>
#include <QColor>
#include <QPalette>
#include <QStyleFactory>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include "app/Trace.hpp"
#include <QTimer>

ThemeManager* ThemeManager::s_instance = nullptr;
QPointer<SettingsProvider> ThemeManager::provider_ = nullptr;

ThemeManager* ThemeManager::instance()
{
    if (!s_instance) {
        s_instance = new ThemeManager();
    }
    return s_instance;
}

void ThemeManager::setSettingsProvider(SettingsProvider* sp)
{
    provider_ = sp;
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
    , m_currentTheme(Theme::System)
    , m_activeTheme(Theme::System)
{
    PHX_BOOT_TRACE("ThemeManager:ctor:begin");
    loadSettings(false);

    if (auto* hints = QGuiApplication::styleHints()) {
        connect(hints,
                &QStyleHints::colorSchemeChanged,
                this,
                [this](Qt::ColorScheme) {
                    if (m_currentTheme == Theme::System) {
                        applySystemTheme();
                        emitThemeSignals();
                    }
                });
    }

    if (auto* app = QCoreApplication::instance()) {
        app->installEventFilter(this);
    }

    QTimer::singleShot(0, this, [this]() {
        if (m_currentTheme == Theme::System) {
            applySystemTheme();
        } else {
            applyTheme(m_currentTheme);
            emitThemeSignals();
        }
        PHX_BOOT_TRACE("ThemeManager:ctor:end");
    });
}

void ThemeManager::setTheme(Theme theme)
{
    if (m_currentTheme == theme) {
        if (theme == Theme::System) {
            applySystemTheme();
        }
        return;
    }

    m_currentTheme = theme;
    applyTheme(theme);
    saveSettings();

    emitThemeSignals();
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
    auto* hints = QGuiApplication::styleHints();
    if (!hints) {
        return false;
    }
    return hints->colorScheme() == Qt::ColorScheme::Dark;
}

void ThemeManager::themeChanged()
{
    emitThemeSignals();
}

void ThemeManager::loadSettings(bool applyImmediately)
{
    auto* sp = provider_.data();
    if (!sp) {
        // Fall back to System theme if provider not set
        m_currentTheme = Theme::System;
        if (applyImmediately) {
            applyTheme(m_currentTheme);
        }
        return;
    }
    
    auto& s = sp->settings();
    QString themeString = s.value(PhxKeys::UI_THEME, "system").toString();
    if (themeString == "light") {
        m_currentTheme = Theme::Light;
    } else if (themeString == "dark") {
        m_currentTheme = Theme::Dark;
    } else {
        m_currentTheme = Theme::System;
    }
    
    if (applyImmediately) {
        applyTheme(m_currentTheme);
    }
}

void ThemeManager::saveSettings()
{
    auto* sp = provider_.data();
    if (!sp) {
        return;
    }
    
    auto& s = sp->settings();
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
    s.setValue(PhxKeys::UI_THEME, themeString);
}

void ThemeManager::applyTheme(Theme theme)
{
    if (theme == Theme::System) {
        applySystemTheme();
        return;
    }

    if (applyResolvedTheme(theme)) {
        // emit handled by caller
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
    if (m_currentTheme != Theme::System) {
        return;
    }

    const Theme resolved = resolveSystemTheme();
    const bool changed = applyResolvedTheme(resolved);
    if (changed) {
        const QString label = (resolved == Theme::Dark)
                                  ? QStringLiteral("dark")
                                  : QStringLiteral("light");
        qInfo() << "[theme] system color scheme changed →" << label;
        emitThemeSignals();
    }
}

QString ThemeManager::loadStyleSheet(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[Theme] Failed to open" << filename << file.errorString();
        return QString();
    }
    
    QTextStream stream(&file);
    QString content = stream.readAll();
    
    if (file.error() != QFile::NoError) {
        qWarning() << "[Theme] Read error for" << filename << file.errorString();
        return QString();
    }
    
    if (content.isEmpty()) {
        qWarning() << "[Theme] Stylesheet file is empty:" << filename;
    }
    
    return content;
}

ThemeManager::Theme ThemeManager::resolveSystemTheme() const
{
    return systemPrefersDark() ? Theme::Dark : Theme::Light;
}

bool ThemeManager::applyResolvedTheme(Theme resolvedTheme)
{
    if (resolvedTheme == Theme::System) {
        return false;
    }

    const bool changed = !m_paletteInitialized || m_activeTheme != resolvedTheme;
    if (!changed) {
        return false;
    }

    m_paletteInitialized = true;

    switch (resolvedTheme) {
        case Theme::Light:
            applyLightTheme();
            break;
        case Theme::Dark:
            applyDarkTheme();
            break;
        case Theme::System:
            break;
    }

    m_activeTheme = resolvedTheme;
    return true;
}

void ThemeManager::emitThemeSignals()
{
    emit themeChanged(m_currentTheme);
    emit darkModeChanged(isDarkMode());
}

bool ThemeManager::eventFilter(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched);

    if (m_currentTheme == Theme::System) {
        if (event->type() == QEvent::ThemeChange ||
            event->type() == QEvent::ApplicationPaletteChange) {
            applySystemTheme();
            emitThemeSignals();
        }
    }
    return QObject::eventFilter(watched, event);
}
