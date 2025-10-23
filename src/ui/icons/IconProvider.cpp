#include "IconProvider.h"
#include "IconBootstrap.h"
#include "IconTint.h"
#include "ThemeColor.h"
#include "LogCategories.h"
#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QPalette>
#include <QScreen>
#include <QGuiApplication>
#include <QObject>
#include <QMetaObject>
#include <QLoggingCategory>

QHash<IconKey, QIcon> IconProvider::s_cache;
QJsonObject IconProvider::s_iconManifest;
bool IconProvider::s_manifestLoaded = false;

QIcon IconProvider::icon(const QString& name, IconStyle style, int size, bool dark, qreal dpr) {
    IconKey key{name, style, size, dark, dpr};
    
    // Trace once per request
    qCInfo(phxIcons).noquote() << "[ICON] req"
                      << "name=" << name
                      << "style=" << int(style)
                      << "px=" << size
                      << "dark=" << dark;
    
    // Check cache first
    if (s_cache.contains(key)) {
        return s_cache.value(key);
    }
    
    // Load manifest if needed
    if (!s_manifestLoaded) {
        loadManifest();
    }
    
    // Try SVG first, then font
    QIcon result = svgIcon(name, size);
    if (result.isNull()) {
        result = fontIcon(name, style, size, dark);
    }
    
    if (result.isNull()) {
        result = fallback();
    }
    
    // Apply theme-aware tinting for better visibility
    if (!result.isNull()) {
        // Get contrast-aware color
        const QPalette pal = QApplication::palette();
        const QColor iconColor = pickIconColor(pal, dark);
        
        // Tint the icon
        QPixmap pm = result.pixmap(size, size);
        pm.setDevicePixelRatio(dpr);
        pm = tintPixmap(pm, iconColor);
        result = QIcon(pm);
        
        qCInfo(phxIcons).noquote() 
            << "[ICON] returning name=" << name
            << "px=" << size
            << "dark=" << dark
            << "color=" << iconColor.name();
    }
    
    // Cache the result
    s_cache.insert(key, result);
    
    return result;
}

QString IconProvider::fontFamily(IconStyle style) {
    switch (style) {
        case IconStyle::SharpSolid: return IconBootstrap::sharpSolidFamily();
        case IconStyle::SharpRegular: return IconBootstrap::sharpRegularFamily();
        case IconStyle::Duotone: return IconBootstrap::duotoneFamily();
        case IconStyle::Brands: return IconBootstrap::brandsFamily();
        default: return IconBootstrap::sharpSolidFamily();
    }
}

bool IconProvider::isDarkMode(const QWidget* widget) {
    if (!widget) {
        widget = QApplication::activeWindow();
    }
    if (!widget) {
        return false;
    }
    
    QPalette palette = widget->palette();
    return palette.window().color().lightness() < 128;
}

void IconProvider::clearCache() {
    s_cache.clear();
}

int IconProvider::cacheSize() {
    return s_cache.size();
}

void IconProvider::onThemeChanged() {
    clearCache();
}

void IconProvider::setupCacheClearing() {
    // Clear cache on screen/DPR changes
    QObject::connect(qApp, &QGuiApplication::primaryScreenChanged, &IconProvider::clearCache);
    
    // Connect to all existing screens
    for (auto* screen : qApp->screens()) {
        QObject::connect(screen, &QScreen::logicalDotsPerInchChanged, &IconProvider::clearCache);
        QObject::connect(screen, &QScreen::geometryChanged, &IconProvider::clearCache);
    }
}

void IconProvider::loadManifest() {
    QFile manifestFile(":/resources/icons/phx-icons.json");
    if (manifestFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(manifestFile.readAll());
        s_iconManifest = doc.object();
        s_manifestLoaded = true;
    } else {
        qCWarning(phxIcons) << "Could not load icon manifest";
    }
}

QIcon IconProvider::svgIcon(const QString& alias, int size) {
    const QString path = QString(":/icons/%1.svg").arg(alias);
    if (!QFile::exists(path)) {
        qCInfo(phxIcons).noquote() << "[ICON] svg" << path << "MISS";
        return QIcon(); // Return null icon, let caller fallback
    }
    
    qCInfo(phxIcons).noquote() << "[ICON] svg" << path << "FOUND";
    QIcon icon(path);
    if (icon.isNull()) {
        return QIcon(); // Return null icon, let caller fallback
    }
    
    if (size > 0) {
        QPixmap pm = icon.pixmap(size, size);
        return QIcon(pm);
    }
    
    return icon;
}

QIcon IconProvider::fontIcon(const QString& name, IconStyle style, int size, bool dark) {
    QString fontFamily;
    switch (style) {
        case IconStyle::SharpSolid: fontFamily = IconBootstrap::sharpSolidFamily(); break;
        case IconStyle::SharpRegular: fontFamily = IconBootstrap::sharpRegularFamily(); break;
        case IconStyle::Duotone: fontFamily = IconBootstrap::duotoneFamily(); break;
        case IconStyle::Brands: fontFamily = IconBootstrap::brandsFamily(); break;
        default: fontFamily = IconBootstrap::sharpSolidFamily(); break;
    }
    if (fontFamily.isEmpty()) {
        return fallback();
    }
    
    // Get Unicode from manifest
    if (s_manifestLoaded && s_iconManifest.contains(name)) {
        QJsonObject iconData = s_iconManifest[name].toObject();
        QString unicode = iconData["unicode"].toString();
        
        if (!unicode.isEmpty()) {
            QFont font(fontFamily, size);
            QPixmap pixmap(size, size);
            pixmap.fill(Qt::transparent);
            
            QPainter painter(&pixmap);
            painter.setFont(font);
            painter.setPen(dark ? Qt::white : Qt::black);
            painter.drawText(pixmap.rect(), Qt::AlignCenter, unicode);
            
            return QIcon(pixmap);
        }
    }
    
    return fallback();
}

QIcon IconProvider::fallback() {
    // Return question mark SVG icon
    return QIcon(":/icons/circle-question.svg");
}