#!/usr/bin/env bash
# Static Analysis - Run cppcheck and optionally clang-tidy on Palantir/transport/RemoteExecutor code
# Exit codes: 0=PASS, 1=FAIL, 2=SKIPPED

set +e  # CRITICAL: Never use set -e
set -u
set -o pipefail

# Task report directory (passed as first argument)
TASK_DIR="${1:-}"

if [ -z "$TASK_DIR" ]; then
  echo "Error: task directory not provided" >&2
  exit 1
fi

LOG_FILE="$TASK_DIR/static_analysis.log"
REPORT_FILE="$TASK_DIR/static_analysis_report.md"

{
  echo "=== Static Analysis ==="
  echo "Start: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
} > "$LOG_FILE"

# Check for cppcheck (required)
if ! command -v cppcheck >/dev/null 2>&1; then
  {
    echo "⚠️ SKIPPED: cppcheck not found in PATH"
    echo "Reason: cppcheck tool not available"
  } | tee -a "$LOG_FILE" > "$REPORT_FILE"
  exit 2
fi

# Check for clang-tidy (optional)
HAS_CLANG_TIDY=0
if command -v clang-tidy >/dev/null 2>&1; then
  HAS_CLANG_TIDY=1
  echo "clang-tidy found: $(clang-tidy --version | head -1)" >> "$LOG_FILE"
fi

echo "Using cppcheck: $(cppcheck --version | head -1)" >> "$LOG_FILE"

# Check for compile_commands.json (needed for clang-tidy)
HAS_COMPILE_COMMANDS=0
if [ -f "compile_commands.json" ] || [ -f "build/compile_commands.json" ]; then
  HAS_COMPILE_COMMANDS=1
  echo "compile_commands.json found" >> "$LOG_FILE"
fi

# Collect source files to analyze
# Scope: src/palantir/*, src/transport/*, src/analysis/RemoteExecutor*
SOURCE_FILES=""

# Find files in src/palantir/ (if exists)
if [ -d "src/palantir" ]; then
  while IFS= read -r file; do
    if [ -f "$file" ] && [ "${file##*.}" = "cpp" ] || [ "${file##*.}" = "hpp" ] || [ "${file##*.}" = "h" ]; then
      SOURCE_FILES="${SOURCE_FILES} ${file}"
    fi
  done < <(find src/palantir -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) 2>/dev/null || true)
fi

# Find files in src/transport/
if [ -d "src/transport" ]; then
  while IFS= read -r file; do
    if [ -f "$file" ] && [ "${file##*.}" = "cpp" ] || [ "${file##*.}" = "hpp" ] || [ "${file##*.}" = "h" ]; then
      SOURCE_FILES="${SOURCE_FILES} ${file}"
    fi
  done < <(find src/transport -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) 2>/dev/null || true)
fi

# Find RemoteExecutor files
if [ -d "src/analysis" ]; then
  while IFS= read -r file; do
    if [ -f "$file" ] && [ "${file##*.}" = "cpp" ] || [ "${file##*.}" = "hpp" ] || [ "${file##*.}" = "h" ]; then
      SOURCE_FILES="${SOURCE_FILES} ${file}"
    fi
  done < <(find src/analysis -name "RemoteExecutor*" -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) 2>/dev/null || true)
fi

# Remove leading space
SOURCE_FILES=$(echo "$SOURCE_FILES" | sed 's/^ *//')

if [ -z "$SOURCE_FILES" ]; then
  {
    echo "⚠️ SKIPPED: No source files found to analyze"
    echo "Searched in: src/palantir/, src/transport/, src/analysis/RemoteExecutor*"
    echo "Reason: Source files not found"
  } | tee -a "$LOG_FILE" > "$REPORT_FILE"
  exit 2
fi

echo "Found $(echo "$SOURCE_FILES" | wc -w | tr -d ' ') source file(s) to analyze" >> "$LOG_FILE"

# Run cppcheck
CPPCHECK_LOG="$TASK_DIR/cppcheck.log"
CPPCHECK_ERRORS=0
CPPCHECK_WARNINGS=0
CPPCHECK_STYLE=0
CPPCHECK_OTHER=0

echo "Running cppcheck..." >> "$LOG_FILE"

# Use text output (not XML) for easier POSIX parsing
if cppcheck --enable=all \
            --suppress=missingIncludeSystem \
            --suppress=unusedFunction \
            $SOURCE_FILES \
            >> "$CPPCHECK_LOG" 2>&1; then
  CPPCHECK_EXIT=0
else
  CPPCHECK_EXIT=$?
fi

# Parse cppcheck output (count by severity)
# Ensure variables are numeric and default to 0 if empty
CPPCHECK_ERRORS=$(grep -c "error:" "$CPPCHECK_LOG" 2>/dev/null || echo "0")
CPPCHECK_ERRORS=${CPPCHECK_ERRORS:-0}
[ -z "$CPPCHECK_ERRORS" ] && CPPCHECK_ERRORS=0

CPPCHECK_WARNINGS=$(grep -c "warning:" "$CPPCHECK_LOG" 2>/dev/null || echo "0")
CPPCHECK_WARNINGS=${CPPCHECK_WARNINGS:-0}
[ -z "$CPPCHECK_WARNINGS" ] && CPPCHECK_WARNINGS=0

CPPCHECK_STYLE=$(grep -c "style:" "$CPPCHECK_LOG" 2>/dev/null || echo "0")
CPPCHECK_STYLE=${CPPCHECK_STYLE:-0}
[ -z "$CPPCHECK_STYLE" ] && CPPCHECK_STYLE=0

CPPCHECK_OTHER=$(grep -cE "(performance|portability|information):" "$CPPCHECK_LOG" 2>/dev/null || echo "0")
CPPCHECK_OTHER=${CPPCHECK_OTHER:-0}
[ -z "$CPPCHECK_OTHER" ] && CPPCHECK_OTHER=0

# Run clang-tidy (optional)
CLANG_TIDY_LOG="$TASK_DIR/clang_tidy.log"
CLANG_TIDY_ERRORS=0
CLANG_TIDY_WARNINGS=0

if [ $HAS_CLANG_TIDY -eq 1 ] && [ $HAS_COMPILE_COMMANDS -eq 1 ]; then
  echo "Running clang-tidy..." >> "$LOG_FILE"
  
  # Find compile_commands.json
  COMPILE_COMMANDS=""
  if [ -f "compile_commands.json" ]; then
    COMPILE_COMMANDS="compile_commands.json"
  elif [ -f "build/compile_commands.json" ]; then
    COMPILE_COMMANDS="build/compile_commands.json"
  fi
  
  if [ -n "$COMPILE_COMMANDS" ]; then
    # Run clang-tidy on each source file
    for file in $SOURCE_FILES; do
      if [ -f "$file" ]; then
        clang-tidy -p "$(dirname "$COMPILE_COMMANDS")" "$file" >> "$CLANG_TIDY_LOG" 2>&1 || true
      fi
    done
    
    # Parse clang-tidy output
    # Ensure variables are numeric and default to 0 if empty
    CLANG_TIDY_ERRORS=$(grep -c "error:" "$CLANG_TIDY_LOG" 2>/dev/null || echo "0")
    CLANG_TIDY_ERRORS=${CLANG_TIDY_ERRORS:-0}
    [ -z "$CLANG_TIDY_ERRORS" ] && CLANG_TIDY_ERRORS=0
    
    CLANG_TIDY_WARNINGS=$(grep -c "warning:" "$CLANG_TIDY_LOG" 2>/dev/null || echo "0")
    CLANG_TIDY_WARNINGS=${CLANG_TIDY_WARNINGS:-0}
    [ -z "$CLANG_TIDY_WARNINGS" ] && CLANG_TIDY_WARNINGS=0
  fi
else
  echo "Skipping clang-tidy (not available or compile_commands.json missing)" >> "$LOG_FILE"
fi

# Ensure both error variables are numeric before arithmetic
# Use parameter expansion and explicit integer conversion
CPPCHECK_ERRORS=${CPPCHECK_ERRORS:-0}
[ -z "$CPPCHECK_ERRORS" ] && CPPCHECK_ERRORS=0
# Convert to integer (strip any whitespace/newlines)
CPPCHECK_ERRORS=$(echo "$CPPCHECK_ERRORS" | tr -d '[:space:]')
CPPCHECK_ERRORS=${CPPCHECK_ERRORS:-0}

CLANG_TIDY_ERRORS=${CLANG_TIDY_ERRORS:-0}
[ -z "$CLANG_TIDY_ERRORS" ] && CLANG_TIDY_ERRORS=0
# Convert to integer (strip any whitespace/newlines)
CLANG_TIDY_ERRORS=$(echo "$CLANG_TIDY_ERRORS" | tr -d '[:space:]')
CLANG_TIDY_ERRORS=${CLANG_TIDY_ERRORS:-0}

# Determine status with safe arithmetic
TOTAL_ERRORS=$((CPPCHECK_ERRORS + CLANG_TIDY_ERRORS))
STATUS="✅ PASS"
EXIT_CODE=0

if [ $TOTAL_ERRORS -gt 0 ]; then
  STATUS="🔴 FAIL"
  EXIT_CODE=1
fi

# Generate report
{
  echo "# Static Analysis Report"
  echo ""
  echo "## Summary"
  echo "- Source files analyzed: $(echo "$SOURCE_FILES" | wc -w | tr -d ' ')"
  echo "- Scope: src/palantir/*, src/transport/*, src/analysis/RemoteExecutor*"
  echo ""
  echo "## cppcheck Results"
  echo "- Errors: $CPPCHECK_ERRORS"
  echo "- Warnings: $CPPCHECK_WARNINGS"
  echo "- Style issues: $CPPCHECK_STYLE"
  echo "- Other issues: $CPPCHECK_OTHER"
  echo ""
  
  if [ $HAS_CLANG_TIDY -eq 1 ] && [ $HAS_COMPILE_COMMANDS -eq 1 ]; then
    echo "## clang-tidy Results"
    echo "- Errors: $CLANG_TIDY_ERRORS"
    echo "- Warnings: $CLANG_TIDY_WARNINGS"
    echo ""
  else
    echo "## clang-tidy"
    echo "- Not run (tool or compile_commands.json not available)"
    echo ""
  fi
  
  echo "## Status: $STATUS"
  echo ""
  
  if [ $EXIT_CODE -eq 1 ]; then
    echo "Error-level issues detected. See logs for details."
  else
    echo "No error-level issues detected."
    # Ensure warning variables are numeric before arithmetic
    CPPCHECK_WARNINGS=${CPPCHECK_WARNINGS:-0}
    [ -z "$CPPCHECK_WARNINGS" ] && CPPCHECK_WARNINGS=0
    CPPCHECK_WARNINGS=$(echo "$CPPCHECK_WARNINGS" | tr -d '[:space:]')
    CPPCHECK_WARNINGS=${CPPCHECK_WARNINGS:-0}
    
    CLANG_TIDY_WARNINGS=${CLANG_TIDY_WARNINGS:-0}
    [ -z "$CLANG_TIDY_WARNINGS" ] && CLANG_TIDY_WARNINGS=0
    CLANG_TIDY_WARNINGS=$(echo "$CLANG_TIDY_WARNINGS" | tr -d '[:space:]')
    CLANG_TIDY_WARNINGS=${CLANG_TIDY_WARNINGS:-0}
    
    if [ $((CPPCHECK_WARNINGS + CLANG_TIDY_WARNINGS)) -gt 0 ]; then
      echo ""
      echo "Note: Some warnings and style issues found (not counted as failures)."
    fi
  fi
  
  echo ""
  echo "---"
  echo "cppcheck log: [cppcheck.log](cppcheck.log)"
  if [ $HAS_CLANG_TIDY -eq 1 ] && [ $HAS_COMPILE_COMMANDS -eq 1 ]; then
    echo "clang-tidy log: [clang_tidy.log](clang_tidy.log)"
  fi
  echo "Full log: [static_analysis.log](static_analysis.log)"
} > "$REPORT_FILE"

exit $EXIT_CODE

