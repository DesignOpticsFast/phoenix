#!/usr/bin/env python3
"""
Show Phoenix UI Screenshot Information
"""

import os
from pathlib import Path

def show_screenshot_info():
    """Display information about the captured screenshot"""
    project_root = Path(__file__).parent.parent
    screenshot_path = project_root / "screenshots" / "phoenix_ui_qt.png"
    
    print("📸 Phoenix UI Screenshot Information")
    print("=" * 50)
    
    if screenshot_path.exists():
        # Get file size
        file_size = screenshot_path.stat().st_size
        file_size_kb = file_size / 1024
        
        print(f"✅ Screenshot captured successfully!")
        print(f"📁 Location: {screenshot_path}")
        print(f"📏 Size: {file_size_kb:.1f} KB")
        print(f"🖼️  Format: PNG (1024x768)")
        
        print(f"\n🎯 What the screenshot shows:")
        print(f"   • MainWindow with title 'Phoenix - Optical Design Studio'")
        print(f"   • Menu Bar: File, Editors, Analysis, Tools, View, Help")
        print(f"   • Toolbar: New, Open, Save, Preferences buttons")
        print(f"   • Dock Widgets: Toolbox (left), Properties (right)")
        print(f"   • Status Bar: 'Ready' message and debug information")
        print(f"   • Central Widget: Empty placeholder area")
        
        print(f"\n📥 To view the screenshot:")
        print(f"   1. Download: {screenshot_path}")
        print(f"   2. Open in any image viewer")
        print(f"   3. Or use the HTML viewer: screenshots/view_screenshot.html")
        
        print(f"\n🌐 To serve locally:")
        print(f"   cd {project_root}/screenshots")
        print(f"   python3 -m http.server 8080")
        print(f"   Then visit: http://localhost:8080/view_screenshot.html")
        
    else:
        print(f"❌ Screenshot not found at: {screenshot_path}")
        print(f"   Run: python3 scripts/capture_phoenix_ui.py")

if __name__ == "__main__":
    show_screenshot_info()
