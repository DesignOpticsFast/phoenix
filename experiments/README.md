# Experiments Directory

This directory contains experimental, abandoned, or reference implementations that are not included in the Phoenix build. These files are preserved for historical reference or future consideration.

## Structure

### `analysis/`
- **XYWindow.cpp / XYWindow.hpp** (2025-10-23)
  - Experimental analysis window with Palantir integration. Contains leaky dialog patterns (dialogs created without proper ownership). Not in build; moved during Sprint 4 dialog cleanup.

### `ui/`
- **EnvironmentPrefs.cpp / EnvironmentPrefs.hpp** (2025-10-23)
  - Standalone environment preferences dialog. Superseded by `PreferencesDialog` in the active build. Not referenced in current MainWindow implementation.

### `ui/main/`
- **MainWindow_broken.cpp** (2025-10-23)
  - Backup/experimental MainWindow variant with known issues.
- **MainWindow_crash.cpp** (2025-10-23)
  - Backup/experimental MainWindow variant that caused crashes.
- **MainWindow_fixed.cpp** (2025-10-23)
  - Backup/experimental MainWindow variant with fixes applied.
- **MainWindow_minimal.cpp** (2025-10-23)
  - Minimal MainWindow implementation for testing.
- **MainWindow_original.cpp** (2025-10-23)
  - Original MainWindow implementation backup.
- **MainWindow_safe.cpp** (2025-10-23)
  - Backup/experimental MainWindow variant with safety improvements.
- **MainWindow_simple.cpp** (2025-10-23)
  - Simplified MainWindow implementation.
- **MainWindow_working.cpp** (2025-10-23)
  - Backup/experimental MainWindow variant that was working.

**Note:** The canonical MainWindow implementation is `src/ui/main/MainWindow.cpp/h`.

## Notes

- These files are **not** compiled as part of the Phoenix build.
- They are preserved for reference, debugging, or potential future use.
- If you need to reference or restore any of these, check git history for the original location and context.

