# Phoenix Icon System

Phoenix uses Font Awesome Pro icons with a hybrid storage strategy that provides full icon library access while maintaining GitHub-friendly repository size.

## 🎯 Overview

The icon system provides two access methods:
- **Font-based icons**: Access to ALL Font Awesome Pro icons via Unicode
- **SVG-based icons**: High-quality specific icons for custom use cases

## 📁 Directory Structure

```
assets/icons/fontawesome-pro/
├── fonts/                    # Font files (17 variants, ~98MB)
├── svgs/                    # SVG files (on-demand)
├── metadata/               # Lightweight metadata
├── icon-manager.py         # Management tool
├── README.md              # Usage guide
└── QUICK_REFERENCE.md     # Quick reference
```

## 🚀 Usage

### Font-Based Icons (Recommended)

Access any Font Awesome Pro icon via Unicode codes:

```qml
Text {
    text: "\uf0c7"  // Save icon (any Unicode works!)
    font.family: "Font Awesome 6 Pro"
    font.styleName: "Sharp Solid"
    font.pixelSize: 24
    color: "#333"
}
```

```cpp
QFont font("Font Awesome 6 Pro", 24);
font.setStyleName("Sharp Solid");
QString saveIcon = QString::fromUtf8("\uf0c7");
```

### SVG-Based Icons (When Needed)

For high-quality specific icons:

```qml
Image {
    source: "image://phxicon/save?style=SharpSolid&size=24"
    width: 24
    height: 24
}
```

```cpp
QIcon icon(":/icons/save.svg");
QPushButton button;
button.setIcon(icon);
```

## 🎨 Available Font Styles

| Style | Font File | Best For |
|-------|-----------|----------|
| **Sharp Solid** | Font Awesome 6 Sharp-Solid-900.otf | Primary UI icons |
| **Sharp Regular** | Font Awesome 6 Sharp-Regular-400.otf | Secondary icons |
| **Duotone** | Font Awesome 6 Duotone-Solid-900.otf | Data visualization |
| **Brands** | Font Awesome 6 Brands-Regular-400.otf | Social media |
| **Classic Solid** | Font Awesome 6 Pro-Solid-900.otf | Legacy support |

## 🔧 Management

### Icon Manager Tool

```bash
# List available icons
python3 assets/icons/fontawesome-pro/icon-manager.py list

# Add a new icon
python3 assets/icons/fontawesome-pro/icon-manager.py add heart sharp-solid

# Initialize system
python3 assets/icons/fontawesome-pro/icon-manager.py init
```

### Adding New Icons

1. **Font-based** (recommended): Just use the Unicode code
2. **SVG-based**: Run the icon manager to add specific SVGs

## 📊 Storage Strategy

### Why This Approach?

- **Full Access**: Every Font Awesome Pro icon available via fonts
- **GitHub Friendly**: No large files, stays under limits
- **Performance**: Fonts load once, icons render instantly
- **Flexible**: Mix fonts and SVGs as needed
- **Maintainable**: Clear separation of concerns

### Storage Comparison

| Approach | Size | Icons Available | GitHub Friendly |
|----------|------|-----------------|-----------------|
| **Full Font Awesome** | 500MB+ | All | ❌ No |
| **Fonts Only** | 98MB | All | ✅ Yes |
| **Selective SVGs** | 10MB | Used | ✅ Yes |
| **Hybrid (Our Strategy)** | 108MB | All | ✅ Yes |

## 🎯 Common Icons

### Sharp Solid (Recommended)
- Save: `\uf0c7`
- Open: `\uf07c`
- Chart Line: `\uf201`
- Chart Scatter: `\uf201`
- Wave Sine: `\uf201`
- Cube: `\uf1b2`
- Ruler: `\uf545`
- Brain: `\uf5dc`
- Question: `\uf059`

### Duotone
- Chart Line: `\uf201`
- Chart Scatter: `\uf201`

### Brands
- GitHub: `\uf09b`
- Twitter: `\uf099`
- LinkedIn: `\uf08c`

## 🔗 Integration

### C++ Integration

```cpp
#include <QFont>
#include <QString>

// Load Font Awesome Pro font
QFont font("Font Awesome 6 Pro", 24);
font.setStyleName("Sharp Solid");

// Use Unicode for icons
QString saveIcon = QString::fromUtf8("\uf0c7");
QString openIcon = QString::fromUtf8("\uf07c");
```

### QML Integration

```qml
// Font-based icons
Text {
    text: "\uf0c7"  // Save icon
    font.family: "Font Awesome 6 Pro"
    font.styleName: "Sharp Solid"
    font.pixelSize: 24
    color: "#333"
}

// SVG-based icons
Image {
    source: "image://phxicon/save?style=SharpSolid&size=24"
    width: 24
    height: 24
}
```

## 📋 License

Font Awesome Pro is licensed software. See `LICENSE-THIRD-PARTY.md` for details.

## 🎯 Best Practices

- **Use fonts for UI icons** - Fast, scalable, consistent
- **Use SVGs for custom graphics** - When you need specific colors or effects
- **Prefer Sharp Solid** - Modern, clean appearance
- **Use Duotone for charts** - Great for data visualization
- **Brands for social links** - Standard social media icons