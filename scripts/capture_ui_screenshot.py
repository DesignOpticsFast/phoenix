#!/usr/bin/env python3
"""
UI Screenshot Capture Tool for Phoenix
Captures screenshots of the application using Xvfb
"""

import subprocess
import time
import os
from pathlib import Path

class UIScreenshotCapture:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.build_dir = self.project_root / "build-test-mac"
        self.app_path = self.build_dir / "phoenix_app"
        self.screenshots_dir = self.project_root / "screenshots"
        self.screenshots_dir.mkdir(exist_ok=True)
        
    def capture_screenshot(self, timeout=15):
        """Capture screenshot of Phoenix application"""
        try:
            # Start Xvfb with specific display
            display_num = ":99"
            xvfb_cmd = [
                "Xvfb", display_num, "-screen", "0", "1024x768x24", "-ac"
            ]
            
            print(f"Starting Xvfb: {' '.join(xvfb_cmd)}")
            xvfb_process = subprocess.Popen(xvfb_cmd)
            
            # Wait for Xvfb to start
            time.sleep(2)
            
            # Set display environment
            env = os.environ.copy()
            env["DISPLAY"] = display_num
            
            # Start Phoenix application
            print(f"Starting Phoenix application...")
            app_process = subprocess.Popen(
                [str(self.app_path)],
                env=env,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            
            # Wait for application to start
            time.sleep(3)
            
            # Capture screenshot using ImageMagick (if available)
            screenshot_path = self.screenshots_dir / "phoenix_ui.png"
            
            try:
                # Try to capture screenshot
                capture_cmd = [
                    "import", "-window", "root", str(screenshot_path)
                ]
                subprocess.run(capture_cmd, env=env, check=True)
                print(f"✅ Screenshot captured: {screenshot_path}")
                
            except (subprocess.CalledProcessError, FileNotFoundError):
                print("⚠️  ImageMagick not available, using alternative method")
                
                # Alternative: Use xwd to capture window
                try:
                    xwd_cmd = [
                        "xwd", "-root", "-out", str(screenshot_path.with_suffix('.xwd'))
                    ]
                    subprocess.run(xwd_cmd, env=env, check=True)
                    print(f"✅ XWD capture saved: {screenshot_path.with_suffix('.xwd')}")
                except (subprocess.CalledProcessError, FileNotFoundError):
                    print("❌ No screenshot capture method available")
            
            # Terminate application
            app_process.terminate()
            app_process.wait(timeout=5)
            
            # Terminate Xvfb
            xvfb_process.terminate()
            xvfb_process.wait(timeout=5)
            
            return screenshot_path.exists()
            
        except Exception as e:
            print(f"❌ Error capturing screenshot: {e}")
            return False
            
    def capture_multiple_views(self):
        """Capture multiple views of the application"""
        views = [
            ("startup", 2),      # Capture at startup
            ("menu_open", 5),    # Capture with menu open
            ("preferences", 8)   # Capture preferences dialog
        ]
        
        results = {}
        for view_name, delay in views:
            print(f"📸 Capturing {view_name} view...")
            time.sleep(delay)
            success = self.capture_screenshot()
            results[view_name] = success
            
        return results

if __name__ == "__main__":
    import sys
    
    project_root = sys.argv[1] if len(sys.argv) > 1 else "/home/ec2-user/workspace/phoenix"
    capturer = UIScreenshotCapture(project_root)
    
    print("📸 Phoenix UI Screenshot Capture")
    print("=" * 40)
    
    success = capturer.capture_screenshot()
    if success:
        print("✅ Screenshot capture completed")
    else:
        print("❌ Screenshot capture failed")
        sys.exit(1)
