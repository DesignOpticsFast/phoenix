#!/bin/bash
"""
Pre-commit Testing Workflow for Phoenix
Runs comprehensive tests before committing changes
"""

set -e  # Exit on any error

PROJECT_ROOT="/home/ec2-user/workspace/phoenix"
BUILD_DIR="$PROJECT_ROOT/build-test-mac"

echo "🚀 Phoenix Pre-commit Test Suite"
echo "================================="

# 1. Build the application
echo "🔨 Building application..."
cd "$PROJECT_ROOT"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64
make -j4

# 2. Run UI component validation
echo "🔍 Validating UI components..."
python3 "$PROJECT_ROOT/scripts/validate_ui_components.py"

# 3. Run UI test with Xvfb
echo "🖥️  Testing UI with virtual display..."
python3 "$PROJECT_ROOT/scripts/test_ui.py"

# 4. Check for common issues
echo "🔍 Checking for common issues..."

# Check for missing includes
if grep -q "Could not load icon manifest" "$BUILD_DIR/phoenix_app" 2>/dev/null; then
    echo "❌ Icon manifest issues detected"
    exit 1
fi

# Check for object name warnings
if grep -q "objectName.*not set" "$BUILD_DIR/phoenix_app" 2>/dev/null; then
    echo "❌ Object name warnings detected"
    exit 1
fi

echo "✅ All pre-commit tests passed!"
echo "🎉 Ready to commit changes"
