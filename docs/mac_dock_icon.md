# macOS Dock Icon Setup

**Purpose**: Configure Phoenix to display a custom icon in the macOS Dock  
**Scope**: Phase 1 - Dock icon only (no Qt6 discovery or environment setup)

## 🎯 **Quick Setup (Mac Users)**

### **Step 1: Generate the Icon**

```bash
# Run the icon generation script
./scripts/generate_macos_icon.sh
```

### **Step 2: Build Phoenix**

```bash
# Build with the generated icon
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j
```

### **Step 3: Verify the Dock Icon**

```bash
# Check Info.plist has correct icon reference
plutil -p build/Phoenix.app/Contents/Info.plist | grep -i CFBundleIconFile
# Expected output: "Phoenix"

# Verify .icns file is in bundle
ls -l build/Phoenix.app/Contents/Resources/Phoenix.icns

# Test the Dock icon
killall -KILL Dock
open build/Phoenix.app
```

## 🔧 **Technical Details**

### **CMake Configuration**

The `CMakeLists.txt` includes:

- **APPLE block**: Configures macOS bundle with Dock icon
- **Icon source**: `resources/macos/Phoenix.icns`
- **Bundle properties**: `MACOSX_BUNDLE_ICON_FILE "Phoenix"`
- **Guard**: Fails if `.icns` file is missing on macOS

### **Icon Generation Script**

`scripts/generate_macos_icon.sh`:

- **Input**: Phoenix.svg or Phoenix.png (1024×1024 recommended)
- **Output**: `resources/macos/Phoenix.icns`
- **Sizes**: 16, 32, 128, 256, 512 (with @2x variants)
- **Tools**: Uses `sips` and `iconutil` (macOS built-ins)

### **Bundle Structure**

```
Phoenix.app/
├── Contents/
│   ├── Info.plist          # CFBundleIconFile = "Phoenix"
│   └── Resources/
│       └── Phoenix.icns    # Dock icon file
└── MacOS/
    └── Phoenix             # Executable
```

## 🐛 **Troubleshooting**

### **Icon Not Showing in Dock**

1. **Check Info.plist**: `plutil -p build/Phoenix.app/Contents/Info.plist | grep -i CFBundleIconFile`
2. **Verify .icns file**: `ls -l build/Phoenix.app/Contents/Resources/Phoenix.icns`
3. **Regenerate icon**: Run `./scripts/generate_macos_icon.sh` again
4. **Restart Dock**: `killall -KILL Dock`

### **Build Fails with Missing Icon**

- **Error**: "Missing Phoenix.icns — run scripts/generate_macos_icon.sh on macOS"
- **Solution**: Run the icon generation script on macOS

### **Icon Looks Blurry**

- **Cause**: Source image too small or low quality
- **Solution**: Use high-resolution source image (1024×1024 or larger)

## 📋 **Requirements**

### **For Mac Users**

- **macOS**: Any recent version
- **Tools**: `sips` and `iconutil` (built-in)
- **Source**: Phoenix.svg or Phoenix.png (1024×1024 recommended)

### **For Linux Builds**

- **Placeholder**: `resources/macos/Phoenix.icns` (empty file)
- **Purpose**: Keeps Linux builds green
- **Note**: Real icon generated on macOS only

## 🎨 **Icon Specifications**

### **Source Image Requirements**

- **Format**: SVG (preferred) or PNG
- **Size**: 1024×1024 pixels minimum
- **Quality**: High resolution, sharp edges
- **Style**: Should work well at small sizes (16×16)

### **Generated Sizes**

- **16×16**: Dock icon (small)
- **32×32**: Dock icon (medium)
- **128×128**: Dock icon (large)
- **256×256**: Dock icon (extra large)
- **512×512**: Dock icon (retina)
- **1024×1024**: Dock icon (high DPI)

## ✅ **Success Criteria**

- [ ] Phoenix.icns generated successfully
- [ ] CMake builds without errors
- [ ] Info.plist contains `CFBundleIconFile = "Phoenix"`
- [ ] Phoenix.icns exists in bundle Resources/
- [ ] Dock shows custom Phoenix icon
- [ ] Icon is sharp and recognizable at all sizes

---

**Status**: ✅ **COMPLETE** - Ready for Mac users to generate and test  
**Next**: Run the script on macOS and verify Dock icon functionality
