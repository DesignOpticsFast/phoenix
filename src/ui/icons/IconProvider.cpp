#include "IconProvider.h"
#include "IconBootstrap.h"
#include "IconTint.h"
#include "ThemeColor.h"
#include "PhxLogging.h"
#include <QApplication>
#include <QFont>
#include <QFontDatabase>
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
#include <QEvent>
#include <QTimer>
#include <cmath>

QHash<IconKey, QIcon> IconProvider::s_cache;
QJsonObject IconProvider::s_iconManifest;
QHash<QString, QString> IconProvider::s_aliasMap;
bool IconProvider::s_manifestLoaded = false;
QTimer* IconProvider::s_cacheDebounceTimer = nullptr;

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
    
    // Check cache first
    if (s_cache.contains(key)) {
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
        result = fontIcon(canonicalName, resolvedStyle, size, pal, dpr);
    }
    
    // 2. Try SVG (branding only)
    if (result.isNull()) {
        result = svgIcon(canonicalName, size, pal, dpr);
    }
    
    // 3. Try system theme
    if (result.isNull()) {
        result = themeIcon(canonicalName, size, pal, dpr);
    }
    
    // 4. Fallback
    if (result.isNull()) {
        result = fallback(size, pal, dpr);
    }
    
    // Cache the result
    s_cache.insert(key, result);
    
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

QIcon IconProvider::svgIcon(const QString& alias, int size, const QPalette& pal, qreal dpr) {
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
        QColor normalColor = pal.color(QPalette::ButtonText);
        normalPm = tintPixmap(normalPm, normalColor);
        
        QPixmap disabledPm = sourceIcon.pixmap(rw, rh);
        disabledPm.setDevicePixelRatio(dpr);
        QColor disabledColor = pal.color(QPalette::Disabled, QPalette::ButtonText);
        disabledPm = tintPixmap(disabledPm, disabledColor);
        
        QPixmap activePm = sourceIcon.pixmap(rw, rh);
        activePm.setDevicePixelRatio(dpr);
        QColor activeColor = pal.color(QPalette::Active, QPalette::ButtonText);
        activePm = tintPixmap(activePm, activeColor);
        
        QIcon result;
        result.addPixmap(normalPm, QIcon::Normal);
        result.addPixmap(disabledPm, QIcon::Disabled);
        result.addPixmap(activePm, QIcon::Active);
        return result;
    }
    
    return sourceIcon;
}

QIcon IconProvider::fontIcon(const QString& name, IconStyle style, int size, const QPalette& pal, qreal dpr) {
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
                // Compute device-pixel dimensions
                const int rw = qCeil(size * dpr);
                const int rh = qCeil(size * dpr);
                
                // Create font with family and styleName (for macOS compatibility)
                QFont f(face.family);
                if (!face.style.isEmpty()) {
                    f.setStyleName(face.style);
                } else {
                    // Weight fallbacks only if styleName not set
                    if (style == IconStyle::SharpSolid || style == IconStyle::ClassicSolid) {
                        f.setWeight(QFont::Black);
                    } else if (style == IconStyle::SharpRegular) {
                        f.setWeight(QFont::Normal);
                    }
                }
                f.setPixelSize(qRound(qMin(rw, rh) * 0.9));
                
                // Debug logging (once per icon render)
                qCDebug(phxFonts) << "glyph" << name
                                  << "fam" << f.family()
                                  << "style" << f.styleName()
                                  << "px" << f.pixelSize()
                                  << "dpr" << dpr
                                  << "rect" << rw << "x" << rh;
                
                // Helper lambda to create pixmap for a state
                auto createStatePixmap = [&](const QColor& color) -> QPixmap {
                    QPixmap pm(rw, rh);
                    pm.setDevicePixelRatio(dpr);
                    pm.fill(Qt::transparent);
                    
                    QPainter p(&pm);
                    p.setRenderHint(QPainter::TextAntialiasing, true);
                    p.setRenderHint(QPainter::Antialiasing, true);
                    p.setFont(f);
                    p.setPen(color);
                    
                    // Use QString::fromUcs4 for robust Unicode handling
                    const char32_t cp = static_cast<char32_t>(codepoint);
                    p.drawText(QRect(0, 0, rw, rh), Qt::AlignCenter, QString::fromUcs4(&cp, 1));
                    p.end();
                    
                    return pm;
                };
                
                // Build icon with all states
                QIcon icon;
                icon.addPixmap(createStatePixmap(pal.color(QPalette::ButtonText)), QIcon::Normal);
                icon.addPixmap(createStatePixmap(pal.color(QPalette::Disabled, QPalette::ButtonText)), QIcon::Disabled);
                icon.addPixmap(createStatePixmap(pal.color(QPalette::Active, QPalette::ButtonText)), QIcon::Active);
                
                return icon;
            }
        }
    }
    
    return QIcon(); // Return null, let caller fallback
}

QIcon IconProvider::themeIcon(const QString& name, int size, const QPalette& pal, qreal dpr) {
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
            QColor normalColor = pal.color(QPalette::ButtonText);
            QColor disabledColor = pal.color(QPalette::Disabled, QPalette::ButtonText);
            QColor activeColor = pal.color(QPalette::Active, QPalette::ButtonText);
            
            QIcon result;
            result.addPixmap(tintPixmap(normalPm, normalColor), QIcon::Normal);
            result.addPixmap(tintPixmap(normalPm, disabledColor), QIcon::Disabled);
            result.addPixmap(tintPixmap(normalPm, activeColor), QIcon::Active);
            return result;
        }
        return icon;
    }
    return QIcon(); // Return null, let caller fallback
}

QIcon IconProvider::fallback(int size, const QPalette& pal, qreal dpr) {
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
        pm.fill(pal.color(QPalette::ButtonText));
        QIcon icon;
        icon.addPixmap(pm, QIcon::Normal);
        icon.addPixmap(pm, QIcon::Disabled);
        icon.addPixmap(pm, QIcon::Active);
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
        QColor normalColor = pal.color(QPalette::ButtonText);
        QColor disabledColor = pal.color(QPalette::Disabled, QPalette::ButtonText);
        QColor activeColor = pal.color(QPalette::Active, QPalette::ButtonText);
        
        QIcon result;
        result.addPixmap(tintPixmap(normalPm, normalColor), QIcon::Normal);
        result.addPixmap(tintPixmap(normalPm, disabledColor), QIcon::Disabled);
        result.addPixmap(tintPixmap(normalPm, activeColor), QIcon::Active);
        return result;
    }
    
    return sourceIcon;
}