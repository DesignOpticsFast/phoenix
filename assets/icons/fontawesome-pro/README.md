# Font Awesome Pro Icons

This directory contains Font Awesome Pro icons using a GitHub-friendly storage strategy.

## 📁 Directory Structure

```
fontawesome-pro/
├── fonts/           # Font files (17 files, ~98MB total)
├── svgs/           # SVG files (on-demand, as needed)
├── metadata/       # Lightweight metadata
├── icon-manager.py # Management tool
└── README.md       # This file
```

## 🚀 Usage

### Font-Based Icons (Recommended)
Access any Font Awesome icon via Unicode:

```qml
Text {
    text: "\uf0c7"  // Save icon
    font.family: "Font Awesome 6 Pro"
    font.styleName: "Sharp Solid"
    font.pixelSize: 24
}
```

### SVG Icons (When Needed)
Add SVGs only when you need them:

```bash
python3 icon-manager.py add heart sharp-solid
```

## 🔧 Management

```bash
# List available icons
python3 icon-manager.py list

# Add a new icon
python3 icon-manager.py add <icon-name> <style>

# Initialize system
python3 icon-manager.py init
```

## 📊 Storage Strategy

- **Fonts**: Full icon library access (~98MB)
- **SVGs**: On-demand, only what we use
- **Metadata**: Lightweight index, not massive JSON
- **GitHub**: Friendly, no large files

## 🎨 Available Font Styles

- **Sharp Solid**: `Font Awesome 6 Sharp-Solid-900.otf`
- **Sharp Regular**: `Font Awesome 6 Sharp-Regular-400.otf`
- **Duotone**: `Font Awesome 6 Duotone-Solid-900.otf`
- **Brands**: `Font Awesome 6 Brands-Regular-400.otf`
- **Classic Solid**: `Font Awesome 6 Pro-Solid-900.otf`

## 🔗 Integration

### C++ Integration
```cpp
QFont font("Font Awesome 6 Pro", 24);
font.setStyleName("Sharp Solid");
QString icon = QString::fromUtf8("\uf0c7");  // Save icon
```

### QML Integration
```qml
// Font-based
Text {
    text: "\uf0c7"
    font.family: "Font Awesome 6 Pro"
    font.styleName: "Sharp Solid"
    font.pixelSize: 24
}

// SVG-based
Image {
    source: "image://phxicon/save?style=SharpSolid&size=24"
    width: 24
    height: 24
}
```

## 📋 License

Font Awesome Pro is licensed software. See `LICENSE-THIRD-PARTY.md` for details.
