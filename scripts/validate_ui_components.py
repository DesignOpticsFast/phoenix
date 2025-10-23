#!/usr/bin/env python3
"""
UI Component Validator for Phoenix
Validates that UI components are properly implemented
"""

import re
import json
from pathlib import Path

class UIComponentValidator:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.results = {}
        
    def validate_menu_structure(self):
        """Validate menu structure in MainWindow.cpp"""
        print("🔍 Validating Menu Structure...")
        
        mainwindow_cpp = self.project_root / "src/ui/main/MainWindow.cpp"
        if not mainwindow_cpp.exists():
            return {"error": "MainWindow.cpp not found"}
            
        with open(mainwindow_cpp, 'r') as f:
            content = f.read()
            
        # Check for required menu methods
        required_menus = [
            "createFileMenu",
            "createEditorsMenu", 
            "createAnalysisMenu",
            "createToolsMenu",
            "createViewMenu",
            "createHelpMenu"
        ]
        
        found_menus = []
        missing_menus = []
        
        for menu in required_menus:
            if f"QMenu* MainWindow::{menu}()" in content:
                found_menus.append(menu)
            else:
                missing_menus.append(menu)
                
        # Check for menu setup
        setup_menu_found = "setupMenuBar()" in content
        
        # Check for specific actions
        file_actions = ["m_newAction", "m_openAction", "m_saveAction", "m_saveAsAction", "m_exitAction"]
        found_actions = [action for action in file_actions if action in content]
        
        return {
            "found_menus": found_menus,
            "missing_menus": missing_menus,
            "setup_menu_found": setup_menu_found,
            "file_actions_found": found_actions,
            "file_actions_missing": [action for action in file_actions if action not in content]
        }
        
    def validate_toolbar_implementation(self):
        """Validate toolbar implementation"""
        print("🔍 Validating Toolbar Implementation...")
        
        mainwindow_cpp = self.project_root / "src/ui/main/MainWindow.cpp"
        if not mainwindow_cpp.exists():
            return {"error": "MainWindow.cpp not found"}
            
        with open(mainwindow_cpp, 'r') as f:
            content = f.read()
            
        # Check for toolbar methods
        toolbar_checks = [
            ("setupToolBar", "setupToolBar()" in content),
            ("createMainToolBar", "createMainToolBar()" in content),
            ("addToolBar", "addToolBar(" in content),
            ("toolbar_actions", "toolBar->addAction(" in content),
            ("toolbar_movable", "setMovable(true)" in content),
            ("toolbar_floating", "setFloatable(true)" in content)
        ]
        
        return {name: found for name, found in toolbar_checks}
        
    def validate_dock_widgets(self):
        """Validate dock widget implementation"""
        print("🔍 Validating Dock Widgets...")
        
        mainwindow_cpp = self.project_root / "src/ui/main/MainWindow.cpp"
        if not mainwindow_cpp.exists():
            return {"error": "MainWindow.cpp not found"}
            
        with open(mainwindow_cpp, 'r') as f:
            content = f.read()
            
        # Check for dock widget setup
        dock_checks = [
            ("setupDockWidgets", "setupDockWidgets()" in content),
            ("QDockWidget", "QDockWidget" in content),
            ("addDockWidget", "addDockWidget(" in content),
            ("toolbox_dock", "m_toolboxDock" in content),
            ("properties_dock", "m_propertiesDock" in content),
            ("object_names", "setObjectName(" in content)
        ]
        
        return {name: found for name, found in dock_checks}
        
    def validate_icon_system(self):
        """Validate icon system implementation"""
        print("🔍 Validating Icon System...")
        
        # Check icon files
        icon_files = [
            "src/resources/icons/phoenix-icon.svg",
            "src/resources/icons/phx-icons.json"
        ]
        
        icon_results = {}
        for icon_file in icon_files:
            icon_path = self.project_root / icon_file
            icon_results[icon_file] = icon_path.exists()
            
        # Check resource file
        qrc_file = self.project_root / "src/ui/icons/phoenix_icons.qrc"
        qrc_content = ""
        if qrc_file.exists():
            with open(qrc_file, 'r') as f:
                qrc_content = f.read()
                
        qrc_checks = [
            ("phoenix_icon", "phoenix-icon.svg" in qrc_content),
            ("phx_icons_json", "phx-icons.json" in qrc_content),
            ("font_files", "fa-sharp-solid-900.ttf" in qrc_content)
        ]
        
        return {
            "icon_files": icon_results,
            "qrc_checks": {name: found for name, found in qrc_checks}
        }
        
    def validate_preferences_dialog(self):
        """Validate preferences dialog implementation"""
        print("🔍 Validating Preferences Dialog...")
        
        # Check for LanguagePage
        language_page_h = self.project_root / "src/ui/dialogs/LanguagePage.h"
        language_page_cpp = self.project_root / "src/ui/dialogs/LanguagePage.cpp"
        
        language_checks = [
            ("language_page_h", language_page_h.exists()),
            ("language_page_cpp", language_page_cpp.exists())
        ]
        
        # Check CMakeLists.txt for new files
        cmake_file = self.project_root / "CMakeLists.txt"
        cmake_content = ""
        if cmake_file.exists():
            with open(cmake_file, 'r') as f:
                cmake_content = f.read()
                
        cmake_checks = [
            ("language_page_cpp", "LanguagePage.cpp" in cmake_content),
            ("language_page_h", "LanguagePage.h" in cmake_content)
        ]
        
        return {
            "language_files": {name: found for name, found in language_checks},
            "cmake_includes": {name: found for name, found in cmake_checks}
        }
        
    def run_full_validation(self):
        """Run complete UI validation"""
        print("🚀 Phoenix UI Component Validation")
        print("=" * 50)
        
        results = {
            "menu_structure": self.validate_menu_structure(),
            "toolbar": self.validate_toolbar_implementation(),
            "dock_widgets": self.validate_dock_widgets(),
            "icon_system": self.validate_icon_system(),
            "preferences": self.validate_preferences_dialog()
        }
        
        # Print summary
        print("\n📊 Validation Summary:")
        print("=" * 30)
        
        for category, data in results.items():
            print(f"\n{category.upper()}:")
            if isinstance(data, dict):
                for key, value in data.items():
                    status = "✅" if value else "❌"
                    print(f"  {status} {key}: {value}")
            else:
                print(f"  {data}")
                
        return results

if __name__ == "__main__":
    import sys
    
    project_root = sys.argv[1] if len(sys.argv) > 1 else "/home/ec2-user/workspace/phoenix"
    validator = UIComponentValidator(project_root)
    results = validator.run_full_validation()
    
    # Save results to file
    results_file = Path(project_root) / "ui_validation_results.json"
    with open(results_file, 'w') as f:
        json.dump(results, f, indent=2)
        
    print(f"\n📄 Results saved to: {results_file}")
