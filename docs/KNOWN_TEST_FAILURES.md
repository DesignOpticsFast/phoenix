# Known Test Failures

_Last updated: Sprint 4.3 — Unified Daily Ritual_

This document lists tests that are known to fail and should not trigger fatal errors during the daily ritual.

**Rule:** If a test fails that is **NOT** in this list → FATAL ERROR

---

## Known Failures (Sprint 4.3)

These tests are expected to fail until Sprint 5:

- `test_analysis_timeout`
- `test_analysiswindow_autorun`
- `analysis_sanity_tests`
- `test_local_xysine` (Mac only)

---

## Platform-Specific Failures

### Mac Only
- `test_local_xysine` - May fail due to environment differences

### Linux Only
- None currently

---

## Adding New Known Failures

When adding a test to this list:
1. Document why it's a known failure
2. Note the expected resolution sprint
3. Update this file in the same commit that introduces the failure

---

## Test Status Logic

During daily ritual:
1. Run tests or check last test results
2. Compare failures against this list
3. If failure is in this list → WARNING (non-fatal)
4. If failure is NOT in this list → FATAL ERROR (stop work)

---

_End of Known Test Failures_

---

