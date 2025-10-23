#!/usr/bin/env python3
"""
Font Awesome Pro Icon Manager
Smart tool for managing Font Awesome Pro icons without GitHub size limits
"""

import json
import os
import sys
from pathlib import Path

class FontAwesomeManager:
    def __init__(self, base_path="assets/icons/fontawesome-pro"):
        self.base_path = Path(base_path)
        self.fonts_path = self.base_path / "fonts"
        self.svgs_path = self.base_path / "svgs"
        self.metadata_path = self.base_path / "metadata"

    def create_icon_index(self):
        """Create a lightweight index of available icons"""
        index = {
            "version": "1.0.0",
            "fonts": {
                "sharp-solid": "fa-sharp-solid-900.ttf",
                "sharp-regular": "fa-sharp-regular-400.ttf",
                "duotone": "fa-duotone-900.ttf",
                "brands": "fa-brands-400.ttf",
                "classic-solid": "fa-solid-900.ttf"
            },
            "icon_categories": {
                "sharp-solid": [
                    "save", "open", "chart-line", "chart-scatter", "wave-sine",
                    "cube", "ruler-combined", "brain", "circle-question"
                ],
                "duotone": [
                    "chart-line", "chart-scatter"
                ],
                "brands": [
                    "github", "twitter", "linkedin"
                ]
            }
        }

        # Save lightweight index
        with open(self.metadata_path / "icon-index.json", "w") as f:
            json.dump(index, f, indent=2)

        return index

    def add_icon(self, name, style="sharp-solid", unicode=None):
        """Add an icon to the index"""
        index_file = self.metadata_path / "icon-index.json"

        if index_file.exists():
            with open(index_file, "r") as f:
                index = json.load(f)
        else:
            index = self.create_icon_index()

        if style not in index["icon_categories"]:
            index["icon_categories"][style] = []

        if name not in index["icon_categories"][style]:
            index["icon_categories"][style].append(name)

        with open(index_file, "w") as f:
            json.dump(index, f, indent=2)

        print(f"Added icon '{name}' to style '{style}'")

    def get_available_icons(self, style=None):
        """Get list of available icons"""
        index_file = self.metadata_path / "icon-index.json"

        if not index_file.exists():
            return {}

        with open(index_file, "r") as f:
            index = json.load(f)

        if style:
            return index["icon_categories"].get(style, [])
        else:
            return index["icon_categories"]

    def generate_usage_examples(self):
        """Generate usage examples for developers"""
        examples = {
            "font_usage": {
                "qml": """
// QML Font Usage
Text {
    text: "\\uf0c7"  // Unicode for save icon
    font.family: "Font Awesome 6 Pro"
    font.styleName: "Sharp Solid"
    font.pixelSize: 24
    color: "#333"
}""",
                "cpp": """
// C++ Font Usage
QFont font("Font Awesome 6 Pro", 24, QFont::Bold);
font.setStyleName("Sharp Solid");
QString icon = QString::fromUtf8("\\uf0c7");  // Save icon
"""
            },
            "svg_usage": {
                "qml": """
// QML SVG Usage
Image {
    source: "image://phxicon/save?style=SharpSolid&size=24"
    width: 24
    height: 24
}""",
                "cpp": """
// C++ SVG Usage
QIcon icon(":/icons/save.svg");
QPushButton button;
button.setIcon(icon);
"""
            }
        }

        with open(self.metadata_path / "usage-examples.json", "w") as f:
            json.dump(examples, f, indent=2)

    def create_gitignore(self):
        """Create .gitignore to exclude large files"""
        gitignore_content = """# Font Awesome Pro - Exclude large files
metadata/icon-families.json
metadata/icons.json
metadata/icon-metadata.json
# Keep only essential files
!metadata/icon-index.json
!metadata/usage-examples.json
!fonts/*.ttf
!svgs/*.svg
"""

        with open(self.base_path / ".gitignore", "w") as f:
            f.write(gitignore_content)

if __name__ == "__main__":
    manager = FontAwesomeManager()

    if len(sys.argv) > 1:
        command = sys.argv[1]

        if command == "init":
            manager.create_icon_index()
            manager.generate_usage_examples()
            manager.create_gitignore()
            print("✅ Font Awesome Pro manager initialized")

        elif command == "add" and len(sys.argv) > 2:
            icon_name = sys.argv[2]
            style = sys.argv[3] if len(sys.argv) > 3 else "sharp-solid"
            manager.add_icon(icon_name, style)

        elif command == "list":
            style = sys.argv[2] if len(sys.argv) > 2 else None
            icons = manager.get_available_icons(style)
            print(json.dumps(icons, indent=2))

    else:
        print("Font Awesome Pro Icon Manager")
        print("Usage:")
        print("  python icon-manager.py init")
        print("  python icon-manager.py add <icon-name> [style]")
        print("  python icon-manager.py list [style]")