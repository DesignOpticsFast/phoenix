#!/usr/bin/env bash
# Soak Test - Run Palantir integration tests repeatedly to check stability
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

LOG_FILE="$TASK_DIR/soak.log"
REPORT_FILE="$TASK_DIR/soak_report.md"

# Iteration precedence: SOAK_ITERATIONS env → $2 positional → default 500
ITERATIONS="${SOAK_ITERATIONS:-${2:-500}}"

# Validate iterations is a number
if ! [ "$ITERATIONS" -eq "$ITERATIONS" ] 2>/dev/null; then
  echo "Error: SOAK_ITERATIONS must be a number" >&2
  exit 1
fi

# Failure thresholds
FAILURE_RATE_THRESHOLD=5  # 5% failure rate triggers FAIL
TIMEOUT_SECONDS=60        # Per-iteration timeout

{
  echo "=== Soak Test ==="
  echo "Iterations: $ITERATIONS"
  echo "Failure rate threshold: ${FAILURE_RATE_THRESHOLD}%"
  echo "Per-iteration timeout: ${TIMEOUT_SECONDS}s"
  echo "Start: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
} > "$LOG_FILE"

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

# Run iterations
PASSED=0
FAILED=0
TIMED_OUT=0
ITERATION_FAILURES=""
ITERATION_TIMEOUTS=""

for i in $(seq 1 "$ITERATIONS"); do
  echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] Iteration $i/$ITERATIONS" >> "$LOG_FILE"
  
  # Run with timeout
  set +e
  if "$TIMEOUT_BIN" "$TIMEOUT_SECONDS" "$TEST_BINARY" >> "$LOG_FILE" 2>&1; then
    PASSED=$((PASSED + 1))
  else
    EXIT_CODE=$?
    FAILED=$((FAILED + 1))
    ITERATION_FAILURES="${ITERATION_FAILURES} $i"
    
    # Check if it was a timeout (exit code 124 from timeout command)
    if [ $EXIT_CODE -eq 124 ]; then
      TIMED_OUT=$((TIMED_OUT + 1))
      ITERATION_TIMEOUTS="${ITERATION_TIMEOUTS} $i"
      echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] Iteration $i TIMED OUT" >> "$LOG_FILE"
    else
      echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] Iteration $i FAILED (exit code $EXIT_CODE)" >> "$LOG_FILE"
    fi
  fi
  set -e
done

# Calculate failure rate
TOTAL=$ITERATIONS
if [ $TOTAL -gt 0 ]; then
  FAILURE_RATE=$(awk -v failed="$FAILED" -v total="$TOTAL" 'BEGIN{printf "%.2f", (failed/total)*100}')
else
  FAILURE_RATE=0
fi

# Determine status
STATUS="✅ PASS"
EXIT_CODE=0

if [ $TIMED_OUT -gt 0 ]; then
  STATUS="🔴 FAIL"
  EXIT_CODE=1
elif [ "$(echo "$FAILURE_RATE $FAILURE_RATE_THRESHOLD" | awk '{if ($1 > $2) print 1; else print 0}')" = "1" ]; then
  STATUS="🔴 FAIL"
  EXIT_CODE=1
fi

# Generate report
{
  echo "# Soak Test Report"
  echo ""
  echo "## Summary"
  echo "- Iterations: $ITERATIONS"
  echo "- Passed: $PASSED"
  echo "- Failed: $FAILED"
  echo "- Timed out: $TIMED_OUT"
  echo "- Failure rate: ${FAILURE_RATE}%"
  echo "- Threshold: ${FAILURE_RATE_THRESHOLD}%"
  echo ""
  echo "## Status: $STATUS"
  echo ""
  
  if [ $EXIT_CODE -eq 1 ]; then
    echo "### Failure Criteria"
    echo ""
    if [ $TIMED_OUT -gt 0 ]; then
      echo "- **Any timeout triggers FAIL** (found $TIMED_OUT timeout(s))"
    fi
    if [ "$(echo "$FAILURE_RATE $FAILURE_RATE_THRESHOLD" | awk '{if ($1 > $2) print 1; else print 0}')" = "1" ]; then
      echo "- **Failure rate ${FAILURE_RATE}% exceeds threshold ${FAILURE_RATE_THRESHOLD}%**"
    fi
    echo ""
    if [ -n "$ITERATION_TIMEOUTS" ]; then
      echo "Timed out iterations:$ITERATION_TIMEOUTS"
      echo ""
    fi
    if [ -n "$ITERATION_FAILURES" ]; then
      echo "Failed iterations:$ITERATION_FAILURES"
    fi
  else
    echo "All iterations passed within failure rate threshold."
  fi
  
  echo ""
  echo "---"
  echo "Full log: [soak.log](soak.log)"
} > "$REPORT_FILE"

exit $EXIT_CODE

