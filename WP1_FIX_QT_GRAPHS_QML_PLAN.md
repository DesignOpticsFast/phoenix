# Plan: Fix Qt Graphs QML (Axis Renderer + Interactions)

## Problem Analysis

**Current Issue:**
- Qt Graphs view draws a curve ✅
- Pan/zoom/resize don't work ❌
- Critical error: `qt.graphs2d.critical: Axis renderer not found` ❌

**Root Cause:**
The QML `GraphsView` in `XYPlotView.qml` is missing proper axis renderer configuration. The axes (`ValueAxis`) are defined and assigned to the series, but the `GraphsView` itself doesn't have the axis renderer properly configured, which disables interactions.

**Current QML Structure:**
```qml
Item {
    GraphsView {
        LineSeries { objectName: "mainSeries" }
        ValueAxis { id: axisX }
        ValueAxis { id: axisY }
        Component.onCompleted: {
            lineSeries.axisX = axisX
            lineSeries.axisY = axisY
        }
    }
}
```

**Problem:** Axes are assigned to series but `GraphsView` doesn't have axis renderer configured.

---

## Solution Plan

### Step 1: Research Qt Graphs 2D Correct Structure

**Action:** Review Qt 6.10 Graphs 2D documentation/examples for proper axis renderer setup.

**Expected Pattern (based on Qt Charts/Graphs conventions):**
- `GraphsView` should have axes configured at the view level
- Axes can be children of `GraphsView` or assigned via properties
- Interactivity is typically enabled via `GraphsView` properties (e.g., `interactive`, `zoomEnabled`, `panEnabled`)

**Key Properties to Check:**
- `GraphsView.axisX` / `GraphsView.axisY` - Direct axis assignment
- `GraphsView.interactive` - Enable/disable interactions
- `GraphsView.zoomEnabled` / `GraphsView.panEnabled` - Specific interaction controls
- Axis renderer configuration (may be implicit or explicit)

---

### Step 2: Update QML Structure

**File:** `src/qml/XYPlotView.qml`

**Changes Needed:**

1. **Ensure axes are properly configured on GraphsView:**
   - Either assign axes directly to `GraphsView` properties
   - Or ensure axes are children and properly recognized

2. **Enable interactivity:**
   - Add `interactive: true` or equivalent properties
   - Enable pan/zoom if separate properties exist

3. **Maintain C++ compatibility:**
   - Keep `LineSeries` with `objectName: "mainSeries"`
   - Keep `GraphsView` with `objectName: "graphsView"` (if needed)
   - Ensure axes are still assigned to series for data binding

**Proposed Structure:**
```qml
import QtQuick
import QtGraphs

Item {
    id: root
    
    GraphsView {
        id: graphView
        objectName: "graphsView"
        anchors.fill: parent
        
        // Enable interactions
        interactive: true
        
        // Define axes (may need to be at GraphsView level)
        ValueAxis {
            id: axisX
            titleText: "X"
        }
        
        ValueAxis {
            id: axisY
            titleText: "Y"
        }
        
        // Assign axes to GraphsView (if required by API)
        // axisX: axisX
        // axisY: axisY
        
        // LineSeries with axes assigned
        LineSeries {
            id: lineSeries
            objectName: "mainSeries"
            name: "Data"
            axisX: axisX
            axisY: axisY
        }
    }
}
```

**Alternative Structure (if axes must be direct children):**
```qml
GraphsView {
    id: graphView
    anchors.fill: parent
    interactive: true
    
    // Axes as direct children (GraphsView manages them)
    ValueAxis {
        id: axisX
        titleText: "X"
    }
    
    ValueAxis {
        id: axisY
        titleText: "Y"
    }
    
    LineSeries {
        id: lineSeries
        objectName: "mainSeries"
        name: "Data"
        axisX: axisX
        axisY: axisY
    }
}
```

---

### Step 3: Verify Qt 6.10 API

**Action:** Check actual Qt 6.10 Graphs 2D API to confirm:
- How axes are configured on `GraphsView`
- What properties enable interactivity
- Whether axis renderer is implicit or needs explicit configuration

**Resources:**
- Qt 6.10 documentation: `GraphsView` QML type
- Qt 6.10 examples: "Graphs 2D in Qt Widgets" or similar
- Qt installation examples: `/opt/Qt/6.10.0/gcc_64/examples/graphs/` (if available)

---

### Step 4: Implementation Steps

1. **Backup current QML** (optional, git handles this)
2. **Update `XYPlotView.qml`** with correct structure:
   - Add interactivity properties
   - Ensure axes are properly configured
   - Keep `objectName: "mainSeries"` for C++ access
3. **Test QML syntax** (CMake will validate during build)
4. **Build `phoenix_app`** to verify no QML errors
5. **Check Application Output** for:
   - ✅ No "Axis renderer not found" critical message
   - ✅ No QML errors
   - ✅ Successful QML load

---

### Step 5: Verification

**Build Verification:**
```bash
cd /home/ec2-user/workspace/phoenix
rm -rf build/graphs
cmake -S . -B build/graphs \
  -DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64 \
  -DBUILD_TESTING=OFF
cmake --build build/graphs --target phoenix_app --parallel
```

**Expected Results:**
- ✅ Build succeeds
- ✅ No QML compilation errors
- ✅ No "Axis renderer not found" in output

**Runtime Verification (on Mac after pull):**
- ✅ XY Plot window opens
- ✅ Sine wave displays correctly
- ✅ Pan works (drag mouse/trackpad)
- ✅ Zoom works (pinch/scroll)
- ✅ Resize works (window resize)
- ✅ No critical warnings in Application Output

---

## Implementation Notes

### Critical Requirements

1. **Maintain C++ Compatibility:**
   - `LineSeries` must have `objectName: "mainSeries"`
   - C++ code uses `findChild<QObject*>("mainSeries")` - must still work
   - `setData()` calls `replace()` on the series - must still work

2. **Don't Change C++ Unless Necessary:**
   - `XYPlotViewGraphs.cpp` - no changes expected
   - `MainWindow.cpp` - no changes expected
   - Only QML file should change

3. **Error Handling:**
   - If Qt 6.10 API differs from expectations, document the actual API
   - If axis renderer requires different approach, adapt accordingly

### Potential API Variations

**Scenario A: Axes as GraphsView Properties**
```qml
GraphsView {
    axisX: ValueAxis { id: axisX; titleText: "X" }
    axisY: ValueAxis { id: axisY; titleText: "Y" }
    LineSeries { axisX: axisX; axisY: axisY }
}
```

**Scenario B: Axes as Direct Children**
```qml
GraphsView {
    ValueAxis { id: axisX }
    ValueAxis { id: axisY }
    LineSeries { axisX: axisX; axisY: axisY }
}
```

**Scenario C: Explicit Axis Renderer**
```qml
GraphsView {
    axisRenderer: SomeRendererType { ... }
    // or
    axes: [axisX, axisY]
}
```

**Action:** Check Qt 6.10 documentation to determine correct pattern.

---

## Deliverable

**Summary Should Include:**

1. **QML Changes:**
   - Show before/after QML structure
   - Explain what was added/changed
   - Document any Qt 6.10 API discoveries

2. **Build Status:**
   - Confirmation that build succeeds
   - Confirmation that "Axis renderer not found" is gone
   - Any warnings or notes

3. **API Documentation:**
   - How axes are configured in Qt 6.10 Graphs 2D
   - What properties enable interactivity
   - Any deviations from expected patterns

4. **Testing Notes:**
   - Build verification results
   - Runtime verification (if possible on dev-01)
   - Instructions for Mac testing

---

## Risks & Mitigation

**Risk 1: Qt 6.10 API differs from expectations**
- **Mitigation:** Check documentation first, adapt to actual API

**Risk 2: Interactivity properties unknown**
- **Mitigation:** Try common patterns (`interactive: true`, `zoomEnabled: true`, etc.)

**Risk 3: Breaking C++ data binding**
- **Mitigation:** Keep `objectName: "mainSeries"` unchanged, test C++ still finds series

**Risk 4: QML syntax errors**
- **Mitigation:** CMake will catch during build, fix iteratively

---

## Success Criteria

- ✅ QML file updated with proper axis renderer configuration
- ✅ Interactivity enabled (pan/zoom/resize)
- ✅ `objectName: "mainSeries"` preserved for C++ access
- ✅ Build succeeds without QML errors
- ✅ "Axis renderer not found" critical message eliminated
- ✅ No C++ changes required
- ✅ Ready for Mac testing

---

## Next Steps After Implementation

1. Commit and push changes
2. User pulls on Mac
3. User rebuilds Phoenix
4. User tests pan/zoom/resize
5. User reports results

---

## Questions to Resolve During Implementation

1. What is the exact Qt 6.10 Graphs 2D API for axis configuration?
2. What properties enable pan/zoom interactions?
3. Does `GraphsView` need axes assigned directly, or is series assignment sufficient?
4. Is there an explicit axis renderer component, or is it implicit?

These will be answered during Step 3 (API verification).

