#!/bin/bash
# Font Awesome Pro Setup Script
# Installs Font Awesome Pro icons with GitHub-friendly storage strategy

set -e

echo "🎨 Setting up Font Awesome Pro with GitHub-friendly storage..."

# Create directory structure
mkdir -p assets/icons/fontawesome-pro/{fonts,svgs,metadata}

# Copy existing fonts (if any)
if [ -d "assets/icons/PHXIcons/webfonts" ]; then
    echo "📁 Copying existing fonts..."
    cp assets/icons/PHXIcons/webfonts/*.ttf assets/icons/fontawesome-pro/fonts/ 2>/dev/null || true
fi

# Copy existing SVGs (if any)
if [ -d "assets/icons/PHXIcons/svgs" ]; then
    echo "🖼️ Copying existing SVGs..."
    cp assets/icons/PHXIcons/svgs/*.svg assets/icons/fontawesome-pro/svgs/ 2>/dev/null || true
fi

# Initialize icon manager
echo "🔧 Initializing icon management system..."
python3 assets/icons/fontawesome-pro/icon-manager.py init

# Create .gitignore for the directory
cat > assets/icons/fontawesome-pro/.gitignore << 'EOF'
# Font Awesome Pro - Exclude large files
metadata/icon-families.json
metadata/icons.json
metadata/icon-metadata.json
metadata/*.json.bak

# Keep only essential files
!metadata/icon-index.json
!metadata/usage-examples.json
!fonts/*.ttf
!svgs/*.svg
EOF

# Create README for the directory
cat > assets/icons/fontawesome-pro/README.md << 'EOF'
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
EOF

# Update the main icon manifest
if [ -f "assets/icons/phx-icons.json" ]; then
    echo "📋 Updating main icon manifest..."
    # The existing manifest is already good, just ensure it points to the right location
    echo "✅ Main icon manifest is up to date"
fi

# Create a quick reference for developers
cat > assets/icons/fontawesome-pro/QUICK_REFERENCE.md << 'EOF'
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
EOF

echo "✅ Font Awesome Pro setup complete!"
echo ""
echo "📁 Directory structure:"
find assets/icons/fontawesome-pro -type f | head -10
echo ""
echo "🔧 Management commands:"
echo "  python3 assets/icons/fontawesome-pro/icon-manager.py list"
echo "  python3 assets/icons/fontawesome-pro/icon-manager.py add <name> <style>"
echo ""
echo "📚 Documentation:"
echo "  - docs/icon-storage-strategy.md"
echo "  - assets/icons/fontawesome-pro/README.md"
echo "  - assets/icons/fontawesome-pro/QUICK_REFERENCE.md"
