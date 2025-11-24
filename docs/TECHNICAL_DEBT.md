# Technical Debt Register

This document tracks temporary implementations, workarounds, and known limitations that should be addressed in future sprints.

---

## Demo-Mode Local XY Sine (Sprint 4.4)

**Item:** `XYSineDemo` namespace and local compute path in `AnalysisWorker`

**Location:**
- `src/analysis/demo/XYSineDemo.hpp`
- `src/analysis/demo/XYSineDemo.cpp`
- `src/analysis/AnalysisWorker.cpp` (demo mode check)

**Why:** Temporary bridge to allow Mac testing without Bedrock/Palantir server running. Enables Capo to test XY Sine functionality (async, progress, cancel, timeout, auto-run) on macOS without requiring a full Bedrock setup.

**Current State:**
- Demo mode enabled via `PHOENIX_DEMO_MODE=1` environment variable
- Local XY Sine computation matches Bedrock's math exactly
- Preserves all async/progress/cancel/timeout semantics
- License checks remain in place (licensing is core, not optional)

**Remove By:** End of Big Sprint 5 (or whenever we have a permanent local/remote compute story)

**Owner:** Mark / Lyra

**Notes:**
- This is explicitly marked as temporary and demo-only
- Must NOT become a permanent architecture feature
- All demo code is clearly namespaced and documented with TODO(Sprint 4.4) tags
- Demo mode is opt-in only (no automatic fallback)

---

## Future Items

Add new technical debt items here as they are identified.

