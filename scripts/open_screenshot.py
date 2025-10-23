#!/usr/bin/env python3
"""
Open Phoenix UI Screenshot in Cursor
"""

import subprocess
import os
from pathlib import Path

def open_screenshot_in_cursor():
    """Open the Phoenix UI screenshot in Cursor"""
    project_root = Path(__file__).parent.parent
    screenshot_path = project_root / "screenshots" / "phoenix_ui_qt.png"
    
    print("🖼️  Opening Phoenix UI Screenshot in Cursor")
    print("=" * 50)
    
    if screenshot_path.exists():
        print(f"✅ Screenshot found: {screenshot_path}")
        print(f"📏 Size: {screenshot_path.stat().st_size} bytes")
        
        # Try to open in Cursor
        try:
            # Use cursor command if available
            result = subprocess.run([
                "cursor", str(screenshot_path)
            ], capture_output=True, text=True)
            
            if result.returncode == 0:
                print("✅ Screenshot opened in Cursor")
            else:
                print("⚠️  Cursor command not found, trying alternative...")
                
                # Try with code command (VS Code compatible)
                result = subprocess.run([
                    "code", str(screenshot_path)
                ], capture_output=True, text=True)
                
                if result.returncode == 0:
                    print("✅ Screenshot opened in VS Code")
                else:
                    print("❌ Neither Cursor nor VS Code found")
                    print(f"📁 Manual path: {screenshot_path}")
                    
        except FileNotFoundError:
            print("❌ Cursor not found in PATH")
            print(f"📁 Manual path: {screenshot_path}")
            
    else:
        print(f"❌ Screenshot not found: {screenshot_path}")
        print("   Run: python3 scripts/capture_phoenix_ui.py")

if __name__ == "__main__":
    open_screenshot_in_cursor()
