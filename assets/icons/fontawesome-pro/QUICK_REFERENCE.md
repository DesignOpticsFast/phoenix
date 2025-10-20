# Font Awesome Pro Quick Reference

## 🎯 Common Icons (Unicode Codes)

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

## 🎨 Usage Examples

### QML Font Usage
```qml
Text {
    text: "\uf0c7"  // Save icon
    font.family: "Font Awesome 6 Pro"
    font.styleName: "Sharp Solid"
    font.pixelSize: 24
    color: "#333"
}
```

### C++ Font Usage
```cpp
QFont font("Font Awesome 6 Pro", 24);
font.setStyleName("Sharp Solid");
QString icon = QString::fromUtf8("\uf0c7");
```

### QML SVG Usage
```qml
Image {
    source: "image://phxicon/save?style=SharpSolid&size=24"
    width: 24
    height: 24
}
```

## 🔧 Adding New Icons

1. **Font-based** (recommended): Just use the Unicode code
2. **SVG-based**: Run `python3 icon-manager.py add <name> <style>`
