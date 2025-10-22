#pragma once
#include <QPixmap>
#include <QColor>
#include <QImage>
#include <QPainter>

/**
 * Universal icon tinting helper for theme-aware icon coloring
 * 
 * This function tints a pixmap to the specified color while preserving
 * the alpha channel, making icons theme-aware for dark/light mode visibility.
 * 
 * @param src Source pixmap to tint
 * @param color Target color for tinting
 * @return Tinted pixmap with preserved alpha
 */
inline QPixmap tintPixmap(const QPixmap& src, const QColor& color) {
    if (src.isNull()) return src;
    
    QImage img = src.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QPainter p(&img);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn); // keep alpha, apply color
    p.fillRect(img.rect(), color);
    p.end();
    return QPixmap::fromImage(img);
}
