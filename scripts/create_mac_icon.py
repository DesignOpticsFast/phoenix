#!/usr/bin/env python3
"""
Create macOS ICNS icon from SVG
"""

import subprocess
import os
from pathlib import Path

def create_mac_icon():
    """Create macOS ICNS icon from SVG"""
    project_root = Path(__file__).parent.parent
    svg_path = project_root / "src/resources/icons/phoenix-icon.svg"
    icns_path = project_root / "src/resources/icons/phoenix-icon.icns"
    
    print("🍎 Creating macOS ICNS Icon")
    print("=" * 40)
    
    if not svg_path.exists():
        print(f"❌ SVG icon not found: {svg_path}")
        return False
    
    try:
        # Create iconset directory
        iconset_dir = project_root / "src/resources/icons/phoenix-icon.iconset"
        iconset_dir.mkdir(exist_ok=True)
        
        # Define required icon sizes for macOS
        sizes = [
            (16, "icon_16x16.png"),
            (32, "icon_16x16@2x.png"),
            (32, "icon_32x32.png"),
            (64, "icon_32x32@2x.png"),
            (128, "icon_128x128.png"),
            (256, "icon_128x128@2x.png"),
            (256, "icon_256x256.png"),
            (512, "icon_256x256@2x.png"),
            (512, "icon_512x512.png"),
            (1024, "icon_512x512@2x.png")
        ]
        
        print("📐 Creating icon sizes...")
        
        # Create PNG files for each size
        for size, filename in sizes:
            png_path = iconset_dir / filename
            try:
                # Use rsvg-convert if available, otherwise use ImageMagick
                cmd = ["rsvg-convert", "-w", str(size), "-h", str(size), str(svg_path), "-o", str(png_path)]
                result = subprocess.run(cmd, capture_output=True, text=True)
                
                if result.returncode != 0:
                    # Fallback to ImageMagick
                    cmd = ["convert", "-background", "transparent", "-resize", f"{size}x{size}", str(svg_path), str(png_path)]
                    result = subprocess.run(cmd, capture_output=True, text=True)
                    
                if result.returncode == 0:
                    print(f"✅ Created {filename} ({size}x{size})")
                else:
                    print(f"❌ Failed to create {filename}: {result.stderr}")
                    
            except FileNotFoundError:
                print(f"❌ No conversion tool available for {filename}")
        
        # Create ICNS file
        print("📦 Creating ICNS file...")
        cmd = ["iconutil", "-c", "icns", str(iconset_dir), "-o", str(icns_path)]
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"✅ ICNS created: {icns_path}")
            
            # Clean up iconset directory
            import shutil
            shutil.rmtree(iconset_dir)
            print("🧹 Cleaned up temporary files")
            
            return True
        else:
            print(f"❌ Failed to create ICNS: {result.stderr}")
            return False
            
    except Exception as e:
        print(f"❌ Error creating macOS icon: {e}")
        return False

if __name__ == "__main__":
    success = create_mac_icon()
    if success:
        print("\n🎉 macOS icon created successfully!")
    else:
        print("\n❌ Failed to create macOS icon")
