#pragma once
#include <QPalette>
#include <QColor>
#include <cmath>

/**
 * Contrast-aware icon color picker for theme-aware icon tinting
 * 
 * This function determines the appropriate icon color based on the background
 * color to ensure proper contrast in both light and dark themes.
 * 
 * @param pal Current application palette
 * @param darkHint Optional hint for dark mode (if ThemeManager available)
 * @return QColor appropriate for the background (black on light, white on dark)
 */
inline QColor pickIconColor(const QPalette& pal, bool darkHint = false) {
    // Use darkHint if available (from ThemeManager)
    if (darkHint) return Qt::white;
    
    const QColor bg = pal.color(QPalette::Window);
    
    // Relative luminance calculation (WCAG standard)
    auto lin = [](double u){ 
        return (u <= 0.03928) ? u/12.92 : std::pow((u + 0.055)/1.055, 2.4); 
    };
    
    const double L = 0.2126*lin(bg.redF()) + 0.7152*lin(bg.greenF()) + 0.0722*lin(bg.blueF());
    
    // Return BLACK on light backgrounds (L > 0.5), WHITE on dark backgrounds (L <= 0.5)
    // Light background (L > 0.5) → need dark icons (black) for contrast
    // Dark background (L <= 0.5) → need light icons (white) for contrast
    return (L > 0.5) ? QColor(0,0,0) : QColor(255,255,255);
}
