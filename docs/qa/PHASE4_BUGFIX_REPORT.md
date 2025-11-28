# Phase 4: Orchestrator Bugfix & Re-Validation Report

**Date:** 2025-11-25  
**Validator:** UnderLord  
**Status:** ✅ **BUGFIXES COMPLETE - VALIDATION PASSED**

---

## Summary

Fixed two critical bugs in `scripts/overnight/run_overnight_qa.sh`:
1. **Exit code handling bug**: SKIPPED tasks (exit code 2) were incorrectly reported as PASS
2. **Stray "0" line**: Extra "0" appeared in Health Monitoring section

Both bugs are now fixed and verified via smoke test.

---

## Bug #1: Exit Code Handling & Status Mapping

### Problem

Tasks that exited with code 2 (SKIPPED) were being reported as ✅ PASS in the summary. The exit code capture logic was flawed.

### Root Cause

The original code used an `if !` construct that interfered with proper exit code capture:

```bash
# BEFORE (BUGGY)
local exit_code=0
if ! "$script_path" "$task_dir" >> "$MASTER_LOG" 2>&1; then
  exit_code=$?
fi
```

When a script exited with code 2:
- The `if !` condition evaluated to true (2 is non-zero)
- We entered the then block
- `$?` should have been 2, but the logic was fragile

### Fix Applied

Changed to direct exit code capture with no intervening commands:

```bash
# AFTER (FIXED)
"$script_path" "$task_dir" >> "$MASTER_LOG" 2>&1
local exit_code=$?
```

**Key changes:**
- Removed `if !` construct
- Removed `exit_code=0` initialization
- Capture `$?` immediately after task invocation
- No commands between task invocation and exit code capture

### Status Mapping

Added centralized mapper function for consistency:

```bash
map_exit_to_status() {
  case "$1" in
    0) echo "✅ PASS" ;;
    1) echo "🔴 FAIL" ;;
    2) echo "⚠️ SKIPPED" ;;
    *) echo "🔴 FAIL" ;;
  esac
}
```

Updated status assignment to use the mapper:

```bash
# BEFORE
case $exit_code in
  0) eval "${status_var}='✅ PASS'" ;;
  1) eval "${status_var}='🔴 FAIL'" ;;
  2) eval "${status_var}='⚠️ SKIPPED'" ;;
  *) eval "${status_var}='🔴 FAIL'" ;;
esac

# AFTER
local status
status=$(map_exit_to_status "$exit_code")
eval "${status_var}='$status'"
```

### Summary Counts

The summary count logic was already correct (counting from status variables), but now works correctly because statuses are assigned properly.

**Verification:**
- ✅ Passed: Counts tasks with status containing "PASS"
- ✅ Failed: Counts tasks with status containing "FAIL"
- ✅ Skipped: Counts tasks with status containing "SKIPPED"

---

## Bug #2: Stray "0" Line in Health Monitoring Section

### Problem

The generated summary contained an extra "0" on its own line in the Health Monitoring section:

```markdown
## Health Monitoring
- Health checks performed: 6
- Warnings: 0
0      # ← stray line
```

### Root Cause

The issue was in how `health_warnings` and `health_checks` were computed:

```bash
# BEFORE (BUGGY)
health_warnings=$(grep -c "\[HEALTH\] WARNING" "$MASTER_LOG" 2>/dev/null || echo "0")
health_checks=$(grep -c "\[HEALTH\]" "$MASTER_LOG" 2>/dev/null || echo "0")
```

When `grep -c` found 0 matches, it returned exit code 0 (success) but output "0" as the count. The `|| echo "0"` pattern could cause issues in edge cases.

### Fix Applied

Changed to explicit empty check:

```bash
# AFTER (FIXED)
health_warnings=$(grep -c "\[HEALTH\] WARNING" "$MASTER_LOG" 2>/dev/null)
if [ -z "$health_warnings" ]; then
  health_warnings=0
fi
health_checks=$(grep -c "\[HEALTH\]" "$MASTER_LOG" 2>/dev/null)
if [ -z "$health_checks" ]; then
  health_checks=0
fi
```

**Key changes:**
- Removed `|| echo "0"` pattern
- Explicitly check if variable is empty
- Set to 0 if empty (handles both grep failure and 0 matches)

---

## Additional Improvements

### 1. "All Tasks Skipped" Indicator

Added optional indicator when all tasks are skipped:

```bash
# Add "all tasks skipped" warning if applicable
if [ $skip_count -eq 6 ] && [ $pass_count -eq 0 ] && [ $fail_count -eq 0 ]; then
  echo "" >> "$summary_file"
  echo "⚠️ **ALL TASKS SKIPPED** — No tests were actually executed." >> "$summary_file"
fi
```

This prevents misinterpreting an "all SKIPPED" run as equivalent to a real PASS.

### 2. Centralized Status Mapper

Added `map_exit_to_status()` function to reduce risk of future drift and ensure consistent status mapping across the codebase.

---

## Pipefail Check

**Status:** ✅ **VERIFIED**

The script uses `set -uo pipefail` (line 6):
- `-u`: Treat unset variables as errors
- `-o pipefail`: Return value of pipeline is last command to exit with non-zero status

**Note:** No pipelines are used for task invocations (tasks are invoked directly with redirection), so pipefail does not affect task exit code capture.

---

## Smoke Test Results (Post-Fix)

### Command
```bash
TSAN_ITERATIONS=5 ASAN_ITERATIONS=3 SOAK_ITERATIONS=10 \
  ./scripts/overnight/run_overnight_qa.sh
```

### Report Directory
`qa_reports_20251125_153553/`

### Results

**✅ All Fixes Verified:**

1. **Exit Code Handling:**
   - All 6 tasks correctly show as ⚠️ SKIPPED (not PASS)
   - Master log shows: `[TASK] Finished: TSAN - ⚠️ SKIPPED (0s)`

2. **Summary Counts:**
   - Passed: 0 ✅
   - Failed: 0 ✅
   - Skipped: 6 ✅

3. **Health Monitoring Section:**
   - No stray "0" line ✅
   - Clean formatting:
     ```markdown
     ## Health Monitoring
     - Health checks performed: 6
     - Warnings: 0
     ```

4. **"All Tasks Skipped" Indicator:**
   - Present in summary ✅
   - Shows: `⚠️ **ALL TASKS SKIPPED** — No tests were actually executed.`

5. **Orchestrator Exit Code:**
   - Exit code: 0 ✅ (correct - no FAIL tasks)

### Before/After Comparison

**BEFORE (Phase 3):**
```markdown
| Task | Status | Duration |
|------|--------|----------|
| TSAN Stress | ✅ PASS | 1s |
| ASAN Stress | ✅ PASS | 0s |
...

## Summary
- Passed: 6
- Failed: 0
- Skipped: 0

## Health Monitoring
- Health checks performed: 6
- Warnings: 0
0      # ← stray line
```

**AFTER (Phase 4):**
```markdown
| Task | Status | Duration |
|------|--------|----------|
| TSAN Stress | ⚠️ SKIPPED | 0s |
| ASAN Stress | ⚠️ SKIPPED | 0s |
...

## Summary
- Passed: 0
- Failed: 0
- Skipped: 6

⚠️ **ALL TASKS SKIPPED** — No tests were actually executed.

## Health Monitoring
- Health checks performed: 6
- Warnings: 0
```

---

## Files Modified

1. **`scripts/overnight/run_overnight_qa.sh`**
   - Fixed exit code capture (lines 207-209)
   - Added `map_exit_to_status()` function (lines 60-67)
   - Updated status assignment to use mapper (lines 219-228)
   - Fixed health warnings/checks computation (lines 273-282)
   - Added "all tasks skipped" indicator (lines 310-314)

---

## Validation Checklist

- ✅ Exit code captured immediately after task invocation
- ✅ No commands between task invocation and exit code capture
- ✅ Status mapping: 0→PASS, 1→FAIL, 2→SKIPPED
- ✅ Summary counts computed from status variables
- ✅ Health Monitoring section has no stray "0" line
- ✅ "All tasks skipped" indicator present when applicable
- ✅ Orchestrator exit code: 0 when no FAIL tasks
- ✅ Smoke test passes with correct statuses and counts
- ✅ No other files modified (only `run_overnight_qa.sh`)

---

## Phase 4 Completion Status

**Status:** ✅ **COMPLETE**

**Deliverables:**
- ✅ `scripts/overnight/run_overnight_qa.sh` - Fixed both bugs
- ✅ `docs/qa/PHASE4_BUGFIX_REPORT.md` - This report
- ✅ Smoke test results - All fixes verified

**Next Steps:**
- Await Capo/Consiglieri review
- Proceed to full validation after approval
- Sync Fulcrum workspace for Linux validation (future phase)

---

**End of Phase 4 Bugfix Report**

