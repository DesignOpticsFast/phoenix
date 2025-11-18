# WP1 Chunk 6: Phoenix AnalysisWindow & IAnalysisView Shell - Implementation Plan

**Date:** 2025-11-17  
**Status:** Planning Complete - Ready for Review  
**WP1 Chunk:** 6 of 10

---

## Overview

Create the initial analysis UI layer in Phoenix with an `IAnalysisView` interface, a QCustomPlot-based stub view (`XYPlotViewQCP`), and an `AnalysisWindow` shell widget. This establishes the UI scaffolding for Sprint 4.2, ready for real data integration in WP2/WP3.

**Critical Constraint:** All builds must use Qt 6.10.0 via `-DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64`

---

## Current Codebase Analysis

### Existing Structure:

1. **Plot Infrastructure:**
   - `src/plot/QtGraphsPlotView.*` - Custom-painted plot view (not QCustomPlot)
   - Uses Qt Graphs API for rendering

2. **QCustomPlot Library:**
   - `third_party/qcustomplot/qcustomplot.h` and `.cpp`
   - CMake target: `phoenix_qcustomplot` (static library)
   - Include directory: `${CMAKE_CURRENT_SOURCE_DIR}/third_party/qcustomplot`
   - Include pattern: `#include "qcustomplot.h"`

3. **UI Structure:**
   - `src/ui/` contains UI components organized by function:
     - `main/` - MainWindow
     - `dialogs/` - Dialog widgets
     - `widgets/` - Empty (available for general widgets)
   - Analysis-specific widgets should go in new `src/ui/analysis/` directory

4. **Plot Structure:**
   - `src/plot/` contains plotting-related classes
   - `XYPlotViewQCP` should go here (plotting class)

---

## Implementation Plan

### Phase 1: Create Directory Structure

1. **Create `src/ui/analysis/` directory:**
   - For analysis-specific UI components
   - Will contain: `IAnalysisView.hpp`, `AnalysisWindow.hpp/cpp`

2. **Use existing `src/plot/` directory:**
   - For plotting-related classes
   - Will contain: `XYPlotViewQCP.hpp/cpp`

---

### Phase 2: Create IAnalysisView Interface

**File:** `src/ui/analysis/IAnalysisView.hpp`

**Design:**
- Pure virtual interface (no QObject base)
- Minimal API for widget embedding and basic operations
- No real data APIs yet (WP2/WP3)

**API:**
```cpp
#pragma once

#include <QString>

class QWidget;

class IAnalysisView {
public:
    virtual ~IAnalysisView() = default;

    // Widget that can be embedded in an AnalysisWindow or other container
    virtual QWidget* widget() = 0;

    // Optional: simple API for title / reset; we can expand later
    virtual void setTitle(const QString& title) = 0;
    virtual QString title() const = 0;

    virtual void clear() = 0;
};
```

**Notes:**
- Pure virtual (no implementation)
- Uses Qt types (`QString`, `QWidget*`)
- Minimal surface area (will expand in WP2/WP3)

---

### Phase 3: Implement XYPlotViewQCP Stub

**Files:**
- `src/plot/XYPlotViewQCP.hpp`
- `src/plot/XYPlotViewQCP.cpp`

**Design:**
- Implements `IAnalysisView`
- Owns a `QWidget` container with a `QCustomPlot` instance
- Uses `QVBoxLayout` for layout
- Stub implementation (no real data yet)

**Header (`XYPlotViewQCP.hpp`):**
```cpp
#pragma once

#include "ui/analysis/IAnalysisView.hpp"
#include <QString>
#include <memory>

class QCustomPlot;
class QWidget;

class XYPlotViewQCP : public IAnalysisView {
public:
    XYPlotViewQCP();
    ~XYPlotViewQCP() override;

    QWidget* widget() override;

    void setTitle(const QString& title) override;
    QString title() const override;

    void clear() override;

private:
    QString m_title;
    QWidget* m_container;   // parent widget container
    QCustomPlot* m_plot;    // owned by m_container via parent/child
};
```

**Implementation (`XYPlotViewQCP.cpp`):**
```cpp
#include "plot/XYPlotViewQCP.hpp"
#include "qcustomplot.h"

#include <QWidget>
#include <QVBoxLayout>

XYPlotViewQCP::XYPlotViewQCP()
    : m_container(new QWidget)
    , m_plot(new QCustomPlot(m_container))
{
    auto* layout = new QVBoxLayout(m_container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_plot);
    m_container->setLayout(layout);
}

XYPlotViewQCP::~XYPlotViewQCP() = default;

QWidget* XYPlotViewQCP::widget() {
    return m_container;
}

void XYPlotViewQCP::setTitle(const QString& title) {
    m_title = title;
    // For now, we might set the QCustomPlot's title in future; leave stubbed.
}

QString XYPlotViewQCP::title() const {
    return m_title;
}

void XYPlotViewQCP::clear() {
    if (m_plot) {
        m_plot->clearGraphs();
        m_plot->replot();
    }
}
```

**Notes:**
- QCustomPlot is owned by container via Qt parent/child
- Container widget is owned by XYPlotViewQCP
- Layout has zero margins (clean embedding)
- `clear()` removes graphs and replots (safe even with no graphs)

---

### Phase 4: Implement AnalysisWindow Shell

**Files:**
- `src/ui/analysis/AnalysisWindow.hpp`
- `src/ui/analysis/AnalysisWindow.cpp`

**Design:**
- `QWidget` subclass (not yet docked/integrated into main window)
- Owns exactly one `IAnalysisView` at a time
- Uses `QVBoxLayout` for layout
- Manages view lifecycle (setView, remove old view)

**Header (`AnalysisWindow.hpp`):**
```cpp
#pragma once

#include <QWidget>
#include <memory>

class IAnalysisView;
class QVBoxLayout;

class AnalysisWindow : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisWindow(QWidget* parent = nullptr);
    ~AnalysisWindow() override;

    void setView(std::unique_ptr<IAnalysisView> view);
    IAnalysisView* view() const;

private:
    std::unique_ptr<IAnalysisView> m_view;
    QVBoxLayout* m_layout;
};
```

**Implementation (`AnalysisWindow.cpp`):**
```cpp
#include "ui/analysis/AnalysisWindow.hpp"
#include "ui/analysis/IAnalysisView.hpp"

#include <QVBoxLayout>

AnalysisWindow::AnalysisWindow(QWidget* parent)
    : QWidget(parent)
    , m_view(nullptr)
    , m_layout(new QVBoxLayout(this))
{
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);
}

AnalysisWindow::~AnalysisWindow() = default;

void AnalysisWindow::setView(std::unique_ptr<IAnalysisView> view) {
    // Remove old widget, if any
    if (m_view && m_view->widget()) {
        m_layout->removeWidget(m_view->widget());
        m_view->widget()->setParent(nullptr);
    }

    m_view = std::move(view);

    if (m_view && m_view->widget()) {
        m_layout->addWidget(m_view->widget());
    }
}

IAnalysisView* AnalysisWindow::view() const {
    return m_view.get();
}
```

**Notes:**
- Uses `std::unique_ptr` for ownership
- Properly removes old view widget before adding new one
- Layout has zero margins (clean embedding)
- No menu integration yet (WP2/WP3)

---

### Phase 5: CMake Integration

**Add `phoenix_analysis` library target to `CMakeLists.txt`:**

Place after `phoenix_transport` library (around line 295):

```cmake
# ---- analysis library ----------------------------------------------------
add_library(phoenix_analysis STATIC
  src/ui/analysis/IAnalysisView.hpp
  src/ui/analysis/AnalysisWindow.cpp
  src/ui/analysis/AnalysisWindow.hpp
  src/plot/XYPlotViewQCP.cpp
  src/plot/XYPlotViewQCP.hpp
)

target_include_directories(phoenix_analysis PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(phoenix_analysis PUBLIC
  Qt6::Core
  Qt6::Widgets
  phoenix_qcustomplot
)
```

**Optional: Link phoenix_app to phoenix_analysis (but don't use yet):**

In the `phoenix_app` target_link_libraries section (around line 350), add:
```cmake
target_link_libraries(phoenix_app
  PRIVATE
    ...
    phoenix_analysis  # Added but not used yet
    ...
)
```

**Notes:**
- Static library (matches other Phoenix libraries)
- Links Qt6::Core, Qt6::Widgets, and phoenix_qcustomplot
- Public include directory: `${CMAKE_CURRENT_SOURCE_DIR}/src`
- Optional link to phoenix_app (for future use)

---

### Phase 6: QtTest Sanity Tests

**File:** `tests/analysis_sanity_tests.cpp`

**Test Cases:**
1. `testAttachXYPlotView()` - Verify AnalysisWindow can hold XYPlotViewQCP
2. `testClearDoesNotCrash()` - Verify clear() doesn't crash

**Implementation:**
```cpp
#include <QtTest/QtTest>
#include "ui/analysis/AnalysisWindow.hpp"
#include "plot/XYPlotViewQCP.hpp"

class AnalysisSanityTests : public QObject {
    Q_OBJECT

private slots:
    void testAttachXYPlotView() {
        AnalysisWindow window;
        auto view = std::make_unique<XYPlotViewQCP>();
        IAnalysisView* raw = view.get();
        
        window.setView(std::move(view));
        
        QVERIFY(window.view() == raw);
        QVERIFY(window.view()->widget() != nullptr);
    }

    void testClearDoesNotCrash() {
        XYPlotViewQCP view;
        view.clear(); // Should not crash or throw
        QVERIFY(true);
    }
};

QTEST_MAIN(AnalysisSanityTests)
#include "analysis_sanity_tests.moc"
```

**Wire into `tests/CMakeLists.txt`:**

Add after `transport_sanity_tests` section:

```cmake
# analysis sanity tests
if(BUILD_TESTING)
  add_executable(analysis_sanity_tests
    analysis_sanity_tests.cpp
  )

  target_link_libraries(analysis_sanity_tests
    PRIVATE
      phoenix_analysis
      Qt6::Test
      Qt6::Core
      Qt6::Widgets
  )

  target_include_directories(analysis_sanity_tests
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME analysis_sanity_tests COMMAND analysis_sanity_tests)
endif()
```

**Notes:**
- QtTest framework
- Tests widget embedding and clear() safety
- Links phoenix_analysis, Qt6::Test, Qt6::Core, Qt6::Widgets

---

## File Structure

### Files to Create:

```
src/ui/analysis/
├── IAnalysisView.hpp          # Pure virtual interface
├── AnalysisWindow.hpp          # Analysis window shell
└── AnalysisWindow.cpp          # Analysis window implementation

src/plot/
├── XYPlotViewQCP.hpp          # QCustomPlot-based view header
└── XYPlotViewQCP.cpp          # QCustomPlot-based view implementation

tests/
└── analysis_sanity_tests.cpp  # QtTest sanity tests
```

### Files to Modify:

- `CMakeLists.txt` - Add phoenix_analysis library target
- `tests/CMakeLists.txt` - Add analysis_sanity_tests

---

## Implementation Steps

1. **Create directory structure:**
   - Create `src/ui/analysis/` directory
   - Verify `src/plot/` directory exists

2. **Create IAnalysisView interface:**
   - Write `src/ui/analysis/IAnalysisView.hpp`
   - Pure virtual interface with minimal API

3. **Create XYPlotViewQCP stub:**
   - Write `src/plot/XYPlotViewQCP.hpp`
   - Write `src/plot/XYPlotViewQCP.cpp`
   - Implement stub methods (no real data)

4. **Create AnalysisWindow shell:**
   - Write `src/ui/analysis/AnalysisWindow.hpp`
   - Write `src/ui/analysis/AnalysisWindow.cpp`
   - Implement view management

5. **Add CMake integration:**
   - Add `phoenix_analysis` library target to CMakeLists.txt
   - Optionally link phoenix_app to phoenix_analysis (but don't use yet)

6. **Create sanity tests:**
   - Write `tests/analysis_sanity_tests.cpp`
   - Wire into tests/CMakeLists.txt

7. **Verify build:**
   - Configure with Qt 6.10.0
   - Build phoenix_analysis library
   - Build and run analysis_sanity_tests
   - Verify all tests pass

---

## Verification Checklist

- [ ] `src/ui/analysis/` directory created
- [ ] `IAnalysisView.hpp` created (pure virtual interface)
- [ ] `XYPlotViewQCP.hpp/cpp` created (QCustomPlot-based stub)
- [ ] `AnalysisWindow.hpp/cpp` created (shell widget)
- [ ] `phoenix_analysis` library target added to CMakeLists.txt
- [ ] `analysis_sanity_tests.cpp` created
- [ ] Test wired into tests/CMakeLists.txt
- [ ] Local build succeeds with Qt 6.10.0
- [ ] Analysis sanity tests pass
- [ ] No regressions or warnings
- [ ] CI passes on all platforms

---

## Design Decisions

### ✅ Resolved:

1. **Directory Structure:**
   - `src/ui/analysis/` for analysis UI components
   - `src/plot/` for plotting classes

2. **Interface Design:**
   - Pure virtual (no QObject base)
   - Minimal API (widget, title, clear)
   - Will expand in WP2/WP3

3. **QCustomPlot Integration:**
   - Use existing `phoenix_qcustomplot` library
   - Include via `#include "qcustomplot.h"`
   - Owned via Qt parent/child relationship

4. **AnalysisWindow Design:**
   - QWidget subclass (not QMainWindow)
   - Owns one IAnalysisView via unique_ptr
   - Uses QVBoxLayout for embedding

5. **CMake Integration:**
   - Static library (matches Phoenix style)
   - Links Qt6::Core, Qt6::Widgets, phoenix_qcustomplot
   - Optional link to phoenix_app (for future use)

### ❓ Open Questions:

1. **Include Paths:** Should `IAnalysisView.hpp` be included as `"ui/analysis/IAnalysisView.hpp"` or `"IAnalysisView.hpp"`?
   - **Recommendation:** Use `"ui/analysis/IAnalysisView.hpp"` (matches existing patterns like `"plot/QtGraphsPlotView.hpp"`)

2. **AnalysisWindow Q_OBJECT:** Should AnalysisWindow use Q_OBJECT macro?
   - **Recommendation:** Yes, for future signal/slot support (even if not used yet)

3. **Widget Ownership:** Should AnalysisWindow take ownership of the view widget, or just reference it?
   - **Recommendation:** Take ownership via `std::unique_ptr` (as specified in prompt)

4. **Clear Implementation:** Should `clear()` on XYPlotViewQCP also clear axes/title?
   - **Recommendation:** Just clear graphs for now (minimal stub), can expand later

---

## Expected Outcomes

### Files Created:
- `src/ui/analysis/IAnalysisView.hpp`
- `src/ui/analysis/AnalysisWindow.hpp`
- `src/ui/analysis/AnalysisWindow.cpp`
- `src/plot/XYPlotViewQCP.hpp`
- `src/plot/XYPlotViewQCP.cpp`
- `tests/analysis_sanity_tests.cpp`

### Files Modified:
- `CMakeLists.txt` - Added phoenix_analysis library target
- `tests/CMakeLists.txt` - Added analysis_sanity_tests

### Build Output:
- `phoenix_analysis` static library builds successfully
- `analysis_sanity_tests` executable builds and passes
- All platforms (macOS, Linux, Windows) build successfully

---

## Summary

This chunk establishes the analysis UI scaffolding by:
1. **Creating IAnalysisView interface** - Pure virtual interface for analysis views
2. **Implementing XYPlotViewQCP stub** - QCustomPlot-based view (no real data yet)
3. **Creating AnalysisWindow shell** - Widget container for IAnalysisView
4. **Adding CMake integration** - phoenix_analysis library target
5. **Adding sanity tests** - QtTest-based verification

The result is a clean, minimal analysis UI layer ready for WP2/WP3 data integration.

---

**Ready for review and approval before implementation.**

