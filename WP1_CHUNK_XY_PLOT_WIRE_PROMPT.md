# 🧠 UnderLord Prompt – Wire XY Plot Icon → AnalysisWindow (Qt Graphs)

## Intent

Wire the existing "XY Plot" UI action in Phoenix to open an `AnalysisWindow` that uses `XYPlotViewGraphs` (Qt Graphs), so I can click the icon and immediately exercise pan/zoom on the Qt Graphs view.

---

## Context

- **Repo:** DesignOpticsFast/phoenix
- **Qt:** 6.10, Qt Graphs + QQuickWidget already integrated.
- **WP1.A:** Finished. `XYPlotViewGraphs` exists, implements `IAnalysisView`, and is used in tests and perf benchmarks.
- **Current state:** 
  - There is an XY plot action (`m_xyPlotAction`) in the Phoenix main window UI (both menu and ribbon toolbar).
  - The action is already connected to `MainWindow::showXYPlot()` slot.
  - `showXYPlot()` currently shows a "not yet implemented" message box (line ~1198 in `MainWindow.cpp`).
  - `AnalysisWindow` has `setView(std::unique_ptr<IAnalysisView>)` API ready to use.

I want that icon to open a real `AnalysisWindow` that hosts an `XYPlotViewGraphs` instance, with some simple dummy data (e.g. a sine wave), so I can visually test Qt Graphs on my Mac by just running Phoenix and clicking the icon.

---

## Ask

Please implement the following steps in the Phoenix repo.

### 1. Locate the existing XY Plot action implementation

1. **File:** `src/ui/main/MainWindow.cpp` (around line 1196)
2. **Current implementation:**
   ```cpp
   void MainWindow::showXYPlot()
   {
       QMessageBox::information(this, tr("XY Plot"), tr("This feature is not yet implemented."));
   }
   ```
3. **Action details:**
   - Action name: `m_xyPlotAction` (member variable)
   - Already connected to `showXYPlot()` slot (line ~377)
   - Appears in both Analysis menu and ribbon toolbar
   - Icon: "chart" icon from IconProvider

**Note:** The action wiring is already complete; we just need to replace the stub implementation.

---

### 2. Replace `showXYPlot()` implementation

Replace the message box stub with code that:

1. **Include necessary headers** at the top of `MainWindow.cpp`:
   ```cpp
   #include "ui/analysis/AnalysisWindow.hpp"
   #include "plot/XYPlotViewGraphs.hpp"
   #include <QPointF>
   #include <vector>
   #include <cmath>
   #include <memory>
   ```

2. **Create an AnalysisWindow** with the main window as parent:
   ```cpp
   auto* win = new AnalysisWindow(this);
   win->setWindowTitle(tr("XY Plot – Qt Graphs"));
   ```

3. **Generate a simple test dataset** (e.g. 1000-point sine wave):
   ```cpp
   std::vector<QPointF> points;
   points.reserve(1000);
   for (int i = 0; i < 1000; ++i) {
       double x = i * 0.01;  // x from 0 to 9.99
       double y = std::sin(x);
       points.emplace_back(x, y);
   }
   ```

4. **Create XYPlotViewGraphs instance** and set the data:
   ```cpp
   auto view = std::make_unique<XYPlotViewGraphs>();
   view->setTitle(tr("Sine Wave Test"));
   view->setData(points);
   ```

5. **Install the view into AnalysisWindow**:
   ```cpp
   win->setView(std::move(view));
   ```

6. **Configure and show the window**:
   ```cpp
   win->resize(900, 600);
   win->setAttribute(Qt::WA_DeleteOnClose);  // Clean up when closed
   win->show();
   ```

**Important:** 
- Use `std::make_unique<XYPlotViewGraphs>()` and pass it via `std::move()` to `setView()`.
- Set data **before** calling `setView()` so the graph is populated when the window appears.
- Use `Qt::WA_DeleteOnClose` so the window cleans up properly when closed.

---

### 3. Verify includes and forward declarations

1. **Check `MainWindow.h`** (around line 70):
   - Ensure `showXYPlot()` is declared in the private slots section.
   - No changes needed here (already declared).

2. **Check `MainWindow.cpp`**:
   - Add the includes listed in step 2.1 above.
   - Ensure they're added near the top with other includes.

---

### 4. Build & verification (code-level)

1. **Run CMake and build Phoenix:**
   ```bash
   cd /home/ec2-user/workspace/phoenix
   rm -rf build/graphs  # Clean build to catch any missing dependencies
   cmake -S . -B build/graphs \
     -DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64 \
     -DBUILD_TESTING=OFF  # Skip tests for faster build
   cmake --build build/graphs --target phoenix_app --parallel
   ```

2. **Ensure:**
   - `XYPlotViewGraphs` compiles and links correctly.
   - `AnalysisWindow` compiles and links correctly.
   - `MainWindow` compiles without errors or warnings.
   - No missing includes or undefined symbols.

3. **Optional quick smoke test:**
   - If you can run the app in your environment, verify that clicking the XY Plot icon doesn't crash.
   - Visual verification will be done on Mac, so this is optional.

---

### 5. Deliverable & reporting

When you're done, provide a brief summary:

1. **Files changed:**
   - `src/ui/main/MainWindow.cpp` (replaced `showXYPlot()` implementation, added includes)
   - Any other files if needed (e.g., if includes were added elsewhere)

2. **Implementation details:**
   - Confirmation that `showXYPlot()` now creates an `AnalysisWindow` with `XYPlotViewGraphs`.
   - Dataset: 1000-point sine wave (x: 0-9.99, y: sin(x)).
   - Window title: "XY Plot – Qt Graphs"
   - Window size: 900x600

3. **Action wiring:**
   - Action: `m_xyPlotAction` (already wired, no changes needed)
   - Slot: `MainWindow::showXYPlot()` (implementation replaced)
   - Location: Analysis menu and ribbon toolbar (both already connected)

4. **Build status:**
   - Confirmation that the project builds successfully for Qt 6.10.
   - Any warnings or notes about the build.

5. **Known behavior:**
   - Clicking the XY Plot icon opens an `AnalysisWindow` with `XYPlotViewGraphs`.
   - The graph shows a sine wave (visually obvious that drawing works).
   - The window can be panned/zoomed (Qt Graphs handles this automatically).
   - Window closes cleanly when dismissed.

---

## Definition of Done

- ✅ Clicking the XY Plot icon/action in the Phoenix main window opens an `AnalysisWindow` that hosts `XYPlotViewGraphs`.
- ✅ The graph is populated with simple dummy data (sine wave) so it's visually obvious that drawing works.
- ✅ The window can be panned/zoomed (Qt Graphs handles this automatically).
- ✅ All changes build cleanly without errors or warnings.
- ✅ Window cleanup works correctly (`Qt::WA_DeleteOnClose` ensures proper disposal).

---

## Notes & Improvements Over Original Prompt

1. **Specific file locations:** Identified exact file and line numbers where changes are needed.
2. **Existing code reference:** Referenced the current stub implementation that needs replacement.
3. **Correct API usage:** Uses `std::unique_ptr` and `std::move()` as required by `AnalysisWindow::setView()`.
4. **Proper order:** Sets data before installing view so graph is populated when window appears.
5. **Memory management:** Explicitly uses `Qt::WA_DeleteOnClose` for proper cleanup.
6. **Includes:** Lists all necessary includes upfront.
7. **Build command:** Provides specific build command with correct paths.
8. **Error handling:** Implicitly handled by Qt's parent-child relationship and `WA_DeleteOnClose`.

---

## Potential Edge Cases (Optional Enhancements)

If you want to go beyond the basic requirement, consider:

1. **Multiple windows:** Currently, each click opens a new window. This is fine for testing, but you might want to track open windows and reuse/focus existing ones.
2. **Error handling:** If `XYPlotViewGraphs` fails to initialize (e.g., QML errors), show a user-friendly error message instead of crashing.
3. **Window state:** Save/restore window geometry (position, size) using QSettings.
4. **Data variety:** Add menu options for different test datasets (sine, cosine, random, etc.).

These are **not required** for this chunk - the basic wiring is sufficient.

