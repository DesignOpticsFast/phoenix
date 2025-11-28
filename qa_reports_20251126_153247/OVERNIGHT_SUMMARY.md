# Overnight QA Summary - 20251126_153247

## Platform
- OS: darwin
- Hostname: Crucible
- Start: 2025-11-26T23:32:47Z
- End: 2025-11-26T23:32:49Z
- Total Duration: 0h 0m 2s (2 seconds)

## Task Results

| Task | Status | Duration | Notes |
|------|--------|----------|-------|
| TSAN Stress | 🔴 FAIL | 0s | See [tsan_report.md](tsan/tsan_report.md) |
| ASAN Stress | 🔴 FAIL | 0s | See [asan_report.md](asan/asan_report.md) |
| Valgrind Check | ⚠️ SKIPPED | 0s | See [valgrind_report.md](valgrind/valgrind_report.md) |
| Soak Test | 🔴 FAIL | 0s | See [soak_report.md](soak/soak_report.md) |
| Coverage Report | 🔴 FAIL | 2s | See [coverage_report.md](coverage/coverage_report.md) |
| Static Analysis | ✅ PASS | 0s | See [static_analysis_report.md](static_analysis/static_analysis_report.md) |

## Health Monitoring
- Health checks performed: 6
- Warnings: 0

## Summary
- Passed: 1
- Failed: 4
- Skipped: 1

## Action Items
- [ ] Investigate TSAN failures: see [tsan_report.md](tsan/tsan_report.md)
- [ ] Investigate ASAN failures: see [asan_report.md](asan/asan_report.md)
- [ ] Investigate Soak test failures: see [soak_report.md](soak/soak_report.md)
- [ ] Investigate Coverage issues: see [coverage_report.md](coverage/coverage_report.md)

---

Full master log: [master.log](master.log)
