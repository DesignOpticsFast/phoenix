#include "IconBootstrap.h"
#include "IconProvider.h"  // For IconStyle enum
#include <QFontDatabase>
#include <QDebug>

QString IconBootstrap::sSharpSolid;
QString IconBootstrap::sSharpRegular;
QString IconBootstrap::sDuotone;
QString IconBootstrap::sBrands;
QString IconBootstrap::sClassicSolid;

void IconBootstrap::initializeFonts() {
    
    sSharpSolid = loadFont(":/assets/icons/fontawesome-pro/fonts/fa-sharp-solid-900.ttf");
    sSharpRegular = loadFont(":/assets/icons/fontawesome-pro/fonts/fa-sharp-regular-400.ttf");
    sDuotone = loadFont(":/assets/icons/fontawesome-pro/fonts/fa-duotone-900.ttf");
    sBrands = loadFont(":/assets/icons/fontawesome-pro/fonts/fa-brands-400.ttf");
    sClassicSolid = loadFont(":/assets/icons/fontawesome-pro/fonts/fa-solid-900.ttf");
    
}

bool IconBootstrap::fontsLoaded() {
    return !sSharpSolid.isEmpty() && !sSharpRegular.isEmpty() && 
           !sDuotone.isEmpty() && !sBrands.isEmpty() && !sClassicSolid.isEmpty();
}

QStringList IconBootstrap::loadedFontFamilies() {
    QStringList families;
    if (!sSharpSolid.isEmpty()) families << sSharpSolid;
    if (!sSharpRegular.isEmpty()) families << sSharpRegular;
    if (!sDuotone.isEmpty()) families << sDuotone;
    if (!sBrands.isEmpty()) families << sBrands;
    if (!sClassicSolid.isEmpty()) families << sClassicSolid;
    return families;
}

QString IconBootstrap::getFontFamily(IconStyle style) {
    switch (style) {
        case IconStyle::SharpSolid: return sSharpSolid;
        case IconStyle::SharpRegular: return sSharpRegular;
        case IconStyle::Duotone: return sDuotone;
        case IconStyle::Brands: return sBrands;
        case IconStyle::ClassicSolid: return sClassicSolid;
        default: return sSharpSolid;
    }
}

QString IconBootstrap::loadFont(const QString& resourcePath) {
    int fontId = QFontDatabase::addApplicationFont(resourcePath);
    if (fontId == -1) {
        qWarning() << "Failed to load font:" << resourcePath;
        return QString();
    }
    
    QStringList families = QFontDatabase::applicationFontFamilies(fontId);
    if (families.isEmpty()) {
        qWarning() << "No font families found for:" << resourcePath;
        return QString();
    }
    
    return families.first();
}