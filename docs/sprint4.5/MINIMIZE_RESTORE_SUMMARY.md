# Phoenix Minimize/Restore Issue - Executive Summary

**Date:** 2025-11-27  
**Status:** Manual sync implemented, but visual two-stage behavior persists

## Problem
Analysis windows and floating toolbars do not minimize/restore in sync with MainWindow on macOS, showing a visible "two-stage" effect.

## Current State
✅ **Code works correctly** - Diagnostic logs confirm:
- Hide/show functions execute at correct times
- All windows receive HIDE/SHOW events
- 7 windows tracked (5 analysis + 2 floating)

❌ **Visual behavior insufficient** - User reports:
- Windows still appear to hide/show sequentially
- Two-stage effect remains visible

## Root Cause
1. **macOS/Qt limitation:** No automatic window grouping via Qt flags or parent relationships
2. **Event loop timing:** Even with `processEvents()`, macOS window animations are asynchronous
3. **No atomic operation:** Cannot synchronize Qt hide/show with macOS minimize animation

## Approaches Tried
1. ❌ Qt window flags (`Qt::Tool`, `WindowStaysOnTopHint`)
2. ❌ Parent/child relationships
3. ❌ Transient parent relationships
4. ⚠️ Manual hide/show sync (current) - works but not visually perfect

## Options for Next Steps

### Option A: Timing Optimizations (Qt-only)
- Batch operations differently
- Use `QTimer::singleShot(0, ...)` for alignment
- **Effort:** Low | **Likelihood of success:** Medium

### Option B: Native macOS Grouping (Platform-specific)
- Use Objective-C++ to set native window collection
- **Effort:** Medium | **Likelihood of success:** High | **Constraint:** Violates "Qt-only" requirement

### Option C: Architectural Change
- Convert analysis windows to QDockWidgets
- **Effort:** High | **Likelihood of success:** High | **Impact:** Major refactor

### Option D: Accept Current Behavior
- Document as expected Qt/macOS limitation
- **Effort:** None | **Likelihood of success:** N/A

## Recommendation
**Short term:** Try Option A (timing optimizations)  
**If insufficient:** Review Option B (native grouping) vs Option C (architectural change) based on project constraints

## Full Analysis
See `MINIMIZE_RESTORE_ANALYSIS.md` for complete technical details, diagnostic logs, and implementation notes.

