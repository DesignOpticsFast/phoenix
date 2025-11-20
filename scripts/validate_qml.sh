#!/usr/bin/env bash
set -euo pipefail

# QML Validation Script for Phoenix
# Validates all QML files under src/qml/ for:
# - qmllint syntax/type checking
# - Forbidden QtCharts imports
# - Forbidden QtCharts properties

root="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
cd "$root"

# Find qmllint
QML_LINT=""
if [ -f "/opt/Qt/6.10.0/gcc_64/bin/qmllint" ]; then
    QML_LINT="/opt/Qt/6.10.0/gcc_64/bin/qmllint"
elif command -v qmllint >/dev/null 2>&1; then
    QML_LINT="qmllint"
else
    echo "❌ ERROR: qmllint not found"
    echo "   Please ensure Qt 6.10 is installed and qmllint is in PATH"
    exit 1
fi

# Find all QML files
qml_dir="src/qml"
if [ ! -d "$qml_dir" ]; then
    echo "⚠️  WARNING: QML directory not found: $qml_dir"
    echo "✅ QML validation skipped (no QML files)"
    exit 0
fi

qml_files=$(find "$qml_dir" -name "*.qml" -type f 2>/dev/null | sort || true)

if [ -z "$qml_files" ]; then
    echo "ℹ️  No QML files found in $qml_dir"
    echo "✅ QML validation skipped (no QML files)"
    exit 0
fi

file_count=$(echo "$qml_files" | grep -c . || echo "0")
echo "[qml-validation] Checking $file_count QML file(s)..."

# Policy check: Forbidden QtCharts imports and properties
echo "[qml-validation] Policy check: Scanning for forbidden QtCharts usage..."

forbidden_patterns=(
    "import QtCharts"
    "gridLineColor"
    "backgroundColor"
    "plotAreaColor"
)

violations_found=false
violation_files=()

for pattern in "${forbidden_patterns[@]}"; do
    # Use grep to find violations
    violating_files=$(echo "$qml_files" | xargs grep -l "$pattern" 2>/dev/null || true)
    if [ -n "$violating_files" ]; then
        violations_found=true
        while IFS= read -r file; do
            if [ -n "$file" ]; then
                violation_files+=("$file:$pattern")
                echo "❌ FORBIDDEN: $file uses '$pattern'"
            fi
        done <<< "$violating_files"
    fi
done

if [ "$violations_found" = true ]; then
    echo ""
    echo "❌ QML VALIDATION FAILED: Forbidden QtCharts usage detected"
    echo ""
    echo "The following files contain forbidden QtCharts imports or properties:"
    for violation in "${violation_files[@]}"; do
        echo "  - $violation"
    done
    echo ""
    echo "QtCharts is deprecated. Use QtGraphs 6.10 instead."
    echo "Forbidden patterns:"
    for pattern in "${forbidden_patterns[@]}"; do
        echo "  - $pattern"
    done
    exit 1
fi

echo "[qml-validation] ✅ Policy check passed (no forbidden patterns)"

# Run qmllint on all QML files
echo "[qml-validation] Running qmllint..."

lint_errors=false
lint_output=""

while IFS= read -r qml_file; do
    if [ -z "$qml_file" ]; then
        continue
    fi
    
    # Run qmllint (capture output)
    file_output=$("$QML_LINT" "$qml_file" 2>&1 || true)
    lint_exit=$?
    
    if [ $lint_exit -ne 0 ] || [ -n "$file_output" ]; then
        lint_errors=true
        lint_output="${lint_output}${qml_file}:\n${file_output}\n\n"
        echo "❌ qmllint failed for: $qml_file"
        echo "$file_output"
    fi
done <<< "$qml_files"

if [ "$lint_errors" = true ]; then
    echo ""
    echo "❌ QML VALIDATION FAILED: qmllint errors detected"
    echo ""
    echo "qmllint output:"
    echo -e "$lint_output"
    echo ""
    echo "Fix QML syntax/type errors before proceeding."
    exit 1
fi

echo "[qml-validation] ✅ qmllint passed (all files clean)"
echo ""
echo "✅ QML validation passed: $file_count file(s) checked."

