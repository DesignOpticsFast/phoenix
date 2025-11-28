# Overnight QA Summary - 20251126_153945

## Platform
- OS: darwin
- Hostname: Crucible
- Start: 2025-11-26T23:39:45Z
- End: 2025-11-26T23:40:01Z
- Total Duration: 0h 0m 16s (16 seconds)

## Task Results

| Task | Status | Duration | Notes |
|------|--------|----------|-------|
| TSAN Stress | ✅ PASS | 12s | See [tsan_report.md](tsan/tsan_report.md) |
| ASAN Stress | ✅ PASS | 1s | See [asan_report.md](asan/asan_report.md) |
| Valgrind Check | ⚠️ SKIPPED | 0s | See [valgrind_report.md](valgrind/valgrind_report.md) |
| Soak Test | ✅ PASS | 1s | See [soak_report.md](soak/soak_report.md) |
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
