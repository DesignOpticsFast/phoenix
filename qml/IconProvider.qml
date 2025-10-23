pragma Singleton
import QtQuick 2.15

QtObject {
    function icon(name, style, size, dark) {
        return `image://phxicon/${name}?style=${style}&size=${size}&dark=${dark ? 1 : 0}`;
    }
    
    function themedIcon(name, style, size) {
        return icon(name, style, size, isDarkMode());
    }
    
    function isDarkMode() {
        return Qt.application.palette.window.lightness < Qt.application.palette.windowText.lightness;
    }
    
    function fontFamily(style) {
        // This would need to be connected to C++ backend
        switch (style) {
            case "SharpSolid": return "Font Awesome 6 Pro";
            case "SharpRegular": return "Font Awesome 6 Pro";
            case "Duotone": return "Font Awesome 6 Duotone";
            case "Brands": return "Font Awesome 6 Brands";
            case "ClassicSolid": return "Font Awesome 6 Pro";
            default: return "Font Awesome 6 Pro";
        }
    }
}