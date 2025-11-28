#!/usr/bin/env bash
# ASAN Stress Test - Run Palantir integration tests with AddressSanitizer
# Exit codes: 0=PASS, 1=FAIL, 2=SKIPPED

set +e  # CRITICAL: Never use set -e
set -u
set -o pipefail

# Task report directory (passed as first argument)
TASK_DIR="${1:-}"

# Detect timeout binary (timeout or gtimeout)
TIMEOUT_BIN=$(command -v timeout || command -v gtimeout || echo "")

if [ -z "$TASK_DIR" ]; then
  echo "Error: task directory not provided" >&2
  exit 1
fi

LOG_FILE="$TASK_DIR/asan.log"
REPORT_FILE="$TASK_DIR/asan_report.md"

# Iteration precedence: ASAN_ITERATIONS env → $2 positional → default 50
ITERATIONS="${ASAN_ITERATIONS:-${2:-50}}"

# Validate iterations is a number
if ! [ "$ITERATIONS" -eq "$ITERATIONS" ] 2>/dev/null; then
  echo "Error: ASAN_ITERATIONS must be a number" >&2
  exit 1
fi

{
  echo "=== ASAN Stress Test ==="
  echo "Iterations: $ITERATIONS"
  echo "Start: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
} > "$LOG_FILE"

# Find ASAN build directory
BUILD_DIR=""
for dir in "build_asan" "build/asan"; do
  if [ -d "$dir" ]; then
    BUILD_DIR="$dir"
    break
  fi
done

if [ -z "$BUILD_DIR" ]; then
  {
    echo "⚠️ SKIPPED: ASAN build directory not found"
    echo "Searched for: build_asan/, build/asan/"
    echo "Reason: ASAN-enabled build not available"
  } | tee -a "$LOG_FILE" > "$REPORT_FILE"
  exit 2
fi

# Find test binary
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

# Check for timeout binary
if [ -z "$TIMEOUT_BIN" ]; then
  {
    echo "⚠️ SKIPPED: timeout command not found"
    echo "Searched for: timeout, gtimeout"
    echo "Reason: timeout tool not available (install coreutils for gtimeout on macOS)"
  } | tee -a "$LOG_FILE" > "$REPORT_FILE"
  exit 2
fi

echo "Using test binary: $TEST_BINARY" >> "$LOG_FILE"
echo "Using timeout: $TIMEOUT_BIN" >> "$LOG_FILE"

# Set ASAN options (avoid protobuf false positives)
export ASAN_OPTIONS="detect_container_overflow=0"

# Run iterations
PASSED=0
FAILED=0
ASAN_ERRORS=0
LEAKS=0
ITERATION_FAILURES=""

for i in $(seq 1 "$ITERATIONS"); do
  echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] Iteration $i/$ITERATIONS" >> "$LOG_FILE"
  
  # Run with timeout (5 minutes per iteration)
  if "$TIMEOUT_BIN" 300 "$TEST_BINARY" >> "$LOG_FILE" 2>&1; then
    PASSED=$((PASSED + 1))
  else
    FAILED=$((FAILED + 1))
    ITERATION_FAILURES="${ITERATION_FAILURES} $i"
    echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] Iteration $i FAILED" >> "$LOG_FILE"
  fi
done

# Parse log for ASAN errors (POSIX grep, no -P)
ASAN_ERRORS=$(grep -c "ERROR: AddressSanitizer:" "$LOG_FILE" 2>/dev/null || echo "0")
SUMMARY_LINES=$(grep -c "SUMMARY: AddressSanitizer:" "$LOG_FILE" 2>/dev/null || echo "0")
LEAKS=$(grep -c "LeakSanitizer:" "$LOG_FILE" 2>/dev/null || echo "0")

# Categorize errors
USE_AFTER_FREE=$(grep -c "use-after-free" "$LOG_FILE" 2>/dev/null || echo "0")
HEAP_BUFFER_OVERFLOW=$(grep -c "heap-buffer-overflow" "$LOG_FILE" 2>/dev/null || echo "0")
STACK_BUFFER_OVERFLOW=$(grep -c "stack-buffer-overflow" "$LOG_FILE" 2>/dev/null || echo "0")

# Generate report
{
  echo "# ASAN Stress Test Report"
  echo ""
  echo "## Summary"
  echo "- Iterations: $ITERATIONS"
  echo "- Passed: $PASSED"
  echo "- Failed: $FAILED"
  echo "- ASAN errors detected: $ASAN_ERRORS"
  echo "- Memory leaks detected: $LEAKS"
  echo ""
  
  if [ "$ASAN_ERRORS" -gt 0 ] || [ "$SUMMARY_LINES" -gt 0 ] || [ "$LEAKS" -gt 0 ]; then
    echo "## Status: 🔴 FAIL"
    echo ""
    echo "ASAN errors or leaks detected. See asan.log for details."
    echo ""
    echo "Error breakdown:"
    echo "- Use-after-free: $USE_AFTER_FREE"
    echo "- Heap buffer overflow: $HEAP_BUFFER_OVERFLOW"
    echo "- Stack buffer overflow: $STACK_BUFFER_OVERFLOW"
    echo "- Memory leaks: $LEAKS"
    if [ -n "$ITERATION_FAILURES" ]; then
      echo ""
      echo "Failed iterations:$ITERATION_FAILURES"
    fi
  elif [ "$FAILED" -gt 0 ]; then
    echo "## Status: 🔴 FAIL"
    echo ""
    echo "Test failures detected (no ASAN errors). See asan.log for details."
    echo ""
    echo "Failed iterations:$ITERATION_FAILURES"
  else
    echo "## Status: ✅ PASS"
    echo ""
    echo "All iterations passed with no ASAN errors or leaks detected."
  fi
  
  echo ""
  echo "---"
  echo "Full log: [asan.log](asan.log)"
} > "$REPORT_FILE"

# Exit code
if [ "$ASAN_ERRORS" -gt 0 ] || [ "$SUMMARY_LINES" -gt 0 ] || [ "$LEAKS" -gt 0 ] || [ "$FAILED" -gt 0 ]; then
  exit 1
else
  exit 0
fi

