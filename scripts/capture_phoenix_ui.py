#!/usr/bin/env python3
"""
Capture Phoenix UI using Qt screenshot capabilities
"""

import subprocess
import time
import os
from pathlib import Path

def capture_phoenix_ui():
    """Capture Phoenix UI using Qt screenshot tool"""
    project_root = Path(__file__).parent.parent
    build_dir = project_root / "build-test-mac"
    app_path = build_dir / "phoenix_app"
    qt_screenshot_path = project_root / "scripts/build/qt_screenshot"
    screenshots_dir = project_root / "screenshots"
    screenshots_dir.mkdir(exist_ok=True)
    
    print("📸 Capturing Phoenix UI with Qt Screenshot Tool")
    print("=" * 50)
    
    # Clean up any existing X server
    subprocess.run(["pkill", "-f", "Xvfb"], capture_output=True)
    subprocess.run(["pkill", "-f", "phoenix_app"], capture_output=True)
    time.sleep(1)
    
    # Remove lock files
    subprocess.run(["rm", "-f", "/tmp/.X*-lock"], shell=True)
    
    try:
        # Start Xvfb
        display_num = ":99"
        xvfb_cmd = ["Xvfb", display_num, "-screen", "0", "1024x768x24", "-ac", "-nolisten", "tcp"]
        
        print(f"Starting Xvfb: {' '.join(xvfb_cmd)}")
        xvfb_process = subprocess.Popen(xvfb_cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(2)
        
        # Set display environment
        env = os.environ.copy()
        env["DISPLAY"] = display_num
        
        print("Starting Phoenix application...")
        
        # Start Phoenix application
        app_process = subprocess.Popen(
            [str(app_path)],
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        
        # Wait for application to start and render
        time.sleep(4)
        
        print("Capturing screenshot with Qt tool...")
        
        # Use Qt screenshot tool
        qt_result = subprocess.run(
            [str(qt_screenshot_path)],
            env=env,
            capture_output=True,
            text=True,
            timeout=10
        )
        
        if qt_result.returncode == 0:
            print("✅ Qt screenshot captured successfully")
            print("Output:", qt_result.stdout.strip())
            
            # Check if screenshot was saved
            screenshot_path = screenshots_dir / "phoenix_ui_qt.png"
            if screenshot_path.exists():
                print(f"✅ Screenshot saved: {screenshot_path}")
                return str(screenshot_path)
            else:
                print("❌ Screenshot file not found")
        else:
            print(f"❌ Qt screenshot failed: {qt_result.stderr}")
        
        # Terminate application
        app_process.terminate()
        try:
            app_process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            app_process.kill()
        
        # Terminate Xvfb
        xvfb_process.terminate()
        try:
            xvfb_process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            xvfb_process.kill()
            
    except Exception as e:
        print(f"❌ Error in screenshot capture: {e}")
    
    return None

if __name__ == "__main__":
    screenshot_path = capture_phoenix_ui()
    if screenshot_path:
        print(f"\n🎉 Phoenix UI Screenshot saved to: {screenshot_path}")
    else:
        print("\n❌ Screenshot capture failed")
