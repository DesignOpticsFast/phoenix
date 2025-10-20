#pragma once
#include <QStringList>

enum class IconStyle; // Forward declaration

class IconBootstrap {
public:
    static void initializeFonts();
    static bool fontsLoaded();
    static QStringList loadedFontFamilies();
    static QString getFontFamily(IconStyle style);

private:
    static QString sSharpSolid, sSharpRegular, sDuotone, sBrands, sClassicSolid;
    static QString loadFont(const QString& resourcePath);
};