# Preflight Performance Profile (Sprint 4.3)

_Generated: 2025-11-20T09:28:48-08:00_  
_Chunk: S4.3-Preflight-Probe-1_

**Note:** Preflight is no longer part of `make daily`; it must be run explicitly when needed (e.g., `scripts/dev01-preflight.sh`).

## Executive Summary

The preflight ritual takes **~4 minutes** to complete, with the breakdown:
- **Configure:** 2 seconds
- **Build:** 152 seconds (~2.5 minutes)
- **Tests:** 81 seconds (~1.4 minutes)
- **Total:** ~235 seconds

**Key Finding:** The preflight script creates a **new build directory every time** (`build-preflight-$$`), forcing a full clean build on every `make daily` run. This is the primary bottleneck.

---

## Phase-by-Phase Timing Breakdown

### Phase 1: CMake Configure
- **Time:** 2 seconds
- **Status:** ✅ Efficient
- **Details:** Standard CMake configuration, no issues detected

### Phase 2: Build
- **Time:** 152 seconds (~2.5 minutes)
- **Status:** ⚠️ Full clean build every time
- **Details:**
  - Builds ALL targets (`--target all`)
  - Uses parallel builds (`-j $(nproc)`)
  - Build type: `Release`
  - **Problem:** Creates new build directory (`build-preflight-$$`) every run, so no incremental builds possible

### Phase 3: Tests
- **Time:** 81.25 seconds (~1.4 minutes)
- **Status:** ⚠️ One test hits timeout
- **Total Tests:** 21 tests
- **Pass Rate:** 81% (17 passed, 4 failed)
- **Details:**
  - Test timeout set to 60 seconds per test
  - One test (`test_analysiswindow_autorun`) hits the 60s timeout
  - Tests run every time, even if build didn't change

---

## Slowest Tests

| Test Name | Time | Status | Notes |
|-----------|------|--------|-------|
| `test_analysiswindow_autorun` | **60.06s** | ⏱️ **TIMEOUT** | Hits 60s timeout limit |
| `test_analysis_timeout` | 9.55s | ❌ Failed | Known failure (Sprint 4.3) |
| `test_async_architecture` | 5.68s | ❌ Failed | |
| `test_local_xysine` | 5.08s | ❌ Failed | Known failure (Mac only, Sprint 4.3) |
| `test_analysis_cancel` | 0.29s | ✅ Passed | |

**Note:** The timeout test (`test_analysiswindow_autorun`) is consuming 60+ seconds, which is 74% of total test time.

---

## Root Cause Analysis

### Primary Bottleneck: Full Clean Build Every Time

**Location:** `.underlord/preflight.sh` line 143
```bash
BUILD_DIR="build-preflight-$$"
```

**Impact:**
- Every `make daily` run creates a new build directory
- CMake must reconfigure from scratch
- All source files must be recompiled (no incremental builds)
- Build artifacts are deleted at the end (line 196: `rm -rf "$BUILD_DIR"`)

**Why This Happens:**
- The script uses `$$` (process ID) to create unique build directories
- No check for existing builds
- No reuse of previous build artifacts

### Secondary Bottleneck: Slow/Hanging Tests

**Problem Tests:**
1. `test_analysiswindow_autorun` - Hits 60s timeout (likely waiting on QML/UI events)
2. `test_analysis_timeout` - Takes 9.55s (known failure, but still runs)
3. `test_async_architecture` - Takes 5.68s (failed, but still runs)

**Impact:**
- Tests run every time, even if code didn't change
- Failed tests still consume time
- Timeout tests consume maximum time (60s) even if they would eventually pass

---

## Hypotheses About Recent Slowdown

### What Changed Recently?
1. **New unified daily ritual** - Added full preflight check to `make daily`
2. **New test additions** - `test_xyplot_autoscale` and other tests added
3. **QML loading issues** - `test_analysiswindow_autorun` shows QML load failures, suggesting UI test complexity increased

### Why It Feels Slower Now
- **Before:** Preflight may have been optional or run less frequently
- **Now:** Every `make daily` runs full preflight (configure + build + tests)
- **Impact:** 4+ minutes added to every daily ritual run

---

## Low-Hanging Fruit Fixes

### Fix 1: Reuse Build Directory (High Impact)
**Current:** Creates new `build-preflight-$$` every time  
**Proposed:** Use a fixed `build-preflight` directory and reuse it
- **Benefit:** Incremental builds (first run: 152s, subsequent runs: ~10-30s if nothing changed)
- **Risk:** Low (can still clean if needed)
- **Effort:** Low (change one line)

### Fix 2: Skip Tests for Fast Preflight Mode (Medium Impact)
**Current:** Always runs all 21 tests  
**Proposed:** Add `SKIP_TESTS=1` environment variable option
- **Benefit:** Save 81 seconds when tests aren't needed
- **Risk:** Low (opt-in via environment variable)
- **Effort:** Low (add conditional check)

### Fix 3: Exclude Known-Failing Tests from Preflight (Medium Impact)
**Current:** Runs all tests, including known failures  
**Proposed:** Use `ctest --exclude-regex` to skip known failures
- **Benefit:** Save ~15 seconds (9.55s + 5.08s for known failures)
- **Risk:** Low (tests still run in CI)
- **Effort:** Low (add exclusion list)

### Fix 4: Reduce Timeout for UI Tests (Low Impact)
**Current:** 60s timeout per test  
**Proposed:** Reduce timeout to 30s for UI tests, or mark them as "long-running"
- **Benefit:** Fail faster if test is actually hanging
- **Risk:** Medium (might cause false failures)
- **Effort:** Medium (need to identify UI tests)

### Fix 5: Parallel Test Execution (Low Impact)
**Current:** Tests run sequentially  
**Proposed:** Use `ctest --parallel` (already supported)
- **Benefit:** Faster test execution if tests are independent
- **Risk:** Low (ctest handles parallelization)
- **Effort:** Low (add `--parallel` flag)

---

## Recommended Follow-Up Changes

### Option 1: Fast Preflight Mode (Recommended)
**Goal:** Reduce daily ritual time to <30 seconds when nothing changed

**Changes:**
1. Use fixed `build-preflight` directory (reuse across runs)
2. Add `SKIP_TESTS=1` option for fast mode
3. Only run tests if build actually changed

**Expected Impact:**
- First run: ~4 minutes (unchanged)
- Subsequent runs (no changes): ~10-30 seconds
- Subsequent runs (code changed): ~2-3 minutes (incremental build)

### Option 2: Separate Test Suite
**Goal:** Move slow/hanging tests to separate "extended" suite

**Changes:**
1. Create `test_extended` target for slow tests
2. Exclude `test_analysiswindow_autorun` from standard preflight
3. Run extended suite only in CI or on-demand

**Expected Impact:**
- Preflight tests: ~20 seconds (remove 60s timeout test)
- Extended suite: Run separately when needed

### Option 3: Smarter Build Detection
**Goal:** Only rebuild if source files changed

**Changes:**
1. Check git diff to see if relevant files changed
2. Skip build if no changes detected
3. Skip tests if build didn't run

**Expected Impact:**
- No changes: ~5 seconds (just checks)
- Code changed: ~2-3 minutes (incremental build + tests)

---

## Test Failure Analysis

### Known Failures (Expected)
- `test_analysis_timeout` - Known failure (Sprint 4.3)
- `test_local_xysine` - Known failure (Mac only, Sprint 4.3)
- `test_analysiswindow_autorun` - QML loading issues (likely environment-specific)

### Unknown Failures (Need Investigation)
- `test_async_architecture` - Failed but not in known failures list
  - **Action:** Add to `KNOWN_TEST_FAILURES.md` or fix

---

## Measurement Methodology

**Test Environment:**
- Host: dev-01 (EC2)
- Build Type: Release
- Parallel Build: Yes (`-j $(nproc)`)
- Test Timeout: 60 seconds per test

**Measurement Tools:**
- Manual timing with `date +%s`
- `ctest --output-on-failure --timeout 60`
- Build logs captured to `/tmp/preflight_*.log`

**Build Directory:**
- Created: `build-preflight-profile-19002`
- Preserved for inspection (not deleted)

---

## Next Steps

1. **Immediate:** Implement Option 1 (Fast Preflight Mode) - highest impact, lowest risk
2. **Short-term:** Investigate `test_async_architecture` failure
3. **Medium-term:** Fix QML loading issues in `test_analysiswindow_autorun`
4. **Long-term:** Consider separate test suites for fast vs. comprehensive testing

---

_End of Preflight Profile Report_

