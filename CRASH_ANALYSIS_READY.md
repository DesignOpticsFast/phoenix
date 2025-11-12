# Crash Analysis - Ready for Backtrace

## Potential Crash Sites Identified

Based on code review, here are the most likely crash locations:

### 1. Deferred Lambda (MainWindow.cpp:147-180)
**Location**: `QTimer::singleShot(0, this, [this]() { ... })`
**Potential issues**:
- Line 149: `safeIconSizeHint()` → `QApplication::style()` might not be ready
- Line 152: `refreshThemeActionIcons(startupIconSize)` → calls `host->style()->pixelMetric()` 
- Line 156: `applyRibbonPalette(m_rightRibbon)` → calls `target->style()`
- Line 161: `m_themeManager->currentTheme()` → ThemeManager might emit signals too early
- Line 174: `updateStatusBar()` → calls `updateDebugInfo()` (but has guards)

### 2. refreshThemeActionIcons() (MainWindow.cpp:1430-1490)
**Potential issues**:
- Line 1451: `host->style()->pixelMetric(...)` - if `host` (m_themeMenu) not realized
- Line 1482: `m_rightRibbon->widgetForAction(action)` - if ribbon/widget not ready
- Line 1484: `button->setIcon(...)` - if button not fully constructed

### 3. applyRibbonPalette() (MainWindow.cpp:1503-1543)
**Potential issues**:
- Line 1533: `target->style()` - if target not fully realized on macOS
- Line 1534-1535: `st->unpolish()` / `st->polish()` - if style not ready
- Line 1540: `p->setPalette(...)` - if parent not ready

### 4. wireSideRibbonAction() (MainWindow.cpp:1601-1624)
**Potential issues**:
- Line 1611: `m_rightRibbon->widgetForAction(action)` - if ribbon not ready
- Line 1617: `button->setIconSize(m_rightRibbon->iconSize())` - if ribbon not ready

### 5. updateDebugInfo() (MainWindow.cpp:992-1074)
**Has guards**: Line 995 checks all labels
**But**: Could still crash if called before guards are set, or if labels are partially constructed

## Analysis Plan

Once we have `/tmp/phoenix_crash_full.txt`:

1. **Find first Phoenix frame** in backtrace
2. **Map to exact line** in current code
3. **Identify root cause**:
   - Null pointer dereference?
   - Widget not fully realized?
   - Style not ready?
   - ThemeManager signal too early?
4. **Propose surgical fix** tied directly to the crash site

## Key Questions to Answer

- Is the crash in the deferred lambda or before it runs?
- Is it a widget realization issue (macOS-specific)?
- Is it a style/palette access before widget is ready?
- Is ThemeManager emitting signals during construction?

## Next Steps

1. Wait for Mark to provide `/tmp/phoenix_crash_full.txt`
2. Analyze backtrace to find exact crash site
3. Propose minimal surgical fix
4. Verify on macOS

