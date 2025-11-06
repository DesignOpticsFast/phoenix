#include "IconProvider.h"
#include "IconBootstrap.h"
#include "IconTint.h"
#include "ThemeColor.h"
#include "PhxLogging.h"
#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QJsonDocument>
#include <QJsonArray>
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
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <QEvent>
#include <QTimer>
#include <QFontInfo>
#include <QFontMetricsF>
#include <QProcessEnvironment>
#include <cmath>

QHash<IconKey, QIcon> IconProvider::s_cache;
QJsonObject IconProvider::s_iconManifest;
QHash<QString, QString> IconProvider::s_aliasMap;
bool IconProvider::s_manifestLoaded = false;
QTimer* IconProvider::s_cacheDebounceTimer = nullptr;
static bool s_cacheBypassed = false;

// Helper to compute DPR from widget's screen or primary screen
static qreal computeDpr(const QWidget* w = nullptr) {
    if (const QScreen* s = w ? w->screen() : QGuiApplication::primaryScreen()) {
        return std::max<qreal>(1.0, s->devicePixelRatio());
    }
    return 1.0;
}

QIcon IconProvider::icon(const QString& name, IconStyle style, int size, bool dark, qreal dpr) {
    IconKey key{name, style, size, dark, dpr};
    
    // Trace once per request (debug level to reduce noise)
    qCDebug(phxIcons).noquote() << "[ICON] req"
                      << "name=" << name
                      << "style=" << int(style)
                      << "px=" << size
                      << "dark=" << dark;
    
    // Check cache first (unless bypassed)
    if (!isCacheBypassed() && s_cache.contains(key)) {
        return s_cache.value(key);
    }
    
    // Load manifest if needed
    if (!s_manifestLoaded) {
        loadManifest();
    }
    
    // NEW PRIORITY ORDER: glyph → SVG → theme → fallback
    QIcon result;
    QString canonicalName = resolveAlias(name);
    
    // Get palette for consistent color role usage
    const QPalette pal = QApplication::palette();
    // Ensure DPR is at least 1.0
    dpr = qMax(1.0, dpr);
    
    // 1. Try FA glyph (if available and manifest has entry)
    if (IconBootstrap::faAvailable() && s_iconManifest.contains(canonicalName)) {
        QJsonObject iconData = s_iconManifest[canonicalName].toObject();
        QString styleStr = iconData["style"].toString("sharp-solid");
        IconStyle resolvedStyle = parseStyleString(styleStr);
        result = fontIcon(canonicalName, resolvedStyle, size, pal, dpr, nullptr);  // Old overload has no widget
    }
    
    // 2. Try SVG (branding only)
    if (result.isNull()) {
        result = svgIcon(canonicalName, size, pal, dpr, nullptr);  // Old overload has no widget
    }
    
    // 3. Try system theme
    if (result.isNull()) {
        result = themeIcon(canonicalName, size, pal, dpr, nullptr);  // Old overload has no widget
    }
    
    // 4. Fallback
    if (result.isNull()) {
        result = fallback(size, pal, dpr, nullptr);  // Old overload has no widget
    }
    
    // Cache the result (unless bypassed)
    if (!isCacheBypassed()) {
        s_cache.insert(key, result);
    }
    
    return result;
}

QIcon IconProvider::icon(const QString& logicalName, const QSize& size, const QPalette& pal) {
    const int s = std::max(1, qMin(size.width(), size.height()));
    const qreal dpr = computeDpr();
    
    bool isDark = pal.window().color().lightness() < 128;
    return icon(logicalName, IconStyle::SharpSolid, s, isDark, dpr);
}

QIcon IconProvider::icon(const QString& logicalName, const QSize& size, const QWidget* widget) {
    const int s = std::max(1, qMin(size.width(), size.height()));
    const qreal dpr = computeDpr(widget);
    
    QPalette pal = getPaletteForIcon(widget);
    bool isDark = pal.window().color().lightness() < 128;
    return icon(logicalName, IconStyle::SharpSolid, s, isDark, dpr);
}

QPalette IconProvider::getPaletteForIcon(const QWidget* widget) {
    // Try to get palette from widget (especially toolbars) first
    if (widget) {
        // Check if it's a toolbar or has a toolbar parent
        const QToolBar* toolbar = qobject_cast<const QToolBar*>(widget);
        if (!toolbar && widget->parent()) {
            toolbar = qobject_cast<const QToolBar*>(widget->parent());
        }
        if (toolbar) {
            return toolbar->palette();
        }
        return widget->palette();
    }
    // Fall back to application palette
    return QApplication::palette();
}

QColor IconProvider::normalColorFor(const QWidget* w) {
    const QPalette pal = w ? w->palette() : QApplication::palette();
    if (qobject_cast<const QMenu*>(w))      return pal.color(QPalette::Text);
    if (qobject_cast<const QMenuBar*>(w))   return pal.color(QPalette::WindowText);
    if (qobject_cast<const QToolBar*>(w))   return pal.color(QPalette::ButtonText);
    return pal.color(QPalette::WindowText);
}

QColor IconProvider::selectedColorFor(const QWidget* w) {
    const QPalette pal = w ? w->palette() : QApplication::palette();
    if (qobject_cast<const QMenu*>(w))      return pal.color(QPalette::HighlightedText);
    if (qobject_cast<const QMenuBar*>(w))   return pal.color(QPalette::HighlightedText);
    if (qobject_cast<const QToolBar*>(w))   return pal.color(QPalette::ButtonText);
    return pal.color(QPalette::HighlightedText);
}

QColor IconProvider::disabledColorFor(const QWidget* w) {
    const QPalette pal = w ? w->palette() : QApplication::palette();
    return pal.color(QPalette::Disabled, QPalette::Text);
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

bool IconProvider::isCacheBypassed() {
    // Check once and cache the result
    static bool checked = false;
    if (!checked) {
        s_cacheBypassed = QProcessEnvironment::systemEnvironment().contains("PHX_ICON_NOCACHE");
        checked = true;
        if (s_cacheBypassed) {
            qCDebug(phxIcons) << "Icon cache bypassed via PHX_ICON_NOCACHE environment variable";
        }
    }
    return s_cacheBypassed;
}

void IconProvider::scheduleIconCacheClear() {
    if (!s_cacheDebounceTimer) {
        // Initialize timer on first use
        s_cacheDebounceTimer = new QTimer(qApp);
        s_cacheDebounceTimer->setSingleShot(true);
        s_cacheDebounceTimer->setInterval(75);
        QObject::connect(s_cacheDebounceTimer, &QTimer::timeout, &IconProvider::clearCache);
    }
    if (!s_cacheDebounceTimer->isActive()) {
        s_cacheDebounceTimer->start();
    }
}

int IconProvider::cacheSize() {
    return s_cache.size();
}

void IconProvider::onThemeChanged() {
    clearCache();
}

void IconProvider::setupCacheClearing() {
    // Lambda to wire all screen signals for cache invalidation
    auto wireScreen = [](QScreen* s) {
        if (!s) return;
        QObject::connect(s, &QScreen::geometryChanged, &IconProvider::scheduleIconCacheClear);
        QObject::connect(s, &QScreen::logicalDotsPerInchChanged, &IconProvider::scheduleIconCacheClear);
        QObject::connect(s, &QScreen::physicalDotsPerInchChanged, &IconProvider::scheduleIconCacheClear);
        // Note: devicePixelRatioChanged signal may not be available in all Qt versions
        // The above signals should cover DPR changes through geometry/DPI changes
    };
    
    // Wire existing screens
    for (QScreen* screen : QGuiApplication::screens()) {
        wireScreen(screen);
    }
    
    // Handle screen added/removed
    QObject::connect(qApp, &QGuiApplication::screenAdded, [wireScreen](QScreen* s) {
        wireScreen(s);
        IconProvider::scheduleIconCacheClear();
    });
    
    QObject::connect(qApp, &QGuiApplication::screenRemoved, [](QScreen*) {
        IconProvider::scheduleIconCacheClear();
    });
    
    // Clear cache on primary screen change
    QObject::connect(qApp, &QGuiApplication::primaryScreenChanged, &IconProvider::scheduleIconCacheClear);
    
    // Install event filter to catch palette changes (immediate clear, no debounce needed)
    class PaletteChangeFilter : public QObject {
    public:
        bool eventFilter(QObject* obj, QEvent* event) override {
            if (event->type() == QEvent::ApplicationPaletteChange || 
                event->type() == QEvent::PaletteChange) {
                IconProvider::clearCache(); // Immediate clear for palette changes
            }
            return QObject::eventFilter(obj, event);
        }
    };
    
    static PaletteChangeFilter* filter = new PaletteChangeFilter();
    qApp->installEventFilter(filter);
}

void IconProvider::loadManifest() {
    QFile manifestFile(":/resources/icons/phx-icons.json");
    if (!manifestFile.open(QIODevice::ReadOnly)) {
        qCCritical(phxIcons) << "Failed to open icon manifest resource:" << manifestFile.errorString();
        s_iconManifest = QJsonObject();
        s_manifestLoaded = true; // Mark as loaded to avoid repeated attempts
        return;
    }

    QByteArray data = manifestFile.readAll();
    if (data.isEmpty()) {
        qCCritical(phxIcons) << "Icon manifest resource is empty";
        s_iconManifest = QJsonObject();
        s_manifestLoaded = true;
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || doc.isNull()) {
        qCCritical(phxIcons) << "Failed to parse icon manifest JSON:" << parseError.errorString();
        s_iconManifest = QJsonObject();
        s_manifestLoaded = true;
        return;
    }

    QJsonObject root = doc.object();
    s_iconManifest = root["icons"].toObject();
    
    // Build alias map
    s_aliasMap.clear();
    for (auto it = s_iconManifest.begin(); it != s_iconManifest.end(); ++it) {
        QString canonicalName = it.key();
        QJsonObject iconData = it.value().toObject();
        if (iconData.contains("aliases")) {
            QJsonArray aliases = iconData["aliases"].toArray();
            for (const QJsonValue& alias : aliases) {
                s_aliasMap[alias.toString()] = canonicalName;
            }
        }
        // Also map canonical name to itself
        s_aliasMap[canonicalName] = canonicalName;
    }
    
    s_manifestLoaded = true;
    qCDebug(phxIcons) << "Loaded icon manifest with" << s_iconManifest.size() << "icons";
}

QString IconProvider::resolveAlias(const QString& name) {
    if (!s_manifestLoaded) {
        loadManifest();
    }
    return s_aliasMap.value(name, name); // Return canonical name or original if not found
}

IconStyle IconProvider::parseStyleString(const QString& styleStr) {
    if (styleStr == "sharp-solid" || styleStr == "solid") {
        return IconStyle::SharpSolid;
    } else if (styleStr == "sharp-regular" || styleStr == "regular") {
        return IconStyle::SharpRegular;
    } else if (styleStr == "duotone") {
        return IconStyle::Duotone;
    } else if (styleStr == "brands") {
        return IconStyle::Brands;
    } else if (styleStr == "classic-solid") {
        return IconStyle::ClassicSolid;
    }
    return IconStyle::SharpSolid; // Default
}

QIcon IconProvider::svgIcon(const QString& alias, int size, const QPalette& pal, qreal dpr, const QWidget* widget) {
    const QString path = QString(":/icons/%1.svg").arg(alias);
    if (!QFile::exists(path)) {
        qCDebug(phxIcons).noquote() << "[ICON] svg" << path << "MISS";
        return QIcon(); // Return null icon, let caller fallback
    }
    
    qCDebug(phxIcons).noquote() << "[ICON] svg" << path << "FOUND";
    QIcon sourceIcon(path);
    if (sourceIcon.isNull()) {
        return QIcon(); // Return null icon, let caller fallback
    }
    
    if (size > 0) {
        // Compute device-pixel dimensions
        const int rw = qCeil(size * dpr);
        const int rh = qCeil(size * dpr);
        
        // Render at device-pixel size and set DPR
        // tintPixmap() will preserve DPR
        QPixmap normalPm = sourceIcon.pixmap(rw, rh);
        normalPm.setDevicePixelRatio(dpr);
        QColor normalColor = normalColorFor(widget);
        normalPm = tintPixmap(normalPm, normalColor);
        
        QPixmap disabledPm = sourceIcon.pixmap(rw, rh);
        disabledPm.setDevicePixelRatio(dpr);
        QColor disabledColor = disabledColorFor(widget);
        disabledPm = tintPixmap(disabledPm, disabledColor);
        
        QPixmap activePm = sourceIcon.pixmap(rw, rh);
        activePm.setDevicePixelRatio(dpr);
        QColor activeColor = normalColorFor(widget);  // Active same as normal
        activePm = tintPixmap(activePm, activeColor);
        
        QPixmap selectedPm = sourceIcon.pixmap(rw, rh);
        selectedPm.setDevicePixelRatio(dpr);
        QColor selectedColor = selectedColorFor(widget);
        selectedPm = tintPixmap(selectedPm, selectedColor);
        
        QIcon result;
        result.addPixmap(normalPm, QIcon::Normal, QIcon::Off);
        result.addPixmap(disabledPm, QIcon::Disabled, QIcon::Off);
        result.addPixmap(activePm, QIcon::Active, QIcon::Off);
        result.addPixmap(selectedPm, QIcon::Selected, QIcon::Off);
        return result;
    }
    
    return sourceIcon;
}

// Helper to check if a face supports a specific codepoint
static bool faceHasGlyph(const QString& fam, const QString& style, char32_t cp) {
    // 12 pt is irrelevant; we only probe the face
    QFont probe = QFontDatabase::font(fam, style, 12);
    // If style not found, try family-only
    if (probe.family().isEmpty()) {
        probe = QFont(fam);
        if (!style.isEmpty()) {
            probe.setStyleName(style);
        }
    }
    // Ensure we don't merge in a fallback font
    probe.setStyleStrategy(QFont::NoFontMerging);
    // Use QFontMetrics to check if the glyph exists in the font
    QFontMetrics fm(probe);
    QChar ch(cp);
    return fm.inFont(ch);
}

QIcon IconProvider::fontIcon(const QString& name, IconStyle style, int size, const QPalette& pal, qreal dpr, const QWidget* widget) {
    if (!IconBootstrap::faAvailable()) {
        return QIcon(); // Return null, let caller fallback
    }
    
    // Get Face (family + styleName) for the requested style
    IconBootstrap::Face face = IconBootstrap::faceForStyle(static_cast<int>(style));
    if (face.family.isEmpty()) {
        return QIcon(); // Return null, let caller fallback
    }
    
    // Get codepoint from manifest (new format)
    if (s_manifestLoaded && s_iconManifest.contains(name)) {
        QJsonObject iconData = s_iconManifest[name].toObject();
        QString codepointStr = iconData["codepoint"].toString();
        
        if (!codepointStr.isEmpty()) {
            // Convert hex string to Unicode character
            bool ok;
            uint32_t codepoint = codepointStr.toUInt(&ok, 16);
            if (ok) {
                const char32_t cp = static_cast<char32_t>(codepoint);
                
                // Force-select face via QFontDatabase::font (more reliable on macOS)
                QFont f = QFontDatabase::font(face.family, face.style, 12);
                if (f.family().isEmpty()) {
                    qCWarning(phxFonts) << "QFontDatabase::font failed for" << face.family << face.style
                                        << "falling back to QFont constructor";
                    f = QFont(face.family);
                    if (!face.style.isEmpty()) {
                        f.setStyleName(face.style);
                    }
                }
                
                // No font merging; we want a hard failure if the glyph doesn't exist
                f.setStyleStrategy(QFont::NoFontMerging);
                
                // Check glyph availability
                bool has = faceHasGlyph(face.family, face.style, cp);
                
                // Style fallback logic if primary face doesn't support the codepoint
                if (!has) {
                    if (face.family == "Font Awesome 6 Pro") {
                        // Bidirectional fallback: try alt style
                        const QString alt = (face.style == "Solid") ? "Regular" : "Solid";
                        QFont f2 = QFontDatabase::font(face.family, alt, 12);
                        if (!f2.family().isEmpty() && faceHasGlyph(face.family, alt, cp)) {
                            f = f2;
                            f.setStyleStrategy(QFont::NoFontMerging);
                            has = true;
                            qCWarning(phxFonts) << "FA fallback face" << alt << "for cp" << QString::number(codepoint, 16);
                        }
                    } else if (face.family == "Font Awesome 6 Duotone" || face.family == "Font Awesome 6 Brands") {
                        // Try family-only first, then the reported style
                        if (!face.style.isEmpty()) {
                            QFont f2 = QFontDatabase::font(face.family, QString(), 12);
                            if (!f2.family().isEmpty() && faceHasGlyph(face.family, QString(), cp)) {
                                f = f2;
                                f.setStyleStrategy(QFont::NoFontMerging);
                                has = true;
                                qCWarning(phxFonts) << "FA fallback family-only for" << face.family
                                                    << "cp" << QString::number(codepoint, 16);
                            }
                        }
                    }
                }
                
                // If no face supports the codepoint, let the pipeline fall through to SVG/theme/fallback
                if (!has) {
                    qCWarning(phxFonts) << "No FA face supports codepoint" << QString::number(codepoint, 16)
                                        << "for icon" << name << "falling through to SVG/theme/fallback";
                    return QIcon(); // Return null to trigger fallback chain
                }
                
                // Compute device-pixel dimensions for pixmap backing
                const int rw = qCeil(size * dpr);
                const int rh = qCeil(size * dpr);
                
                // Calculate logical dimensions (QPainter uses logical coords on DPR pixmaps)
                const qreal lw = static_cast<qreal>(rw) / dpr;  // logical width
                const qreal lh = static_cast<qreal>(rh) / dpr;  // logical height
                
                // Apply font pixel size in logical units (0.9 padding for visual fit)
                f.setPixelSize(qRound(std::min(lw, lh) * 0.9));
                
                // Prepare glyph string and font metrics
                const QString glyph = QString::fromUcs4(&cp, 1);
                QFontMetricsF fm(f);
                
                // Compute baseline-centered coordinates in logical units
                // Horizontal: center using advance (handles negative left bearings better)
                const qreal adv = fm.horizontalAdvance(glyph);
                const qreal x = (lw - adv) * 0.5;
                
                // Vertical: compute baseline so the visual box (ascent+descent) is centered
                const qreal ascent = fm.ascent();
                const qreal descent = fm.descent();
                const qreal y = (lh + ascent - descent) * 0.5; // baseline position in logical units
                
                // Debug logging (once per icon render)
                qCDebug(phxFonts) << "CENTERED glyph" << name
                                  << "lw/lh" << lw << lh
                                  << "adv" << adv << "asc" << ascent << "des" << descent
                                  << "x" << x << "y(baseline)" << y
                                  << "dpr" << dpr << "rw/rh" << rw << rh;
                
                // Helper lambda to create pixmap for a state
                auto createStatePixmap = [&](const QColor& color, const QString& stateName) -> QPixmap {
                    // Create pixmap at device-pixel size (backing storage)
                    QPixmap pm(rw, rh);
                    pm.setDevicePixelRatio(dpr); // Tell Qt this represents lw×lh logical pixels
                    pm.fill(Qt::transparent);
                    
                    QPainter p(&pm);
                    // QPainter now operates in logical coordinates (0..lw, 0..lh)
                    p.setRenderHint(QPainter::TextAntialiasing, true);
                    p.setRenderHint(QPainter::Antialiasing, true);
                    p.setFont(f);
                    p.setPen(color);
                    
                    // Optional: debug outline (red rectangle in logical coordinates)
                    if (qEnvironmentVariableIsSet("PHX_ICON_DEBUG_OUTLINE")) {
                        p.setPen(QColor(255, 0, 0, 80));
                        p.drawRect(QRectF(0, 0, lw - 1, lh - 1));
                        p.setPen(color);
                    }
                    
                    // Draw glyph at explicit baseline point in logical coordinates
                    // This centers the visual box (ascent+descent) rather than just the baseline
                    p.drawText(QPointF(x, y), glyph);
                    p.end();
                    
                    return pm;
                };
                
                // Build icon with all states including Selected mode
                QIcon icon;
                icon.addPixmap(createStatePixmap(normalColorFor(widget), "Normal"), QIcon::Normal, QIcon::Off);
                icon.addPixmap(createStatePixmap(disabledColorFor(widget), "Disabled"), QIcon::Disabled, QIcon::Off);
                icon.addPixmap(createStatePixmap(normalColorFor(widget), "Active"), QIcon::Active, QIcon::Off);
                icon.addPixmap(createStatePixmap(selectedColorFor(widget), "Selected"), QIcon::Selected, QIcon::Off);
                
                return icon;
            }
        }
    }
    
    return QIcon(); // Return null, let caller fallback
}

QIcon IconProvider::themeIcon(const QString& name, int size, const QPalette& pal, qreal dpr, const QWidget* widget) {
    // Map common icon names to system theme names
    static QHash<QString, QString> themeMap = {
        {"file-new", "document-new"},
        {"file-open", "document-open"},
        {"save", "document-save"},
        {"save-as", "document-save-as"},
        {"settings", "preferences-system"},
        {"close", "window-close"},
        {"search", "edit-find"},
        {"info", "help-about"},
        {"help", "help-contents"}
    };
    
    QString themeName = themeMap.value(name, name);
    if (QIcon::hasThemeIcon(themeName)) {
        QIcon icon = QIcon::fromTheme(themeName);
        
        // Compute device-pixel dimensions
        const int rw = qCeil(size * dpr);
        const int rh = qCeil(size * dpr);
        
        // Generate states for theme icons too (they may not have disabled variants)
        // Request at device-pixel size and set DPR
        QPixmap normalPm = icon.pixmap(rw, rh, QIcon::Normal, QIcon::On);
        if (!normalPm.isNull()) {
            normalPm.setDevicePixelRatio(dpr);
            QColor normalColor = normalColorFor(widget);
            QColor disabledColor = disabledColorFor(widget);
            QColor activeColor = normalColorFor(widget);  // Active same as normal
            QColor selectedColor = selectedColorFor(widget);
            
            QIcon result;
            result.addPixmap(tintPixmap(normalPm, normalColor), QIcon::Normal, QIcon::Off);
            result.addPixmap(tintPixmap(normalPm, disabledColor), QIcon::Disabled, QIcon::Off);
            result.addPixmap(tintPixmap(normalPm, activeColor), QIcon::Active, QIcon::Off);
            result.addPixmap(tintPixmap(normalPm, selectedColor), QIcon::Selected, QIcon::Off);
            return result;
        }
        return icon;
    }
    return QIcon(); // Return null, let caller fallback
}

QIcon IconProvider::fallback(int size, const QPalette& pal, qreal dpr, const QWidget* widget) {
    // Return fallback SVG icon with states
    static constexpr const char* kFallback = ":/icons/fallback.svg";
    
    // Compute device-pixel dimensions
    const int rw = qCeil(size * dpr);
    const int rh = qCeil(size * dpr);
    
    if (!QFile::exists(kFallback)) {
        qCCritical(phxIcons) << "Fallback icon missing:" << kFallback;
        // Return a minimal themed icon as last resort (at device-pixel size)
        QPixmap pm(rw, rh);
        pm.setDevicePixelRatio(dpr);
        pm.fill(normalColorFor(widget));
        QIcon icon;
        icon.addPixmap(pm, QIcon::Normal, QIcon::Off);
        icon.addPixmap(pm, QIcon::Disabled, QIcon::Off);
        icon.addPixmap(pm, QIcon::Active, QIcon::Off);
        icon.addPixmap(pm, QIcon::Selected, QIcon::Off);
        return icon;
    }
    
    QIcon sourceIcon(kFallback);
    if (sourceIcon.isNull()) {
        return QIcon();
    }
    
    // Generate states for fallback icon at device-pixel size
    QPixmap normalPm = sourceIcon.pixmap(rw, rh);
    if (!normalPm.isNull()) {
        normalPm.setDevicePixelRatio(dpr);
        QColor normalColor = normalColorFor(widget);
        QColor disabledColor = disabledColorFor(widget);
        QColor activeColor = normalColorFor(widget);  // Active same as normal
        QColor selectedColor = selectedColorFor(widget);
        
        QIcon result;
        result.addPixmap(tintPixmap(normalPm, normalColor), QIcon::Normal, QIcon::Off);
        result.addPixmap(tintPixmap(normalPm, disabledColor), QIcon::Disabled, QIcon::Off);
        result.addPixmap(tintPixmap(normalPm, activeColor), QIcon::Active, QIcon::Off);
        result.addPixmap(tintPixmap(normalPm, selectedColor), QIcon::Selected, QIcon::Off);
        return result;
    }
    
    return sourceIcon;
}