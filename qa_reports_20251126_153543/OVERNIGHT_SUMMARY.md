# Overnight QA Summary - 20251126_153543

## Platform
- OS: darwin
- Hostname: Crucible
- Start: 2025-11-26T23:35:43Z
- End: 2025-11-26T23:35:45Z
- Total Duration: 0h 0m 2s (2 seconds)

## Task Results

| Task | Status | Duration | Notes |
|------|--------|----------|-------|
| TSAN Stress | ⚠️ SKIPPED | 0s | See [tsan_report.md](tsan/tsan_report.md) |
| ASAN Stress | ⚠️ SKIPPED | 0s | See [asan_report.md](asan/asan_report.md) |
| Valgrind Check | ⚠️ SKIPPED | 0s | See [valgrind_report.md](valgrind/valgrind_report.md) |
| Soak Test | ⚠️ SKIPPED | 0s | See [soak_report.md](soak/soak_report.md) |
| Coverage Report | 🔴 FAIL | 0s | See [coverage_report.md](coverage/coverage_report.md) |
| Static Analysis | ✅ PASS | 1s | See [static_analysis_report.md](static_analysis/static_analysis_report.md) |

## Health Monitoring
- Health checks performed: 6
- Warnings: 0

## Summary
- Passed: 1
- Failed: 1
- Skipped: 4

## Action Items
- [ ] Investigate Coverage issues: see [coverage_report.md](coverage/coverage_report.md)

---

Full master log: [master.log](master.log)
