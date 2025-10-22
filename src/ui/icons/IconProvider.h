#pragma once
#include <QIcon>
#include <QHash>
#include <QJsonObject>
#include <QString>

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
    static QIcon icon(const QString& name, IconStyle style = IconStyle::SharpSolid, 
                     int size = 16, bool dark = false, qreal dpr = 1.0);
    static QString fontFamily(IconStyle style);
    static bool isDarkMode(const QWidget* widget = nullptr);
    static void clearCache();
    static int cacheSize();
    static void onThemeChanged(); // Clears cache on theme change

private:
    static QHash<IconKey, QIcon> s_cache;
    static QJsonObject s_iconManifest;
    static bool s_manifestLoaded;
    
    static void loadManifest();
    static QIcon svgIcon(const QString& alias, int size);
    static QIcon fontIcon(const QString& name, IconStyle style, int size, bool dark);
    static QIcon fallback();
};