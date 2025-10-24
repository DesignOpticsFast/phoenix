# How to Update the macOS Dock Icon

**Purpose**: Guide for updating the Phoenix Dock icon on macOS  
**Last Updated**: 2025-01-22  
**Status**: ✅ Working (Tested on macOS 26.0.1)

## 🎯 **Quick Update Process**

### **Step 1: Update the Source Image**
Replace the source image with your new design:
```bash
# Replace with your new icon (recommended: 1024x1024 PNG or SVG)
cp your_new_icon.png resources/macos/Phoenix.svg
# OR
cp your_new_icon.svg resources/macos/Phoenix.svg
```

### **Step 2: Regenerate the Dock Icon**
```bash
# Run the icon generation script
./scripts/generate_macos_icon.sh
```

### **Step 3: Rebuild Phoenix**
```bash
# Clean previous build
rm -rf build

# Build with new icon
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos
cmake --build build -j
```

### **Step 4: Test the New Icon**
```bash
# Verify the icon was updated
ls -l build/Phoenix.app/Contents/Resources/Phoenix.icns

# Test in Dock
killall -KILL Dock
open build/Phoenix.app
```

## 📋 **Detailed Process**

### **Source Image Requirements**
- **Format**: SVG (preferred) or PNG
- **Size**: 1024×1024 pixels minimum
- **Quality**: High resolution, sharp edges
- **Style**: Should work well at small sizes (16×16)
- **Location**: `resources/macos/Phoenix.svg`

### **Icon Generation Process**
The `generate_macos_icon.sh` script:
1. **Reads**: `resources/macos/Phoenix.svg` (or `.png`)
2. **Generates**: All required icon sizes:
   - 16×16, 32×32, 128×128, 256×256, 512×512
   - Plus @2x variants for retina displays
3. **Creates**: `resources/macos/Phoenix.icns`
4. **Uses**: macOS built-in tools (`sips` and `iconutil`)

### **CMake Integration**
The `CMakeLists.txt` automatically:
- **Detects**: `.icns` file on macOS builds
- **Embeds**: Icon in app bundle at `Contents/Resources/Phoenix.icns`
- **Configures**: `CFBundleIconFile = "Phoenix"` in Info.plist
- **Fails**: If `.icns` file is missing (with helpful error message)

## 🔧 **Troubleshooting**

### **Icon Not Updating**
```bash
# Check if new .icns was created
ls -l resources/macos/Phoenix.icns

# Check file modification time
stat resources/macos/Phoenix.icns

# Force regeneration
rm resources/macos/Phoenix.icns
./scripts/generate_macos_icon.sh
```

### **Build Fails**
```bash
# Check Qt6 path (adjust for your installation)
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/your/qt6

# Common Qt6 paths:
# /Users/username/Qt/6.9.3/macos
# /opt/homebrew/lib/cmake/Qt6
# /usr/local/lib/cmake/Qt6
```

### **Dock Icon Still Shows Old Icon**
```bash
# Force Dock refresh
killall -KILL Dock

# Check bundle contents
ls -l build/Phoenix.app/Contents/Resources/

# Verify Info.plist
plutil -p build/Phoenix.app/Contents/Info.plist | grep -i CFBundleIconFile
```

### **Icon Looks Blurry**
- **Cause**: Source image too small or low quality
- **Solution**: Use higher resolution source image (1024×1024 or larger)
- **Check**: Ensure source image is crisp and detailed

## 📁 **File Structure**

```
phoenix/
├── resources/macos/
│   ├── Phoenix.svg          # Source image (your design)
│   └── Phoenix.icns         # Generated Dock icon
├── scripts/
│   └── generate_macos_icon.sh  # Icon generation script
├── docs/
│   └── HOW_TO_UPDATE_MACOS_DOCK_ICON.md  # This guide
└── CMakeLists.txt           # Contains APPLE block for icon
```

## 🎨 **Design Guidelines**

### **Icon Design Best Practices**
- **Simple**: Works well at 16×16 pixels
- **High Contrast**: Visible against light and dark backgrounds
- **Distinctive**: Easily recognizable as Phoenix
- **Consistent**: Matches your brand/design system

### **Technical Requirements**
- **Vector Format**: SVG preferred for scalability
- **Square Aspect**: 1:1 ratio (1024×1024 recommended)
- **Clean Edges**: Sharp, well-defined shapes
- **Color**: Should work in both light and dark modes

## ✅ **Verification Checklist**

After updating the icon, verify:
- [ ] Source image updated (`resources/macos/Phoenix.svg`)
- [ ] Script runs without errors
- [ ] New `.icns` file created (check timestamp)
- [ ] Phoenix builds successfully
- [ ] Dock shows new icon
- [ ] Icon is sharp at all sizes
- [ ] Works in both light and dark modes

## 🚀 **Advanced Usage**

### **Automated Updates**
```bash
# Create a script to update icon and rebuild
#!/bin/bash
echo "Updating Phoenix Dock icon..."
./scripts/generate_macos_icon.sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=/Users/mark/Qt/6.9.3/macos
cmake --build build -j
echo "Icon updated! Testing..."
killall -KILL Dock
open build/Phoenix.app
```

### **Multiple Icon Versions**
```bash
# Keep different versions
cp resources/macos/Phoenix.svg resources/macos/Phoenix_backup.svg
cp your_new_icon.svg resources/macos/Phoenix.svg
./scripts/generate_macos_icon.sh
```

## 📚 **Related Documentation**

- [mac_dock_icon.md](mac_dock_icon.md) - Initial setup guide
- [ICON_SYSTEM_REVIEW_README.md](../ICON_SYSTEM_REVIEW_README.md) - Complete icon system
- [CONTROL_REV_5.3.md](sprint4/CONTROL_REV_5.3.md) - Sprint 4 requirements

---

**Status**: ✅ **COMPLETE** - Ready for future icon updates  
**Next**: Use this guide whenever you need to update the Phoenix Dock icon





