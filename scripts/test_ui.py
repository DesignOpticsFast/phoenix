#!/usr/bin/env python3
"""
UI Testing Script for Phoenix Application
Tests UI components without requiring a display
"""

import subprocess
import sys
import os
import time
import json
from pathlib import Path

class PhoenixUITester:
    def __init__(self):
        self.project_root = Path(__file__).parent.parent
        self.build_dir = self.project_root / "build-test-mac"
        self.app_path = self.build_dir / "phoenix_app"
        
    def run_with_xvfb(self, timeout=10):
        """Run Phoenix app with Xvfb virtual display"""
        try:
            # Set up virtual display
            cmd = [
                "xvfb-run", "-a", "-s", "-screen 0 1024x768x24",
                str(self.app_path)
            ]
            
            print(f"Running: {' '.join(cmd)}")
            
            # Run with timeout
            process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            
            # Wait for startup
            time.sleep(2)
            
            # Check if process is still running
            if process.poll() is None:
                print("✅ Application started successfully")
                
                # Get debug output
                stdout, stderr = process.communicate(timeout=timeout-2)
                
                # Analyze output
                self.analyze_output(stdout, stderr)
                
                # Terminate gracefully
                process.terminate()
                process.wait(timeout=5)
                
            else:
                print("❌ Application crashed immediately")
                stdout, stderr = process.communicate()
                print("STDOUT:", stdout)
                print("STDERR:", stderr)
                
        except subprocess.TimeoutExpired:
            print("⏰ Application timed out (expected)")
            process.terminate()
        except Exception as e:
            print(f"❌ Error running application: {e}")
            
    def analyze_output(self, stdout, stderr):
        """Analyze application output for UI issues"""
        print("\n🔍 Analyzing Application Output:")
        print("=" * 50)
        
        # Check for debug messages
        if "Creating toolbar with actions:" in stderr:
            print("✅ Toolbar creation debug found")
        else:
            print("❌ No toolbar creation debug found")
            
        if "Toolbar created and added:" in stderr:
            print("✅ Toolbar added debug found")
        else:
            print("❌ No toolbar added debug found")
            
        # Check for icon loading issues
        if "Could not load icon manifest" in stderr:
            print("❌ Icon manifest loading issues detected")
        else:
            print("✅ No icon manifest issues")
            
        # Check for object name warnings
        if "objectName' not set" in stderr:
            print("❌ Object name warnings detected")
        else:
            print("✅ No object name warnings")
            
        # Check for crash indicators
        if "SIGSEGV" in stderr or "SIGBUS" in stderr:
            print("❌ Application crash detected")
        else:
            print("✅ No crash indicators")
            
        print("\n📋 Full Output:")
        print("STDOUT:", stdout)
        print("STDERR:", stderr)
        
    def validate_build(self):
        """Validate that the build is up to date"""
        print("🔨 Validating Build:")
        print("=" * 30)
        
        if not self.app_path.exists():
            print("❌ Application binary not found")
            return False
            
        # Check if build is recent
        build_time = self.app_path.stat().st_mtime
        current_time = time.time()
        
        if current_time - build_time > 300:  # 5 minutes
            print("⚠️  Build is older than 5 minutes")
            print("   Consider running: make -j4")
        else:
            print("✅ Build is recent")
            
        return True
        
    def check_source_files(self):
        """Check if source files have expected content"""
        print("\n📁 Checking Source Files:")
        print("=" * 30)
        
        # Check MainWindow.cpp for menu structure
        mainwindow_cpp = self.project_root / "src/ui/main/MainWindow.cpp"
        if mainwindow_cpp.exists():
            with open(mainwindow_cpp, 'r') as f:
                content = f.read()
                
            checks = [
                ("File menu", "createFileMenu()"),
                ("Editors menu", "createEditorsMenu()"),
                ("Analysis menu", "createAnalysisMenu()"),
                ("Tools menu", "createToolsMenu()"),
                ("Help menu", "createHelpMenu()"),
                ("Exit action", "m_exitAction"),
                ("Toolbar setup", "setupToolBar()"),
                ("App icon", "setWindowIcon")
            ]
            
            for name, pattern in checks:
                if pattern in content:
                    print(f"✅ {name} found")
                else:
                    print(f"❌ {name} missing")
        else:
            print("❌ MainWindow.cpp not found")
            
    def run_full_test(self):
        """Run complete UI test suite"""
        print("🚀 Phoenix UI Test Suite")
        print("=" * 50)
        
        # Validate build
        if not self.validate_build():
            return False
            
        # Check source files
        self.check_source_files()
        
        # Run application test
        print("\n🖥️  Running Application Test:")
        print("=" * 30)
        self.run_with_xvfb()
        
        return True

if __name__ == "__main__":
    tester = PhoenixUITester()
    success = tester.run_full_test()
    
    if success:
        print("\n✅ UI Test Suite Completed")
        sys.exit(0)
    else:
        print("\n❌ UI Test Suite Failed")
        sys.exit(1)
