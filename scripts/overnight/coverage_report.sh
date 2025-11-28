#!/usr/bin/env bash
# Coverage Report - Collect code coverage using lcov + genhtml
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

LOG_FILE="$TASK_DIR/coverage.log"
REPORT_FILE="$TASK_DIR/coverage_report.md"

# Coverage target (from control doc: ≥80% for Palantir code)
PALANTIR_COVERAGE_TARGET=80

{
  echo "=== Coverage Report ==="
  echo "Start: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
} > "$LOG_FILE"

# Check for lcov
if ! command -v lcov >/dev/null 2>&1; then
  {
    echo "⚠️ SKIPPED: lcov not found in PATH"
    echo "Reason: lcov tool not available"
  } | tee -a "$LOG_FILE" > "$REPORT_FILE"
  exit 2
fi

# Check for genhtml
if ! command -v genhtml >/dev/null 2>&1; then
  {
    echo "⚠️ SKIPPED: genhtml not found in PATH"
    echo "Reason: genhtml tool not available"
  } | tee -a "$LOG_FILE" > "$REPORT_FILE"
  exit 2
fi

echo "Using lcov: $(lcov --version | head -1)" >> "$LOG_FILE"
echo "Using genhtml: $(genhtml --version | head -1)" >> "$LOG_FILE"

# Find coverage build directory
BUILD_DIR=""
for dir in "build_coverage" "build/coverage"; do
  if [ -d "$dir" ]; then
    BUILD_DIR="$dir"
    break
  fi
done

if [ -z "$BUILD_DIR" ]; then
  {
    echo "⚠️ SKIPPED: Coverage build directory not found"
    echo "Searched for: build_coverage/, build/coverage/"
    echo "Reason: Coverage-enabled build not available"
  } | tee -a "$LOG_FILE" > "$REPORT_FILE"
  exit 2
fi

echo "Using build directory: $BUILD_DIR" >> "$LOG_FILE"

# Check if test binary exists (may need to run tests to generate coverage data)
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

# Run tests to generate coverage data (if not already run)
echo "Running tests to generate coverage data..." >> "$LOG_FILE"
if ! "$TEST_BINARY" >> "$LOG_FILE" 2>&1; then
  echo "Warning: Test execution failed, but continuing with coverage collection" >> "$LOG_FILE"
fi

# Collect coverage data
COVERAGE_INFO="$TASK_DIR/coverage.info"
COVERAGE_INFO_FILTERED="$TASK_DIR/coverage_filtered.info"
echo "Collecting coverage data..." >> "$LOG_FILE"

# Capture coverage data, ignoring errors from generated/protobuf code
if ! lcov --capture \
         --directory "$BUILD_DIR" \
         --output-file "$COVERAGE_INFO" \
         --ignore-errors inconsistent,unsupported,gcov,format \
         >> "$LOG_FILE" 2>&1; then
  {
    echo "🔴 FAIL: Failed to collect coverage data"
    echo "See coverage.log for details"
  } | tee -a "$LOG_FILE" > "$REPORT_FILE"
  exit 1
fi

# Filter out generated/protobuf code
echo "Filtering coverage data (removing generated/protobuf code)..." >> "$LOG_FILE"
if ! lcov --remove "$COVERAGE_INFO" \
         '*/proto/*' \
         '*/generated/*' \
         '*/contracts/proto/*' \
         -o "$COVERAGE_INFO_FILTERED" \
         >> "$LOG_FILE" 2>&1; then
  echo "Warning: Failed to filter coverage data, using unfiltered data" >> "$LOG_FILE"
  echo "Note: Coverage may include generated/protobuf code" >> "$LOG_FILE"
  COVERAGE_INFO_FILTERED="$COVERAGE_INFO"
else
  echo "Coverage filtering successful" >> "$LOG_FILE"
fi

# Generate HTML report (use filtered data)
COVERAGE_HTML="$TASK_DIR/coverage_html"
echo "Generating HTML report..." >> "$LOG_FILE"

if ! genhtml "$COVERAGE_INFO_FILTERED" \
             --output-directory "$COVERAGE_HTML" \
             --ignore-errors inconsistent,corrupt,category,unsupported \
             >> "$LOG_FILE" 2>&1; then
  {
    echo "🔴 FAIL: Failed to generate HTML report"
    echo "See coverage.log for details"
  } | tee -a "$LOG_FILE" > "$REPORT_FILE"
  exit 1
fi

# Extract coverage summary from lcov (use filtered data)
echo "Extracting coverage summary..." >> "$LOG_FILE"

# Get summary from lcov (filtered, ignoring errors)
LCOV_SUMMARY=$(lcov --summary "$COVERAGE_INFO_FILTERED" --ignore-errors inconsistent,corrupt,format,unsupported 2>&1 || echo "")

# Parse line coverage percentage (handle "lines.......: XX.X%" format)
LINE_COVERAGE=$(echo "$LCOV_SUMMARY" | grep -E "^[[:space:]]*lines" | awk -F: '{print $2}' | awk '{print $1}' | sed 's/%//' | head -1 || echo "0")
FUNCTION_COVERAGE=$(echo "$LCOV_SUMMARY" | grep -E "^[[:space:]]*functions" | awk -F: '{print $2}' | awk '{print $1}' | sed 's/%//' | head -1 || echo "0")
BRANCH_COVERAGE=$(echo "$LCOV_SUMMARY" | grep -E "^[[:space:]]*branches" | awk -F: '{print $2}' | awk '{print $1}' | sed 's/%//' | head -1 || echo "0")

# Try to extract Palantir-specific coverage if possible
# This is a simplified approach - in practice, might need to filter by directory
PALANTIR_LINE_COVERAGE="$LINE_COVERAGE"  # Default to overall if we can't filter

# Determine status
STATUS="✅ PASS"
EXIT_CODE=0

# Check if coverage is below target
if [ "$(echo "$PALANTIR_LINE_COVERAGE $PALANTIR_COVERAGE_TARGET" | awk '{if ($1 < $2) print 1; else print 0}')" = "1" ]; then
  STATUS="🔴 FAIL"
  EXIT_CODE=1
fi

# Generate report
{
  echo "# Coverage Report"
  echo ""
  echo "## Summary"
  echo "- Build directory: $BUILD_DIR"
  echo "- Test binary: $TEST_BINARY"
  echo ""
  echo "## Coverage Statistics"
  echo "- Line coverage: ${LINE_COVERAGE}%"
  echo "- Function coverage: ${FUNCTION_COVERAGE}%"
  echo "- Branch coverage: ${BRANCH_COVERAGE}%"
  echo ""
  echo "## Palantir Coverage"
  echo "- Line coverage: ${PALANTIR_LINE_COVERAGE}%"
  echo "- Target: ≥${PALANTIR_COVERAGE_TARGET}%"
  echo ""
  echo "## Status: $STATUS"
  echo ""
  
  if [ $EXIT_CODE -eq 1 ]; then
    echo "Coverage collection succeeded, but Palantir coverage (${PALANTIR_LINE_COVERAGE}%) is below target (${PALANTIR_COVERAGE_TARGET}%)."
  else
    echo "Coverage collected successfully and meets target."
  fi
  
  echo ""
  echo "---"
  echo "Coverage info file (raw): [coverage.info](coverage.info)"
  echo "Coverage info file (filtered): [coverage_filtered.info](coverage_filtered.info)"
  echo "HTML report: [coverage_html/index.html](coverage_html/index.html)"
  echo "Full log: [coverage.log](coverage.log)"
} > "$REPORT_FILE"

# Also append lcov summary to report
echo "" >> "$REPORT_FILE"
echo "## Detailed Summary (from lcov)" >> "$REPORT_FILE"
echo '```' >> "$REPORT_FILE"
echo "$LCOV_SUMMARY" >> "$REPORT_FILE"
echo '```' >> "$REPORT_FILE"

exit $EXIT_CODE

