#!/usr/bin/env python3
"""
Simple UI Capture for Phoenix
Captures UI output using basic Xvfb and xwd
"""

import subprocess
import time
import os
import tempfile
from pathlib import Path

def capture_ui_screenshot():
    """Capture screenshot of Phoenix UI"""
    project_root = Path(__file__).parent.parent
    build_dir = project_root / "build-test-mac"
    app_path = build_dir / "phoenix_app"
    screenshots_dir = project_root / "screenshots"
    screenshots_dir.mkdir(exist_ok=True)
    
    print("📸 Capturing Phoenix UI Screenshot")
    print("=" * 40)
    
    # Clean up any existing X server
    subprocess.run(["pkill", "-f", "Xvfb"], capture_output=True)
    time.sleep(1)
    
    # Remove lock files
    subprocess.run(["rm", "-f", "/tmp/.X*-lock"], shell=True)
    
    try:
        # Start Xvfb with a unique display number
        display_num = ":99"
        xvfb_cmd = ["Xvfb", display_num, "-screen", "0", "1024x768x24", "-ac", "-nolisten", "tcp"]
        
        print(f"Starting Xvfb: {' '.join(xvfb_cmd)}")
        xvfb_process = subprocess.Popen(xvfb_cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
        # Wait for Xvfb to start
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
        time.sleep(3)
        
        # Try to capture screenshot using xwd
        screenshot_path = screenshots_dir / "phoenix_ui.xwd"
        
        try:
            # Capture the root window
            xwd_cmd = ["xwd", "-root", "-out", str(screenshot_path)]
            result = subprocess.run(xwd_cmd, env=env, capture_output=True, text=True, timeout=10)
            
            if result.returncode == 0 and screenshot_path.exists():
                print(f"✅ Screenshot captured: {screenshot_path}")
                
                # Try to convert to PNG if ImageMagick is available
                png_path = screenshots_dir / "phoenix_ui.png"
                try:
                    convert_cmd = ["convert", str(screenshot_path), str(png_path)]
                    subprocess.run(convert_cmd, check=True, capture_output=True)
                    print(f"✅ PNG version saved: {png_path}")
                    return str(png_path)
                except (subprocess.CalledProcessError, FileNotFoundError):
                    print(f"⚠️  ImageMagick not available, keeping XWD format")
                    return str(screenshot_path)
            else:
                print(f"❌ xwd failed: {result.stderr}")
                
        except subprocess.TimeoutExpired:
            print("⏰ xwd command timed out")
        except Exception as e:
            print(f"❌ Error capturing screenshot: {e}")
        
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
    screenshot_path = capture_ui_screenshot()
    if screenshot_path:
        print(f"\n🎉 Screenshot saved to: {screenshot_path}")
    else:
        print("\n❌ Screenshot capture failed")
