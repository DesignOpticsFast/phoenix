# Overnight Autonomous QA Framework - Quick Start Guide

## Overview

The Overnight Autonomous QA Framework runs 6-8 hours of read-only QA analysis on Crucible (macOS) and Fulcrum (Linux), producing per-task reports and a single `OVERNIGHT_SUMMARY.md` for human review.

**Key Principles:**
- **Read-only**: Never modifies source code, CI config, or installs dependencies
- **Adaptive monitoring**: Checks system health before each task, pauses if needed
- **Tool availability → SKIP**: Missing tools result in ⚠️ SKIPPED (not FAIL)
- **POSIX-compatible**: Works on both macOS (BSD) and Linux (GNU)

## Prerequisites

### Builds Required

The framework assumes the following builds already exist:

- **TSAN build**: `build_tsan/` (for ThreadSanitizer testing)
- **ASAN build**: `build_asan/` (for AddressSanitizer testing)
- **Coverage build**: `build_coverage/` (for coverage collection)
- **Normal build**: `build/` (for soak tests and Valgrind)

### Test Binary

The framework expects `palantir_integration_tests` in the appropriate build directories. If this binary doesn't exist, tasks will be marked as ⚠️ SKIPPED.

### External Tools

- **lcov + genhtml**: Required for coverage collection
- **cppcheck**: Required for static analysis
- **clang-tidy**: Optional (requires `compile_commands.json`)
- **valgrind**: Linux only (automatically skipped on macOS)

## Usage

**Note:** All commands assume you are running from the `phoenix/` directory. If running from the workspace root, prefix paths with `phoenix/`.

### Basic Run

```bash
./scripts/overnight/run_overnight_qa.sh
```

This will:
1. Create a report directory: `qa_reports_YYYYMMDD_HHMMSS/` (under `phoenix/`)
2. Run all tasks in phases:
   - **Phase 1 (Critical Safety)**: TSAN → ASAN → Valgrind
   - **Phase 2 (Stability)**: Soak test
   - **Phase 3 (Quality)**: Coverage → Static analysis
3. Generate `OVERNIGHT_SUMMARY.md` in the report directory

### Customizing Iterations

You can override default iteration counts using environment variables:

```bash
# Run with custom iteration counts
TSAN_ITERATIONS=50 ASAN_ITERATIONS=25 SOAK_ITERATIONS=100 \
  ./scripts/overnight/run_overnight_qa.sh
```

**Default iterations:**
- TSAN: 100
- ASAN: 50
- Soak: 500

### Running Individual Tasks

Each task script can be run independently:

```bash
# Run TSAN stress test only
./scripts/overnight/tsan_stress.sh /path/to/task/dir [iterations]

# Run coverage report only
./scripts/overnight/coverage_report.sh /path/to/task/dir
```

## Task Execution Order

The framework runs tasks in a specific order:

### Phase 1: Critical Safety Checks
1. **TSAN Stress** (`tsan_stress.sh`)
   - Runs Palantir integration tests with ThreadSanitizer
   - Detects data races
   - Default: 100 iterations

2. **ASAN Stress** (`asan_stress.sh`)
   - Runs Palantir integration tests with AddressSanitizer
   - Detects memory errors and leaks
   - Default: 50 iterations

3. **Valgrind Check** (`valgrind_check.sh`)
   - Runs tests under Valgrind memcheck (Linux only)
   - Detects memory leaks and errors
   - Automatically skipped on macOS

### Phase 2: Stability
4. **Soak Test** (`soak_test.sh`)
   - Runs Palantir integration tests repeatedly
   - Checks for stability and memory leaks over time
   - Default: 500 iterations
   - FAIL criteria: Any timeout or >5% failure rate

### Phase 3: Quality Metrics
5. **Coverage Report** (`coverage_report.sh`)
   - Collects code coverage using lcov + genhtml
   - Generates HTML report
   - FAIL if Palantir coverage < 80%

6. **Static Analysis** (`static_analysis.sh`)
   - Runs cppcheck on Palantir/transport/RemoteExecutor code
   - Optionally runs clang-tidy if available
   - FAIL if error-level issues found

## Report Structure

Each run creates a report directory with the following structure:

```
phoenix/qa_reports_YYYYMMDD_HHMMSS/
├── master.log                    # Master log with all events
├── OVERNIGHT_SUMMARY.md          # Executive summary
├── tsan/
│   ├── tsan.log
│   └── tsan_report.md
├── asan/
│   ├── asan.log
│   └── asan_report.md
├── valgrind/
│   ├── valgrind.log
│   └── valgrind_report.md
├── soak/
│   ├── soak.log
│   └── soak_report.md
├── coverage/
│   ├── coverage.log
│   ├── coverage_report.md
│   ├── coverage.info
│   └── coverage_html/            # HTML coverage report
└── static_analysis/
    ├── static_analysis.log
    ├── cppcheck.log
    ├── clang_tidy.log (if run)
    └── static_analysis_report.md
```

## Task Status Semantics

Each task reports one of three statuses:

- **✅ PASS**: Task ran successfully, criteria met
- **🔴 FAIL**: Task ran, but criteria not met (action required)
- **⚠️ SKIPPED**: Task not run (tool/platform limitation)

## Health Monitoring

Before each task, the framework checks:
- CPU temperature (if available)
- System load average
- Available memory
- Disk space

If the system is unhealthy:
- Framework pauses for 60 seconds
- Retries up to 3 times
- Logs warnings but continues (never aborts entire run)

## Exit Codes

- **0**: All tasks passed or were skipped (no failures)
- **1**: One or more tasks failed

## Examples

### Quick Smoke Test

Run with small iteration counts for testing:

```bash
TSAN_ITERATIONS=5 ASAN_ITERATIONS=3 SOAK_ITERATIONS=10 \
  ./scripts/overnight/run_overnight_qa.sh
```

### Full Overnight Run

Run with default iterations (6-8 hours):

```bash
# Run overnight (start before leaving)
nohup ./scripts/overnight/run_overnight_qa.sh > overnight_run.log 2>&1 &

# Check progress
tail -f overnight_run.log

# View summary when complete
cat phoenix/qa_reports_*/OVERNIGHT_SUMMARY.md
```

## Troubleshooting

### Task Skipped: Build Not Found

**Problem**: Task reports ⚠️ SKIPPED because build directory not found.

**Solution**: Ensure the required build exists:
- TSAN: `build_tsan/`
- ASAN: `build_asan/`
- Coverage: `build_coverage/`

### Task Skipped: Test Binary Not Found

**Problem**: Task reports ⚠️ SKIPPED because `palantir_integration_tests` not found.

**Solution**: Build the test binary in the appropriate build directory.

### Coverage Below Target

**Problem**: Coverage report shows 🔴 FAIL with coverage < 80%.

**Solution**: Review coverage report to identify uncovered code paths. Add tests to increase coverage.

### Health Check Warnings

**Problem**: Master log shows health check warnings.

**Solution**: These are informational. The framework will pause and retry automatically. If warnings persist, check system resources manually.

## Reference

For detailed behavioral requirements and constraints, see:
- `phoenix/docs/qa/OVERNIGHT_QA_CONTROL.md` (authoritative control document)

