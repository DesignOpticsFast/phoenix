#!/bin/bash

# Generate macOS Dock Icon Script
# Purpose: Create Phoenix.icns from source image for macOS Dock
# Usage: Run on macOS to generate the Dock icon

set -e

echo "🎨 GENERATING MACOS DOCK ICON"
echo "============================="
echo ""

# Check if we're on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "❌ This script must be run on macOS"
    exit 1
fi

# Check for required tools
if ! command -v sips &> /dev/null; then
    echo "❌ sips command not found (required for image processing)"
    exit 1
fi

if ! command -v iconutil &> /dev/null; then
    echo "❌ iconutil command not found (required for .icns generation)"
    exit 1
fi

# Source image paths (try different locations)
SOURCE_IMAGE=""
for path in "resources/macos/Phoenix.svg" "resources/macos/Phoenix.png" "resources/Phoenix.svg" "resources/Phoenix.png"; do
    if [[ -f "$path" ]]; then
        SOURCE_IMAGE="$path"
        break
    fi
done

if [[ -z "$SOURCE_IMAGE" ]]; then
    echo "❌ No source image found. Looking for:"
    echo "   - resources/macos/Phoenix.svg"
    echo "   - resources/macos/Phoenix.png"
    echo "   - resources/Phoenix.svg"
    echo "   - resources/Phoenix.png"
    exit 1
fi

echo "📸 Source image: $SOURCE_IMAGE"

# Create iconset directory
ICONSET_DIR="resources/macos/Phoenix.iconset"
mkdir -p "$ICONSET_DIR"

echo "🔄 Generating icon sizes..."

# Generate all required icon sizes
# 16x16
sips -z 16 16 "$SOURCE_IMAGE" --out "$ICONSET_DIR/icon_16x16.png" > /dev/null 2>&1

# 32x32
sips -z 32 32 "$SOURCE_IMAGE" --out "$ICONSET_DIR/icon_16x16@2x.png" > /dev/null 2>&1

# 32x32
sips -z 32 32 "$SOURCE_IMAGE" --out "$ICONSET_DIR/icon_32x32.png" > /dev/null 2>&1

# 64x64
sips -z 64 64 "$SOURCE_IMAGE" --out "$ICONSET_DIR/icon_32x32@2x.png" > /dev/null 2>&1

# 128x128
sips -z 128 128 "$SOURCE_IMAGE" --out "$ICONSET_DIR/icon_128x128.png" > /dev/null 2>&1

# 256x256
sips -z 256 256 "$SOURCE_IMAGE" --out "$ICONSET_DIR/icon_128x128@2x.png" > /dev/null 2>&1

# 256x256
sips -z 256 256 "$SOURCE_IMAGE" --out "$ICONSET_DIR/icon_256x256.png" > /dev/null 2>&1

# 512x512
sips -z 512 512 "$SOURCE_IMAGE" --out "$ICONSET_DIR/icon_256x256@2x.png" > /dev/null 2>&1

# 512x512
sips -z 512 512 "$SOURCE_IMAGE" --out "$ICONSET_DIR/icon_512x512.png" > /dev/null 2>&1

# 1024x1024
sips -z 1024 1024 "$SOURCE_IMAGE" --out "$ICONSET_DIR/icon_512x512@2x.png" > /dev/null 2>&1

echo "✅ Icon sizes generated"

# Generate .icns file
echo "🔄 Creating .icns file..."
iconutil -c icns "$ICONSET_DIR" -o "resources/macos/Phoenix.icns"

# Clean up iconset directory
rm -rf "$ICONSET_DIR"

echo "✅ Phoenix.icns created successfully"
echo ""

# Verify the .icns file
if [[ -f "resources/macos/Phoenix.icns" ]]; then
    echo "📋 Verification:"
    echo "   File: resources/macos/Phoenix.icns"
    echo "   Size: $(ls -lh resources/macos/Phoenix.icns | awk '{print $5}')"
    echo ""
    
    echo "🚀 Next steps:"
    echo "   1. Build Phoenix:"
    echo "      cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo"
    echo "      cmake --build build -j"
    echo ""
    echo "   2. Verify bundle:"
    echo "      plutil -p build/Phoenix.app/Contents/Info.plist | grep -i CFBundleIconFile"
    echo "      ls -l build/Phoenix.app/Contents/Resources/Phoenix.icns"
    echo ""
    echo "   3. Test Dock icon:"
    echo "      killall -KILL Dock"
    echo "      open build/Phoenix.app"
    echo ""
    echo "✅ macOS Dock icon generation complete!"
else
    echo "❌ Failed to create Phoenix.icns"
    exit 1
fi