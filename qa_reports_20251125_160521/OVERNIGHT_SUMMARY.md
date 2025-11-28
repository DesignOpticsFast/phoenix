# Overnight QA Summary - 20251125_160521

## Platform
- OS: darwin
- Hostname: Crucible
- Start: 2025-11-26T00:05:21Z
- End: 2025-11-26T00:05:22Z
- Total Duration: 0h 0m 1s (1 seconds)

## Task Results

| Task | Status | Duration | Notes |
|------|--------|----------|-------|
| TSAN Stress | ⚠️ SKIPPED | 0s | See [tsan_report.md](tsan/tsan_report.md) |
| ASAN Stress | ⚠️ SKIPPED | 0s | See [asan_report.md](asan/asan_report.md) |
| Valgrind Check | ⚠️ SKIPPED | 0s | See [valgrind_report.md](valgrind/valgrind_report.md) |
| Soak Test | ⚠️ SKIPPED | 0s | See [soak_report.md](soak/soak_report.md) |
| Coverage Report | ⚠️ SKIPPED | 0s | See [coverage_report.md](coverage/coverage_report.md) |
| Static Analysis | 🔴 FAIL | 1s | See [static_analysis_report.md](static_analysis/static_analysis_report.md) |

## Health Monitoring
- Health checks performed: 6
- Warnings: 0

## Summary
- Passed: 0
- Failed: 1
- Skipped: 5

## Action Items
- [ ] Investigate Static analysis errors: see [static_analysis_report.md](static_analysis/static_analysis_report.md)

---

Full master log: [master.log](master.log)
