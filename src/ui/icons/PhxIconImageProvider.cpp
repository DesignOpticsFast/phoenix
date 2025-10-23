#include "PhxIconImageProvider.h"
#include "IconProvider.h"
#include "IconTint.h"
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QApplication>
#include <QPalette>

PhxIconImageProvider::PhxIconImageProvider() 
    : QQuickImageProvider(QQuickImageProvider::Pixmap) {
}

QPixmap PhxIconImageProvider::requestPixmap(const QString& id, QSize* requestedSize, const QSize& defaultSize) {
    QString name;
    IconStyle style = IconStyle::SharpSolid;
    int size = 24;
    bool dark = false;
    
    parseIconId(id, name, style, size, dark);
    
    // Get device pixel ratio for HiDPI
    qreal dpr = 1.0;
    if (QGuiApplication::instance()) {
        QScreen* screen = QGuiApplication::primaryScreen();
        if (screen) {
            dpr = screen->devicePixelRatio();
        }
    }
    
    // Request icon with proper DPR
    QIcon icon = IconProvider::icon(name, style, size, dark, dpr);
    
    if (icon.isNull()) {
        // Return empty pixmap if icon not found
        return QPixmap();
    }
    
    // Create pixmap with proper size and DPR
    int renderSize = qRound(size * dpr);
    QPixmap pixmap = icon.pixmap(renderSize, renderSize);
    pixmap.setDevicePixelRatio(dpr);
    
    if (requestedSize) {
        *requestedSize = QSize(size, size);
    }
    
    return pixmap;
}

void PhxIconImageProvider::parseIconId(const QString& id, QString& name, IconStyle& style, int& size, bool& dark) {
    // Parse URL like: "save?style=SharpSolid&size=24&dark=0"
    QStringList parts = id.split('?');
    name = parts.first();
    
    if (parts.size() > 1) {
        QStringList params = parts[1].split('&');
        for (const QString& param : params) {
            QStringList keyValue = param.split('=');
            if (keyValue.size() == 2) {
                QString key = keyValue[0];
                QString value = keyValue[1];
                
                if (key == "style") {
                    if (value == "SharpSolid") style = IconStyle::SharpSolid;
                    else if (value == "SharpRegular") style = IconStyle::SharpRegular;
                    else if (value == "Duotone") style = IconStyle::Duotone;
                    else if (value == "Brands") style = IconStyle::Brands;
                    else if (value == "ClassicSolid") style = IconStyle::ClassicSolid;
                } else if (key == "size") {
                    size = value.toInt();
                } else if (key == "dark") {
                    dark = (value == "1" || value == "true");
                }
            }
        }
    }
}