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
        result = themeIcon(canonicalName, pal);
    }
    
    // 4. Fallback
    if (result.isNull()) {
        result = fallback(pal);
    }
    
    // Cache the result
    s_cache.insert(key, result);
    
    return result;
}

QIcon IconProvider::icon(const QString& logicalName, const QSize& size, const QPalette& pal) {
    int iconSize = qMin(size.width(), size.height());
    if (iconSize <= 0) iconSize = 16;
    
    bool isDark = pal.window().color().lightness() < 128;
    qreal dpr = 1.0;
    if (QGuiApplication::instance()) {
        QScreen* screen = QGuiApplication::primaryScreen();
        if (screen) {
            dpr = qMax(1.0, screen->devicePixelRatio());
        }
    }
    
    return icon(logicalName, IconStyle::SharpSolid, iconSize, isDark, dpr);
}

QIcon IconProvider::icon(const QString& logicalName, const QSize& size, const QWidget* widget) {
    QPalette pal = getPaletteForIcon(widget);
    return icon(logicalName, size, pal);
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
        // Render at HiDPI size for sharp display (use ceil to avoid fractional blurs)
        int renderSize = static_cast<int>(std::ceil(size * dpr));
        
        // Normal state
        QPixmap normalPm = sourceIcon.pixmap(renderSize, renderSize);
        normalPm.setDevicePixelRatio(dpr);
        QColor normalColor = pal.color(QPalette::ButtonText);
        normalPm = tintPixmap(normalPm, normalColor);
        
        // Disabled state
        QPixmap disabledPm = sourceIcon.pixmap(renderSize, renderSize);
        disabledPm.setDevicePixelRatio(dpr);
        QColor disabledColor = pal.color(QPalette::Disabled, QPalette::ButtonText);
        disabledPm = tintPixmap(disabledPm, disabledColor);
        
        // Active state (slightly brighter for hover)
        QPixmap activePm = sourceIcon.pixmap(renderSize, renderSize);
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
    
    QString fontFamily;
    switch (style) {
        case IconStyle::SharpSolid: fontFamily = IconBootstrap::sharpSolidFamily(); break;
        case IconStyle::SharpRegular: fontFamily = IconBootstrap::sharpRegularFamily(); break;
        case IconStyle::Duotone: fontFamily = IconBootstrap::duotoneFamily(); break;
        case IconStyle::Brands: fontFamily = IconBootstrap::brandsFamily(); break;
        default: fontFamily = IconBootstrap::sharpSolidFamily(); break;
    }
    if (fontFamily.isEmpty()) {
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
                QChar unicodeChar(codepoint);
                
                // Render at HiDPI size for sharp display (use ceil to avoid fractional blurs)
                int renderSize = static_cast<int>(std::ceil(size * dpr));
                
                // Normal state: use ButtonText role for UI chrome consistency
                QColor normalColor = pal.color(QPalette::ButtonText);
                QPixmap normalPixmap(renderSize, renderSize);
                normalPixmap.fill(Qt::transparent);
                normalPixmap.setDevicePixelRatio(dpr);
                
                QPainter painter(&normalPixmap);
                painter.setFont(QFont(fontFamily, size));
                painter.setPen(normalColor);
                painter.drawText(QRect(0, 0, renderSize, renderSize), Qt::AlignCenter, unicodeChar);
                painter.end();
                
                // Disabled state: use Disabled group with lighter/washed appearance
                QColor disabledColor = pal.color(QPalette::Disabled, QPalette::ButtonText);
                QPixmap disabledPixmap(renderSize, renderSize);
                disabledPixmap.fill(Qt::transparent);
                disabledPixmap.setDevicePixelRatio(dpr);
                
                QPainter disabledPainter(&disabledPixmap);
                disabledPainter.setFont(QFont(fontFamily, size));
                disabledPainter.setPen(disabledColor);
                disabledPainter.drawText(QRect(0, 0, renderSize, renderSize), Qt::AlignCenter, unicodeChar);
                disabledPainter.end();
                
                // Active state: slightly brighter for hover feedback
                QColor activeColor = pal.color(QPalette::Active, QPalette::ButtonText);
                QPixmap activePixmap(renderSize, renderSize);
                activePixmap.fill(Qt::transparent);
                activePixmap.setDevicePixelRatio(dpr);
                
                QPainter activePainter(&activePixmap);
                activePainter.setFont(QFont(fontFamily, size));
                activePainter.setPen(activeColor);
                activePainter.drawText(QRect(0, 0, renderSize, renderSize), Qt::AlignCenter, unicodeChar);
                activePainter.end();
                
                // Build icon with all states
                QIcon icon;
                icon.addPixmap(normalPixmap, QIcon::Normal);
                icon.addPixmap(disabledPixmap, QIcon::Disabled);
                icon.addPixmap(activePixmap, QIcon::Active);
                
                return icon;
            }
        }
    }
    
    return QIcon(); // Return null, let caller fallback
}

QIcon IconProvider::themeIcon(const QString& name, const QPalette& pal) {
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
        
        // Generate states for theme icons too (they may not have disabled variants)
        // Extract pixmaps and tint them
        QPixmap normalPm = icon.pixmap(16, 16);
        if (!normalPm.isNull()) {
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

QIcon IconProvider::fallback(const QPalette& pal) {
    // Return fallback SVG icon with states
    static constexpr const char* kFallback = ":/icons/fallback.svg";
    if (!QFile::exists(kFallback)) {
        qCCritical(phxIcons) << "Fallback icon missing:" << kFallback;
        // Return a minimal themed icon as last resort
        QPixmap pm(16, 16);
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
    
    // Generate states for fallback icon
    QPixmap normalPm = sourceIcon.pixmap(16, 16);
    if (!normalPm.isNull()) {
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