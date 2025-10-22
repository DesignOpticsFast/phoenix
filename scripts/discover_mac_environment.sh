#!/bin/bash

# Mac Environment Discovery Script
# Purpose: Identify Qt6 installation and system details on Mac
# Usage: Run on Mac via Tailscale connection

echo "🔍 MAC ENVIRONMENT DISCOVERY"
echo "============================"
echo ""

# System Information
echo "📱 SYSTEM INFORMATION"
echo "--------------------"
echo "macOS Version:"
sw_vers
echo ""
echo "Architecture:"
uname -m
echo ""
echo "Available Memory:"
system_profiler SPHardwareDataType | grep "Memory:"
echo ""

# Development Tools
echo "🛠️ DEVELOPMENT TOOLS"
echo "-------------------"
echo "CMake Version:"
cmake --version 2>/dev/null || echo "CMake not found"
echo ""

echo "Ninja Build:"
ninja --version 2>/dev/null || echo "Ninja not found"
echo ""

echo "Git Version:"
git --version 2>/dev/null || echo "Git not found"
echo ""

# Qt6 Discovery
echo "🔍 QT6 DISCOVERY"
echo "---------------"

# Check for qmake in common locations
echo "Searching for qmake..."
echo ""

# Check Homebrew
echo "Homebrew Qt6:"
if command -v brew &> /dev/null; then
    echo "Homebrew installed: $(brew --version | head -1)"
    echo "Qt packages:"
    brew list | grep qt || echo "No Qt packages found via Homebrew"
    echo ""
    echo "Homebrew Qt6 path:"
    brew --prefix qt@6 2>/dev/null || echo "Qt6 not found via Homebrew"
    echo ""
else
    echo "Homebrew not found"
    echo ""
fi

# Check common Qt installation paths
echo "Common Qt6 paths:"
for path in "/Users/$USER/Qt" "/opt/Qt" "/usr/local/Qt" "/Applications/Qt" "/opt/homebrew/lib/cmake/Qt6" "/usr/local/lib/cmake/Qt6"; do
    if [ -d "$path" ]; then
        echo "Found: $path"
        ls -la "$path" 2>/dev/null | head -5
    fi
done
echo ""

# Find qmake executables
echo "qmake executables found:"
find /Users -name "qmake*" 2>/dev/null | head -10
find /opt -name "qmake*" 2>/dev/null | head -10
find /usr/local -name "qmake*" 2>/dev/null | head -10
echo ""

# Check Qt6 version
echo "Qt6 version check:"
qmake6 -v 2>/dev/null || qmake -v 2>/dev/null || echo "qmake not found in PATH"
echo ""

# Find Qt6 CMake config files
echo "Qt6 CMake config files:"
find /Users -name "Qt6*Config.cmake" 2>/dev/null | head -10
find /opt -name "Qt6*Config.cmake" 2>/dev/null | head -10
find /usr/local -name "Qt6*Config.cmake" 2>/dev/null | head -10
echo ""

# Check for Qt6 components
echo "Qt6 Components Check:"
echo "Core:"
find /Users -name "Qt6Core*" 2>/dev/null | head -3
find /opt -name "Qt6Core*" 2>/dev/null | head -3
find /usr/local -name "Qt6Core*" 2>/dev/null | head -3
echo ""

echo "Widgets:"
find /Users -name "Qt6Widgets*" 2>/dev/null | head -3
find /opt -name "Qt6Widgets*" 2>/dev/null | head -3
find /usr/local -name "Qt6Widgets*" 2>/dev/null | head -3
echo ""

echo "Graphs:"
find /Users -name "Qt6Graphs*" 2>/dev/null | head -3
find /opt -name "Qt6Graphs*" 2>/dev/null | head -3
find /usr/local -name "Qt6Graphs*" 2>/dev/null | head -3
echo ""

# Test CMake with different paths
echo "🧪 CMAKE TESTING"
echo "---------------"

# Test common CMAKE_PREFIX_PATH values
for prefix in "/opt/homebrew/lib/cmake/Qt6" "/usr/local/lib/cmake/Qt6" "/Users/$USER/Qt/6.10.0/macos/lib/cmake/Qt6" "/opt/Qt/6.10.0/macos/lib/cmake/Qt6"; do
    if [ -d "$prefix" ]; then
        echo "Testing CMAKE_PREFIX_PATH: $prefix"
        CMAKE_PREFIX_PATH="$prefix" cmake --find-package -DNAME=Qt6 -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST 2>/dev/null && echo "✅ Qt6 found" || echo "❌ Qt6 not found"
        echo ""
    fi
done

echo "🎯 RECOMMENDATIONS"
echo "----------------"
echo "1. If Qt6 is installed via Homebrew:"
echo "   export CMAKE_PREFIX_PATH=\$(brew --prefix qt@6)/lib/cmake/Qt6"
echo ""
echo "2. If Qt6 is installed via Qt Installer:"
echo "   export CMAKE_PREFIX_PATH=/Users/\$USER/Qt/6.10.0/macos/lib/cmake/Qt6"
echo ""
echo "3. If Qt6 is installed manually:"
echo "   export CMAKE_PREFIX_PATH=/path/to/qt6/lib/cmake/Qt6"
echo ""

echo "✅ DISCOVERY COMPLETE"
echo "==================="

