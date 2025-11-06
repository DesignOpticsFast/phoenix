#pragma once
#include <QIcon>
#include <QHash>
#include <QJsonObject>
#include <QString>
#include <QSize>
#include <QPalette>
#include <QToolBar>
#include <QEvent>

// Include the enum definition
enum class IconStyle { 
    SharpSolid = 0, 
    SharpRegular = 1, 
    Duotone = 2, 
    Brands = 3, 
    ClassicSolid = 4 
};

struct IconKey {
    QString name;
    IconStyle style;
    int size;
    bool dark;
    qreal dpr;
    
    bool operator==(const IconKey& other) const {
        return name == other.name && style == other.style && 
               size == other.size && dark == other.dark && dpr == other.dpr;
    }
    
    bool operator<(const IconKey& other) const {
        if (name != other.name) return name < other.name;
        if (style != other.style) return style < other.style;
        if (size != other.size) return size < other.size;
        if (dark != other.dark) return dark < other.dark;
        return dpr < other.dpr;
    }
};

// Hash function for IconKey - using qHashMulti for better distribution
inline size_t qHash(const IconKey& key, size_t seed = 0) noexcept {
    return qHashMulti(seed, key.name, static_cast<int>(key.style), key.size, key.dark, qRound64(key.dpr * 1000));
}

class IconProvider {
public:
    // Single widget-aware entrypoint (always uses host widget's palette)
    static QIcon icon(const QString& logicalName, const QSize& logicalSize, const QWidget* host);
    static QString fontFamily(IconStyle style);
    static bool isDarkMode(const QWidget* widget = nullptr);
    static void clearCache();
    static int cacheSize();
    static void onThemeChanged(); // Clears cache on theme change
    static void setupCacheClearing(); // Setup automatic cache clearing on theme/DPR/palette changes
    static void scheduleIconCacheClear(); // Schedule debounced cache clear
    static bool isCacheBypassed(); // Check if cache is bypassed (PHX_ICON_NOCACHE env var)

private:
    static QHash<IconKey, QIcon> s_cache;
    static QJsonObject s_iconManifest;
    static QHash<QString, QString> s_aliasMap; // alias -> canonical name
    static bool s_manifestLoaded;
    static QTimer* s_cacheDebounceTimer; // single-shot debounce for cache clear
    
    static void loadManifest();
    static QString resolveAlias(const QString& name); // Resolve alias to canonical name
    static IconStyle parseStyleString(const QString& styleStr); // Parse "sharp-solid" -> IconStyle
    static QIcon svgIcon(const QString& alias, int size, const QPalette& pal, qreal dpr = 1.0, const QWidget* widget = nullptr);
    static QIcon fontIcon(const QString& name, IconStyle style, int size, const QPalette& pal, qreal dpr = 1.0, const QWidget* widget = nullptr);
    static QIcon themeIcon(const QString& name, int size, const QPalette& pal, qreal dpr = 1.0, const QWidget* widget = nullptr); // Try system theme icons
    static QIcon fallback(int size, const QPalette& pal, qreal dpr = 1.0, const QWidget* widget = nullptr); // Fallback with palette for states
    static QPalette getPaletteForIcon(const QWidget* widget); // Get palette from widget or app
    
    // Color helpers for widget-aware tinting
    static QColor normalColorFor(const QWidget* widget);
    static QColor selectedColorFor(const QWidget* widget);
    static QColor disabledColorFor(const QWidget* widget);
};