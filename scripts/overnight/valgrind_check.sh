#!/usr/bin/env bash
# Valgrind Check - Run Palantir integration tests under Valgrind memcheck
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

LOG_FILE="$TASK_DIR/valgrind.log"
REPORT_FILE="$TASK_DIR/valgrind_report.md"

{
  echo "=== Valgrind Check ==="
  echo "Start: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
} > "$LOG_FILE"

# Platform detection
PLATFORM=$(uname -s | tr '[:upper:]' '[:lower:]')

# Check platform - Valgrind not available on macOS
if [ "$PLATFORM" = "darwin" ]; then
  {
    echo "⚠️ SKIPPED: Valgrind not available on macOS"
    echo "Reason: Valgrind does not support macOS"
  } | tee -a "$LOG_FILE" > "$REPORT_FILE"
  exit 2
fi

# Check for valgrind in PATH
if ! command -v valgrind >/dev/null 2>&1; then
  {
    echo "⚠️ SKIPPED: valgrind not found in PATH"
    echo "Reason: valgrind tool not available"
  } | tee -a "$LOG_FILE" > "$REPORT_FILE"
  exit 2
fi

# Find test binary in normal build directory
BUILD_DIR="build"
TEST_BINARY=""

for binary in "$BUILD_DIR/palantir_integration_tests" \
              "$BUILD_DIR/tests/palantir_integration_tests" \
              "$BUILD_DIR/tests/envelope_helpers_test"; do
  if [ -x "$binary" ]; then
    TEST_BINARY="$binary"
    break
  fi
done

if [ -z "$TEST_BINARY" ]; then
  {
    echo "⚠️ SKIPPED: Test binary not found in $BUILD_DIR"
    echo "Searched for: palantir_integration_tests, envelope_helpers_test"
    echo "Reason: Test binary not available"
  } | tee -a "$LOG_FILE" > "$REPORT_FILE"
  exit 2
fi

echo "Using test binary: $TEST_BINARY" >> "$LOG_FILE"
echo "Valgrind version: $(valgrind --version)" >> "$LOG_FILE"

# Run Valgrind memcheck
# --error-exitcode=1: exit with 1 if errors found
# --leak-check=full: detailed leak checking
# --show-leak-kinds=all: show all leak kinds
if valgrind --tool=memcheck \
            --leak-check=full \
            --show-leak-kinds=all \
            --error-exitcode=1 \
            --log-file="$LOG_FILE" \
            "$TEST_BINARY" >> "$LOG_FILE" 2>&1; then
  VALGRIND_EXIT=0
else
  VALGRIND_EXIT=$?
fi

# Parse Valgrind output (POSIX-compatible)
ERROR_COUNT=0
DEFINITELY_LOST=0
INDIRECTLY_LOST=0
POSSIBLY_LOST=0

# Extract error summary
if grep -q "ERROR SUMMARY:" "$LOG_FILE"; then
  ERROR_COUNT=$(grep "ERROR SUMMARY:" "$LOG_FILE" | awk '{print $4}' | head -1)
  if [ -z "$ERROR_COUNT" ]; then
    ERROR_COUNT=0
  fi
fi

# Extract leak information
if grep -q "definitely lost:" "$LOG_FILE"; then
  DEFINITELY_LOST=$(grep "definitely lost:" "$LOG_FILE" | awk '{print $4}' | head -1)
  if [ -z "$DEFINITELY_LOST" ]; then
    DEFINITELY_LOST=0
  fi
fi

if grep -q "indirectly lost:" "$LOG_FILE"; then
  INDIRECTLY_LOST=$(grep "indirectly lost:" "$LOG_FILE" | awk '{print $4}' | head -1)
  if [ -z "$INDIRECTLY_LOST" ]; then
    INDIRECTLY_LOST=0
  fi
fi

if grep -q "possibly lost:" "$LOG_FILE"; then
  POSSIBLY_LOST=$(grep "possibly lost:" "$LOG_FILE" | awk '{print $4}' | head -1)
  if [ -z "$POSSIBLY_LOST" ]; then
    POSSIBLY_LOST=0
  fi
fi

# Generate report
{
  echo "# Valgrind Check Report"
  echo ""
  echo "## Summary"
  echo "- Test binary: $TEST_BINARY"
  echo "- Valgrind version: $(valgrind --version)"
  echo "- Errors: $ERROR_COUNT"
  echo "- Definitely lost: $DEFINITELY_LOST bytes"
  echo "- Indirectly lost: $INDIRECTLY_LOST bytes"
  echo "- Possibly lost: $POSSIBLY_LOST bytes"
  echo ""
  
  if [ "$ERROR_COUNT" -gt 0 ] || [ "$DEFINITELY_LOST" -gt 0 ]; then
    echo "## Status: 🔴 FAIL"
    echo ""
    echo "Valgrind detected errors or memory leaks. See valgrind.log for details."
  else
    echo "## Status: ✅ PASS"
    echo ""
    echo "No errors or definite leaks detected by Valgrind."
    if [ "$POSSIBLY_LOST" -gt 0 ] || [ "$INDIRECTLY_LOST" -gt 0 ]; then
      echo ""
      echo "Note: Some indirect or possible leaks detected (not counted as failures)."
    fi
  fi
  
  echo ""
  echo "---"
  echo "Full log: [valgrind.log](valgrind.log)"
} > "$REPORT_FILE"

# Exit code
if [ "$ERROR_COUNT" -gt 0 ] || [ "$DEFINITELY_LOST" -gt 0 ]; then
  exit 1
else
  exit 0
fi

