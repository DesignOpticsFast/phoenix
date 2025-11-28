# Phoenix Minimize/Restore Behavior Analysis Report

**Date:** 2025-11-27  
**Issue:** Analysis windows and floating toolbars do not minimize/restore in sync with MainWindow on macOS  
**Status:** Manual sync implemented but still shows two-stage visual behavior

---

## Problem Statement

When minimizing the Phoenix main window on macOS:
- **Expected:** All analysis windows and floating toolbars disappear instantly with MainWindow (single, cohesive action)
- **Actual:** Visual "two-stage" behavior where windows disappear/restore in a noticeable sequence rather than simultaneously

---

## Diagnostic Evidence

### Log Analysis (from most recent test)

**Minimize Sequence:**
```
[MainWindow] WindowStateChange (pre-base) - HIDING children
[XYAnalysisWindow 1] HIDE visible= false
[XYAnalysisWindow 2] HIDE visible= false
[XYAnalysisWindow 3] HIDE visible= false
[XYAnalysisWindow 4] HIDE visible= false
[XYAnalysisWindow 5] HIDE visible= false
[MainWindow] hideChildWindowsForMinimize() hid 7 windows
[MainWindow] WindowStateChange (pre-base) was= NORMAL now= MINIMIZED
[MainWindow] WindowStateChange (post-base) was= NORMAL now= MINIMIZED
```

**Restore Sequence:**
```
[MainWindow] WindowStateChange (pre-base) was= MINIMIZED now= NORMAL
[MainWindow] WindowStateChange (post-base) - RESTORING children
[MainWindow] restoreChildWindowsAfterMinimize() restoring 7 windows
[XYAnalysisWindow 1] SHOW visible= true minimized= false
[XYAnalysisWindow 2] SHOW visible= true minimized= false
[XYAnalysisWindow 3] SHOW visible= true minimized= false
[XYAnalysisWindow 4] SHOW visible= true minimized= false
[XYAnalysisWindow 5] SHOW visible= true minimized= false
[MainWindow] WindowStateChange (post-base) was= MINIMIZED now= NORMAL
```

### Key Findings

1. **Code is executing correctly:**
   - `hideChildWindowsForMinimize()` is called before MainWindow minimize
   - `restoreChildWindowsAfterMinimize()` is called after MainWindow restore
   - All windows receive HIDE/SHOW events in the log

2. **Visual behavior still shows two-stage effect:**
   - Despite code working, user reports visible delay/sequence
   - Windows appear to hide/show one at a time rather than simultaneously

---

## Approaches Attempted

### 1. Qt Window Flags & Parent Relationships
**Attempted:**
- `Qt::Tool` flag on analysis windows
- `Qt::WindowStaysOnTopHint` flag
- Parent relationship (`QMainWindow(parent)` vs `QMainWindow(nullptr)`)
- Transient parent relationships via `QWindow::setTransientParent()`

**Result:** ❌ Analysis windows never received `WindowStateChange` events automatically. macOS/Qt did not group them for minimize/restore.

**Evidence:** Diagnostic logs showed NO `WindowStateChange` events for analysis windows when MainWindow minimized, only `ActivationChange` events.

### 2. Manual Hide/Show Synchronization
**Current Implementation:**
- `MainWindow::changeEvent()` detects minimize/restore transitions
- Calls `hideChildWindowsForMinimize()` before base class processes minimize
- Calls `restoreChildWindowsAfterMinimize()` after base class processes restore
- Uses `setVisible(false/true)` with `QApplication::processEvents()` for immediate effect

**Result:** ⚠️ Code executes correctly (verified in logs), but visual behavior still shows two-stage effect.

**Files Modified:**
- `phoenix/src/ui/main/MainWindow.h` - Added tracking list and helper declarations
- `phoenix/src/ui/main/MainWindow.cpp` - Implemented hide/show synchronization

---

## Root Cause Analysis

### Why Automatic Grouping Doesn't Work

1. **macOS Window Management:**
   - macOS groups windows based on application bundle, not Qt parent relationships
   - `Qt::Tool` windows are treated as NSPanels, which don't minimize with main window
   - Parent/child relationships in Qt don't translate to native window grouping on macOS

2. **Qt Event System:**
   - Child windows don't receive `WindowStateChange` events when parent minimizes
   - Only `ActivationChange` events are received
   - This is by design in Qt - child windows are independent top-level windows

### Why Manual Sync Shows Two-Stage Behavior

**Hypothesis 1: Event Loop Timing**
- `hide()`/`show()` calls are queued in the event loop
- Even with `QApplication::processEvents()`, macOS window animations may be asynchronous
- Multiple windows hiding/showing sequentially creates visible cascade

**Hypothesis 2: macOS Window Server**
- macOS window server may process hide/show operations sequentially
- Window animations (even for hide/show) may be queued separately from minimize animation
- No way to synchronize Qt hide/show with macOS minimize animation

**Hypothesis 3: Visual Rendering**
- Each window's hide/show triggers separate rendering updates
- MainWindow minimize animation happens independently
- No atomic "hide all + minimize main" operation

---

## Current Implementation Details

### Code Structure

```cpp
// MainWindow.h
QList<QPointer<QWidget>> m_hiddenDueToMinimize;
void hideChildWindowsForMinimize();
void restoreChildWindowsAfterMinimize();

// MainWindow.cpp - changeEvent()
if (!wasMinimized && isNowMinimized) {
    hideChildWindowsForMinimize();  // BEFORE base class
}
QMainWindow::changeEvent(event);
if (wasMinimized && !isNowMinimized) {
    restoreChildWindowsAfterMinimize();  // AFTER base class
}
```

### Windows Tracked
- Analysis windows (via `AnalysisWindowManager::windows()`)
- Floating dock widgets (`m_toolboxDock`, `m_propertiesDock`)
- Floating toolbars (`m_mainToolBar`, `m_topRibbon`, `m_rightRibbon`)

---

## Alternative Approaches to Consider

### Option A: Batch Hide/Show with Single Event Processing
**Idea:** Hide all windows, then process events once, then minimize main window
**Pros:** Might reduce visible cascade
**Cons:** Still relies on Qt event loop timing

### Option B: Use QTimer::singleShot(0, ...) for Deferred Execution
**Idea:** Defer hide/show to next event loop iteration to align with minimize animation
**Pros:** Might better align with macOS animation timing
**Cons:** Could introduce delay

### Option C: Native macOS Window Grouping
**Idea:** Use Objective-C++ to set native window collection behavior
**Pros:** True native grouping
**Cons:** Platform-specific code, requires Objective-C++

### Option D: Opacity/Lower Instead of Hide/Show
**Idea:** Set opacity to 0 or lower windows instead of hiding
**Pros:** Might be faster/less visible
**Cons:** Windows still exist, may interfere with other operations

### Option E: Accept Current Behavior
**Idea:** Document that this is expected Qt/macOS behavior
**Pros:** No code complexity
**Cons:** Doesn't meet UX requirements

---

## Recommendations

### Short Term
1. **Try Option A or B** - Minor timing adjustments might improve perceived behavior
2. **Add user feedback** - If two-stage is acceptable with better timing, optimize current approach

### Medium Term
3. **Consider Option C** - If native grouping is required, implement platform-specific solution
4. **UX Review** - Determine if current behavior is acceptable or if native grouping is mandatory

### Long Term
5. **Architecture Review** - Consider if analysis windows should be QDockWidgets instead of separate QMainWindows
6. **Platform Abstraction** - Create window management abstraction for cross-platform minimize behavior

---

## Technical Constraints

- **Qt-only requirement:** User wants Qt-only solution, no platform-specific hacks
- **macOS focus:** Primary development on macOS (Crucible)
- **Screen sharing:** User connects via screen sharing, which may affect window behavior perception

---

## Next Steps

1. **Review this analysis** with team
2. **Decide on approach:**
   - Accept current behavior with minor optimizations
   - Implement native macOS grouping (Option C)
   - Architectural change (Option E - QDockWidgets)
3. **If proceeding with current approach:** Try timing optimizations (Options A/B)
4. **If native grouping required:** Design Objective-C++ integration

---

## Files Involved

- `phoenix/src/ui/main/MainWindow.h` - Tracking and helper declarations
- `phoenix/src/ui/main/MainWindow.cpp` - Implementation
- `phoenix/src/ui/analysis/XYAnalysisWindow.cpp` - Analysis window implementation
- `phoenix/src/ui/analysis/AnalysisWindowManager.hpp` - Window management

---

## Diagnostic Logs Available

- `/tmp/phoenix_minimize_test.log` - Most recent test with manual sync
- Previous test logs show evolution of behavior through different approaches

