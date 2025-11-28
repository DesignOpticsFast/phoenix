# Overnight QA Summary - 20251126_155313

## Platform
- OS: darwin
- Hostname: Crucible
- Start: 2025-11-26T23:53:13Z
- End: 2025-11-26T23:57:32Z
- Total Duration: 0h 4m 19s (259 seconds)

## Task Results

| Task | Status | Duration | Notes |
|------|--------|----------|-------|
| TSAN Stress | ✅ PASS | 229s | See [tsan_report.md](tsan/tsan_report.md) |
| ASAN Stress | ✅ PASS | 4s | See [asan_report.md](asan/asan_report.md) |
| Valgrind Check | ⚠️ SKIPPED | 0s | See [valgrind_report.md](valgrind/valgrind_report.md) |
| Soak Test | ✅ PASS | 24s | See [soak_report.md](soak/soak_report.md) |
| Coverage Report | 🔴 FAIL | 2s | See [coverage_report.md](coverage/coverage_report.md) |
| Static Analysis | ✅ PASS | 0s | See [static_analysis_report.md](static_analysis/static_analysis_report.md) |

## Health Monitoring
- Health checks performed: 6
- Warnings: 0

## Summary
- Passed: 4
- Failed: 1
- Skipped: 1

## Action Items
- [ ] Investigate Coverage issues: see [coverage_report.md](coverage/coverage_report.md)

---

Full master log: [master.log](master.log)
