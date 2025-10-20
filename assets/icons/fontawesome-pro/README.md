# Font Awesome Pro Icons

This directory contains Font Awesome Pro icons using a GitHub-friendly storage strategy.

## 📁 Directory Structure

```
fontawesome-pro/
├── fonts/           # Font files (5 files, ~2MB)
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

- **Fonts**: Full icon library access (~2MB)
- **SVGs**: On-demand, only what we use
- **Metadata**: Lightweight index, not massive JSON
- **GitHub**: Friendly, no large files
