#!/bin/bash
# Generate macOS .icns file from Phoenix.svg
# Run this script on macOS to create the proper .icns file

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SVG_FILE="$PROJECT_ROOT/resources/macos/Phoenix.svg"
ICONSET_DIR="$PROJECT_ROOT/resources/macos/Phoenix.iconset"
ICNS_FILE="$PROJECT_ROOT/resources/macos/Phoenix.icns"

echo "Generating macOS .icns file from Phoenix.svg"
echo "Project root: $PROJECT_ROOT"
echo "SVG file: $SVG_FILE"
echo "Iconset dir: $ICONSET_DIR"
echo "ICNS file: $ICNS_FILE"

# Check if SVG exists
if [ ! -f "$SVG_FILE" ]; then
    echo "Error: SVG file not found at $SVG_FILE"
    exit 1
fi

# Create iconset directory
mkdir -p "$ICONSET_DIR"

# Generate all required icon sizes
echo "Generating icon sizes..."

# First convert SVG to a high-res PNG, then use sips to resize
echo "Converting SVG to high-res PNG..."
sips -s format png "$SVG_FILE" --out "$ICONSET_DIR/temp_1024.png" -z 1024 1024

# Now generate all sizes from the high-res PNG
sips -z 16   16   "$ICONSET_DIR/temp_1024.png" --out "$ICONSET_DIR/icon_16x16.png"
sips -z 32   32   "$ICONSET_DIR/temp_1024.png" --out "$ICONSET_DIR/icon_16x16@2x.png"
sips -z 32   32   "$ICONSET_DIR/temp_1024.png" --out "$ICONSET_DIR/icon_32x32.png"
sips -z 64   64   "$ICONSET_DIR/temp_1024.png" --out "$ICONSET_DIR/icon_32x32@2x.png"
sips -z 128  128  "$ICONSET_DIR/temp_1024.png" --out "$ICONSET_DIR/icon_128x128.png"
sips -z 256  256  "$ICONSET_DIR/temp_1024.png" --out "$ICONSET_DIR/icon_128x128@2x.png"
sips -z 256  256  "$ICONSET_DIR/temp_1024.png" --out "$ICONSET_DIR/icon_256x256.png"
sips -z 512  512  "$ICONSET_DIR/temp_1024.png" --out "$ICONSET_DIR/icon_256x256@2x.png"
sips -z 512  512  "$ICONSET_DIR/temp_1024.png" --out "$ICONSET_DIR/icon_512x512.png"
sips -z 1024 1024 "$ICONSET_DIR/temp_1024.png" --out "$ICONSET_DIR/icon_512x512@2x.png"

# Clean up temp file
rm -f "$ICONSET_DIR/temp_1024.png"

echo "Generated icon sizes:"
ls -la "$ICONSET_DIR/"

# Create .icns file
echo "Creating .icns file..."
iconutil -c icns "$ICONSET_DIR" -o "$ICNS_FILE"

# Clean up iconset directory
rm -rf "$ICONSET_DIR"

echo "✅ Successfully created $ICNS_FILE"
echo "File size: $(ls -lh "$ICNS_FILE" | awk '{print $5}')"

# Verify the .icns file
echo "Verifying .icns file..."
file "$ICNS_FILE"
