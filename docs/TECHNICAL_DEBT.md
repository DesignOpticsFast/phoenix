# Technical Debt Register

This document tracks temporary implementations, workarounds, and known limitations that should be addressed in future sprints.

---

## Local XY Sine Computation (Sprint 4.4+)

**Item:** `XYSineDemo` namespace and local compute implementation

**Location:**
- `src/analysis/demo/XYSineDemo.hpp`
- `src/analysis/demo/XYSineDemo.cpp`
- Used by `LocalExecutor` for local-only XY Sine computation

**Why:** Phoenix supports two compute modes for XY Sine:
- **LocalExecutor**: Uses `XYSineDemo` for local XY Sine computation (no Bedrock server required)
- **RemoteExecutor**: Uses Palantir/Bedrock for remote XY Sine computation

**Current State:**
- `XYSineDemo` provides local XY Sine computation that matches Bedrock's math exactly
- Used by `LocalExecutor` when `AnalysisRunMode::LocalOnly` is selected
- Preserves all async/progress/cancel/timeout semantics
- License checks remain in place (licensing is core, not optional)

**Status:** Production local compute implementation (not a demo or temporary feature)

**Notes:**
- `XYSineDemo` is a legitimate local compute implementation, not a demo
- Both local and remote compute paths are supported and production-ready
- Users can choose between local and remote execution via `AnalysisRunMode`

---

## Future Items

Add new technical debt items here as they are identified.

