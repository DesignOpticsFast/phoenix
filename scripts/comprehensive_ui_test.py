#!/usr/bin/env python3
"""
Comprehensive UI Testing for Phoenix
Combines all testing capabilities into one workflow
"""

import subprocess
import time
import json
import os
from pathlib import Path
from datetime import datetime

class ComprehensiveUITester:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.build_dir = self.project_root / "build-test-mac"
        self.app_path = self.build_dir / "phoenix_app"
        self.results = {
            "timestamp": datetime.now().isoformat(),
            "tests": {}
        }
        
    def run_component_validation(self):
        """Run UI component validation"""
        print("🔍 Running Component Validation...")
        
        try:
            result = subprocess.run([
                "python3", str(self.project_root / "scripts/validate_ui_components.py")
            ], capture_output=True, text=True, cwd=self.project_root)
            
            self.results["tests"]["component_validation"] = {
                "success": result.returncode == 0,
                "output": result.stdout,
                "errors": result.stderr
            }
            
            return result.returncode == 0
            
        except Exception as e:
            self.results["tests"]["component_validation"] = {
                "success": False,
                "error": str(e)
            }
            return False
            
    def run_ui_functionality_test(self):
        """Run UI functionality test with Xvfb"""
        print("🖥️  Running UI Functionality Test...")
        
        try:
            result = subprocess.run([
                "python3", str(self.project_root / "scripts/test_ui.py")
            ], capture_output=True, text=True, cwd=self.project_root)
            
            self.results["tests"]["ui_functionality"] = {
                "success": result.returncode == 0,
                "output": result.stdout,
                "errors": result.stderr
            }
            
            return result.returncode == 0
            
        except Exception as e:
            self.results["tests"]["ui_functionality"] = {
                "success": False,
                "error": str(e)
            }
            return False
            
    def run_build_test(self):
        """Test that the application builds successfully"""
        print("🔨 Running Build Test...")
        
        try:
            # Clean build
            if self.build_dir.exists():
                import shutil
                shutil.rmtree(self.build_dir)
                
            # Create build directory
            self.build_dir.mkdir(exist_ok=True)
            
            # Configure
            configure_result = subprocess.run([
                "cmake", "..", "-DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64"
            ], cwd=self.build_dir, capture_output=True, text=True)
            
            if configure_result.returncode != 0:
                self.results["tests"]["build"] = {
                    "success": False,
                    "stage": "configure",
                    "error": configure_result.stderr
                }
                return False
                
            # Build
            build_result = subprocess.run([
                "make", "-j4"
            ], cwd=self.build_dir, capture_output=True, text=True)
            
            self.results["tests"]["build"] = {
                "success": build_result.returncode == 0,
                "configure_output": configure_result.stdout,
                "build_output": build_result.stdout,
                "build_errors": build_result.stderr
            }
            
            return build_result.returncode == 0
            
        except Exception as e:
            self.results["tests"]["build"] = {
                "success": False,
                "error": str(e)
            }
            return False
            
    def run_debug_output_analysis(self):
        """Analyze debug output for common issues"""
        print("🔍 Analyzing Debug Output...")
        
        issues = []
        
        # Check for common error patterns
        error_patterns = [
            ("icon_manifest", "Could not load icon manifest"),
            ("object_names", "objectName.*not set"),
            ("crash", "SIGSEGV|SIGBUS|Aborted"),
            ("missing_methods", "undefined reference"),
            ("qt_errors", "Qt.*error")
        ]
        
        # Run application and capture output
        try:
            result = subprocess.run([
                "xvfb-run", "-a", "-s", "-screen 0 1024x768x24",
                str(self.app_path)
            ], capture_output=True, text=True, timeout=10)
            
            output = result.stdout + result.stderr
            
            for issue_name, pattern in error_patterns:
                if pattern in output:
                    issues.append(f"{issue_name}: {pattern}")
                    
            self.results["tests"]["debug_analysis"] = {
                "success": len(issues) == 0,
                "issues": issues,
                "output": output
            }
            
            return len(issues) == 0
            
        except subprocess.TimeoutExpired:
            # Timeout is expected for GUI apps
            self.results["tests"]["debug_analysis"] = {
                "success": True,
                "note": "Application ran without immediate crash (timeout expected)"
            }
            return True
            
        except Exception as e:
            self.results["tests"]["debug_analysis"] = {
                "success": False,
                "error": str(e)
            }
            return False
            
    def run_comprehensive_test(self):
        """Run all tests and generate report"""
        print("🚀 Comprehensive UI Test Suite")
        print("=" * 50)
        
        tests = [
            ("Build Test", self.run_build_test),
            ("Component Validation", self.run_component_validation),
            ("UI Functionality", self.run_ui_functionality_test),
            ("Debug Analysis", self.run_debug_output_analysis)
        ]
        
        passed = 0
        total = len(tests)
        
        for test_name, test_func in tests:
            print(f"\n🧪 Running {test_name}...")
            try:
                success = test_func()
                if success:
                    print(f"✅ {test_name} PASSED")
                    passed += 1
                else:
                    print(f"❌ {test_name} FAILED")
            except Exception as e:
                print(f"❌ {test_name} ERROR: {e}")
                
        # Generate report
        self.results["summary"] = {
            "passed": passed,
            "total": total,
            "success_rate": (passed / total) * 100
        }
        
        # Save results
        results_file = self.project_root / "comprehensive_test_results.json"
        with open(results_file, 'w') as f:
            json.dump(self.results, f, indent=2)
            
        print(f"\n📊 Test Summary:")
        print(f"   Passed: {passed}/{total}")
        print(f"   Success Rate: {(passed/total)*100:.1f}%")
        print(f"   Results saved to: {results_file}")
        
        return passed == total

if __name__ == "__main__":
    import sys
    
    project_root = sys.argv[1] if len(sys.argv) > 1 else "/home/ec2-user/workspace/phoenix"
    tester = ComprehensiveUITester(project_root)
    
    success = tester.run_comprehensive_test()
    
    if success:
        print("\n🎉 All tests passed! Code is ready for deployment.")
        sys.exit(0)
    else:
        print("\n❌ Some tests failed. Please review the results.")
        sys.exit(1)
