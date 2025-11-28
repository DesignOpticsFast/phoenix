#!/usr/bin/env bash
# TSAN Stress Test - Run Palantir integration tests with ThreadSanitizer
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

LOG_FILE="$TASK_DIR/tsan.log"
REPORT_FILE="$TASK_DIR/tsan_report.md"

# Iteration precedence: TSAN_ITERATIONS env → $2 positional → default 100
ITERATIONS="${TSAN_ITERATIONS:-${2:-100}}"

# Validate iterations is a number
if ! [ "$ITERATIONS" -eq "$ITERATIONS" ] 2>/dev/null; then
  echo "Error: TSAN_ITERATIONS must be a number" >&2
  exit 1
fi

{
  echo "=== TSAN Stress Test ==="
  echo "Iterations: $ITERATIONS"
  echo "Start: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
} > "$LOG_FILE"

# Find TSAN build directory
BUILD_DIR=""
for dir in "build_tsan" "build/tsan"; do
  if [ -d "$dir" ]; then
    BUILD_DIR="$dir"
    break
  fi
done

if [ -z "$BUILD_DIR" ]; then
  {
    echo "⚠️ SKIPPED: TSAN build directory not found"
    echo "Searched for: build_tsan/, build/tsan/"
    echo "Reason: TSAN-enabled build not available"
  } | tee -a "$LOG_FILE" > "$REPORT_FILE"
  exit 2
fi

# Find test binary (palantir_integration_tests)
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

# Set TSAN options
export TSAN_OPTIONS="halt_on_error=1"

# Run iterations
PASSED=0
FAILED=0
DATA_RACES=0
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

# Parse log for TSAN errors (POSIX grep, no -P)
DATA_RACES=$(grep -c "WARNING: ThreadSanitizer:" "$LOG_FILE" 2>/dev/null || echo "0")
SUMMARY_LINES=$(grep -c "SUMMARY: ThreadSanitizer:" "$LOG_FILE" 2>/dev/null || echo "0")

# Generate report
{
  echo "# TSAN Stress Test Report"
  echo ""
  echo "## Summary"
  echo "- Iterations: $ITERATIONS"
  echo "- Passed: $PASSED"
  echo "- Failed: $FAILED"
  echo "- Data races detected: $DATA_RACES"
  echo "- TSAN summary lines: $SUMMARY_LINES"
  echo ""
  
  if [ "$DATA_RACES" -gt 0 ] || [ "$SUMMARY_LINES" -gt 0 ]; then
    echo "## Status: 🔴 FAIL"
    echo ""
    echo "Data races or TSAN errors detected. See tsan.log for details."
    if [ -n "$ITERATION_FAILURES" ]; then
      echo ""
      echo "Failed iterations:$ITERATION_FAILURES"
    fi
  elif [ "$FAILED" -gt 0 ]; then
    echo "## Status: 🔴 FAIL"
    echo ""
    echo "Test failures detected (no data races). See tsan.log for details."
    echo ""
    echo "Failed iterations:$ITERATION_FAILURES"
  else
    echo "## Status: ✅ PASS"
    echo ""
    echo "All iterations passed with no data races detected."
  fi
  
  echo ""
  echo "---"
  echo "Full log: [tsan.log](tsan.log)"
} > "$REPORT_FILE"

# Exit code
if [ "$DATA_RACES" -gt 0 ] || [ "$SUMMARY_LINES" -gt 0 ] || [ "$FAILED" -gt 0 ]; then
  exit 1
else
  exit 0
fi

