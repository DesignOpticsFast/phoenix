#include "IconProvider.h"
#include "IconBootstrap.h"
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

QCache<IconKey, QIcon> IconProvider::s_cache(512);
QJsonObject IconProvider::s_iconManifest;
bool IconProvider::s_manifestLoaded = false;

QIcon IconProvider::icon(const QString& name, IconStyle style, int size, bool dark, qreal dpr) {
    IconKey key{name, style, size, dark, dpr};
    
    // Check cache first
    if (s_cache.contains(key)) {
        return *s_cache.object(key);
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
    
    // Cache the result
    s_cache.insert(key, new QIcon(result));
    
    return result;
}

QString IconProvider::fontFamily(IconStyle style) {
    return IconBootstrap::getFontFamily(style);
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

void IconProvider::loadManifest() {
    QFile manifestFile(":/icons/phx-icons.json");
    if (manifestFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(manifestFile.readAll());
        s_iconManifest = doc.object();
        s_manifestLoaded = true;
    } else {
        qWarning() << "Could not load icon manifest";
    }
}

QIcon IconProvider::svgIcon(const QString& alias, int size) {
    const QString path = QString(":/icons/%1").arg(alias);
    if (!QFile::exists(path)) {
        return fallback();
    }
    
    QIcon icon(path);
    if (icon.isNull()) {
        return fallback();
    }
    
    if (size > 0) {
        QPixmap pm = icon.pixmap(size, size);
        return QIcon(pm);
    }
    
    return icon;
}

QIcon IconProvider::fontIcon(const QString& name, IconStyle style, int size, bool dark) {
    QString fontFamily = IconBootstrap::getFontFamily(style);
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
    // Return a simple fallback icon
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setPen(Qt::red);
    painter.drawEllipse(2, 2, 12, 12);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "?");
    
    return QIcon(pixmap);
}