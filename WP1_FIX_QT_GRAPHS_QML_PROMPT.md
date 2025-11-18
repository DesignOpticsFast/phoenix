# 🧠 UnderLord Prompt – Fix Qt Graphs QML (Axis Renderer + Interactions)

## Intent

Fix the Qt Graphs QML configuration so that pan/zoom/resize interactions work properly and eliminate the "Axis renderer not found" critical error.

---

## Context

- **Repo:** DesignOpticsFast/phoenix
- **Qt:** 6.10, Qt Graphs + QQuickWidget already integrated
- **Current Issue:**
  - Qt Graphs view draws a curve ✅
  - Pan/zoom/resize don't work ❌
  - Critical error: `qt.graphs2d.critical: Axis renderer not found` ❌
- **File to Fix:** `src/qml/XYPlotView.qml`
- **C++ Code:** `src/plot/XYPlotViewGraphs.cpp` - **DO NOT CHANGE** (works correctly)

**Root Cause:** The QML `GraphsView` is missing proper axis renderer configuration. Axes are defined and assigned to the series, but the `GraphsView` itself doesn't have the axis renderer properly configured, which disables interactions.

---

## Current QML Structure

**File:** `src/qml/XYPlotView.qml`

```qml
import QtQuick
import QtGraphs

Item {
    id: root
    
    GraphsView {
        id: graphView
        objectName: "graphsView"
        anchors.fill: parent
        
        LineSeries {
            id: lineSeries
            objectName: "mainSeries"  // MUST KEEP for C++ access
            name: "Data"
        }
        
        ValueAxis {
            id: axisX
            titleText: "X"
        }
        
        ValueAxis {
            id: axisY
            titleText: "Y"
        }
        
        Component.onCompleted: {
            lineSeries.axisX = axisX
            lineSeries.axisY = axisY
        }
    }
}
```

**Problem:** Axes are assigned to series in `Component.onCompleted`, but `GraphsView` doesn't have axis renderer configured, causing the critical error and disabling interactions.

---

## Ask

Please implement the following steps:

### Step 1: Verify Qt 6.10 Graphs 2D API

Before making changes, check the Qt 6.10 documentation/examples to understand:

1. **How axes are configured on GraphsView:**
   - Are axes assigned as properties (e.g., `GraphsView.axisX`, `GraphsView.axisY`)?
   - Or are they children that GraphsView automatically detects?
   - Or is there an explicit axis renderer component?

2. **What properties enable interactivity:**
   - `interactive: true`?
   - `zoomEnabled: true` / `panEnabled: true`?
   - Other properties?

**Resources to Check:**
- Qt 6.10 documentation: `GraphsView` QML type reference
- Qt installation examples: `/opt/Qt/6.10.0/gcc_64/examples/graphs/` (if available)
- Qt installation docs: `/opt/Qt/6.10.0/gcc_64/doc/qtdoc/qtgraphs-qmlmodule.html` (if available)

**Command to check examples:**
```bash
find /opt/Qt/6.10.0/gcc_64/examples -name "*graph*" -type d 2>/dev/null | head -5
```

---

### Step 2: Update QML with Proper Axis Configuration

**File:** `src/qml/XYPlotView.qml`

**Requirements:**

1. **Maintain C++ Compatibility (CRITICAL):**
   - Keep `LineSeries` with `objectName: "mainSeries"` - C++ uses `findChild<QObject*>("mainSeries")`
   - Keep `GraphsView` structure that C++ can access
   - Ensure axes are still assigned to series for data binding

2. **Fix Axis Renderer:**
   - Configure axes properly so `GraphsView` recognizes them
   - This should eliminate "Axis renderer not found" error

3. **Enable Interactivity:**
   - Add properties to enable pan/zoom/resize
   - Common patterns: `interactive: true`, `zoomEnabled: true`, `panEnabled: true`

**Proposed Structure (adapt based on Step 1 findings):**

```qml
import QtQuick
import QtGraphs

Item {
    id: root
    
    GraphsView {
        id: graphView
        objectName: "graphsView"
        anchors.fill: parent
        
        // Enable interactions (verify exact property names from Step 1)
        interactive: true
        
        // Define axes
        ValueAxis {
            id: axisX
            titleText: "X"
        }
        
        ValueAxis {
            id: axisY
            titleText: "Y"
        }
        
        // Assign axes to GraphsView (if required by API - verify in Step 1)
        // Option A: As properties
        // axisX: axisX
        // axisY: axisY
        
        // Option B: As direct children (GraphsView auto-detects)
        // (axes already defined above)
        
        // LineSeries with axes assigned
        LineSeries {
            id: lineSeries
            objectName: "mainSeries"  // MUST KEEP - C++ depends on this
            name: "Data"
            axisX: axisX
            axisY: axisY
        }
    }
}
```

**Key Changes:**
- Add `interactive: true` (or equivalent) to enable interactions
- Ensure axes are properly recognized by `GraphsView` (method depends on API)
- Keep `objectName: "mainSeries"` unchanged
- Assign axes to series directly (not in `Component.onCompleted`)

---

### Step 3: Build and Verify

1. **Build phoenix_app:**
   ```bash
   cd /home/ec2-user/workspace/phoenix
   rm -rf build/graphs
   cmake -S . -B build/graphs \
     -DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64 \
     -DBUILD_TESTING=OFF
   cmake --build build/graphs --target phoenix_app --parallel
   ```

2. **Verify:**
   - ✅ Build succeeds without QML errors
   - ✅ No "Axis renderer not found" in build output
   - ✅ QML compiles correctly

3. **Optional Runtime Test (if possible on dev-01):**
   - Run phoenix_app
   - Click XY Plot icon
   - Check Application Output for:
     - ✅ No "Axis renderer not found" critical message
     - ✅ No QML errors
     - ✅ Successful QML load

---

### Step 4: Deliverable & Reporting

When done, provide:

1. **QML Changes:**
   - Show the updated `XYPlotView.qml` structure
   - Explain what was added/changed
   - Document Qt 6.10 API findings from Step 1

2. **API Documentation:**
   - How axes are configured in Qt 6.10 Graphs 2D
   - What properties enable interactivity
   - Any deviations from expected patterns

3. **Build Status:**
   - Confirmation that build succeeds
   - Confirmation that "Axis renderer not found" is gone
   - Any warnings or notes

4. **Testing Notes:**
   - Build verification results
   - Runtime verification (if possible)
   - Instructions for Mac testing (user will test pan/zoom/resize)

---

## Definition of Done

- ✅ QML file updated with proper axis renderer configuration
- ✅ Interactivity enabled (pan/zoom/resize properties added)
- ✅ `objectName: "mainSeries"` preserved for C++ access
- ✅ Build succeeds without QML errors
- ✅ "Axis renderer not found" critical message eliminated
- ✅ No C++ changes made (only QML changed)
- ✅ Ready for Mac testing

---

## Important Constraints

1. **DO NOT change C++ code:**
   - `XYPlotViewGraphs.cpp` - works correctly, no changes needed
   - `MainWindow.cpp` - works correctly, no changes needed
   - Only QML file should change

2. **Maintain C++ compatibility:**
   - `LineSeries` must have `objectName: "mainSeries"`
   - C++ uses `findChild<QObject*>("mainSeries")` - must still work
   - `setData()` calls `replace()` on the series - must still work

3. **If Qt 6.10 API differs:**
   - Document the actual API found
   - Adapt QML to match actual API
   - Don't force a pattern that doesn't exist

---

## Troubleshooting

**If "Axis renderer not found" persists:**
- Check if axes need to be assigned to `GraphsView` properties explicitly
- Check if there's an explicit axis renderer component
- Try different axis assignment patterns

**If interactivity still doesn't work:**
- Verify property names (`interactive`, `zoomEnabled`, `panEnabled`, etc.)
- Check Qt 6.10 documentation for correct property names
- May need to check mouse/trackpad event handling

**If C++ can't find mainSeries:**
- Verify `objectName: "mainSeries"` is still present
- Check QML structure hasn't broken `findChild` traversal
- Test with debug output in C++ constructor

---

## Next Steps After Implementation

1. Commit and push changes
2. User pulls on Mac
3. User rebuilds Phoenix
4. User tests pan/zoom/resize
5. User reports results

---

## Summary

Fix the QML to properly configure axis renderer and enable interactions, while maintaining C++ compatibility. The goal is: no "Axis renderer not found" warnings, and basic pan/zoom/resize interactions working in the Qt Graphs view.

