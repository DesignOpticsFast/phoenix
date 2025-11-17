# Phoenix Sprint 4.1 – Stable Shell & Platform Cohesion

## Overview

Sprint 4.1 focused on stabilizing Phoenix's core shell infrastructure, improving cross-platform consistency, and establishing a solid foundation for future development. This sprint addressed critical macOS startup crashes, implemented robust restart workflows, refined layout management, completed internationalization, and established a pragmatic Linux platform strategy. The result is a more reliable, cohesive application that behaves consistently across macOS, Windows, and Linux.

The sprint delivered significant improvements in application stability, user experience, and developer confidence. All major features have been validated across all three platforms, with particular attention paid to macOS-specific edge cases and Linux platform limitations.

## Key Outcomes

- **Zero startup crashes** on macOS after fixing deferred initialization issues
- **Complete i18n integration** with English and German translations working across all UI surfaces
- **Robust restart workflow** with "Restart Now" and "Restart Later" options
- **Canonical layout system** that reliably restores and resets application state
- **Linux platform strategy** that defaults to XWayland for stable docking behavior
- **Theme consistency** across menus and ribbons with proper state synchronization
- **Startup timing** accurately measured and displayed to users

## Shell & Stability Improvements

Sprint 4.1 began with critical stability fixes for macOS. The application was experiencing startup crashes related to early access to UI components before they were fully initialized. We implemented comprehensive guards around theme refresh operations, deferred status bar updates until the window is shown, and ensured all QToolBar operations wait until the UI is ready.

The shell initialization sequence was redesigned to follow a clear, predictable order: application setup → splash screen → icon initialization → theme setup → main window creation → UI finalization. This sequence eliminates race conditions and null pointer dereferences that were causing crashes on macOS.

Status bar semantics were finalized with proper "Starting..." → "Ready" transitions, and startup timing is now accurately measured from splash screen display to application ready state. The timing information is displayed in the status bar, giving users visibility into application performance.

## Restart UX & Startup Timing

A complete restart workflow was implemented, addressing a long-standing gap in Phoenix's lifecycle management. The restart dialog provides clear "Restart Now" and "Restart Later" options, with proper modal behavior and window resolution handling.

On macOS, we implemented platform-specific restart logic that properly relaunches the application bundle. The restart mechanism respects the application's lifecycle, ensuring settings are saved before restart and the application state is properly restored after relaunch.

Startup timing was refactored to provide accurate measurements. The timing system captures the moment the splash screen is shown and tracks through to when the main window signals it's ready. This timing is displayed in the status bar, giving users and developers insight into application startup performance.

## Layout & Docking Behavior

The canonical layout system was implemented to provide a reliable baseline for application state. When docks become invisible after restore (a known Qt limitation), Phoenix now detects this condition and falls back to the canonical layout automatically.

Layout persistence was improved to reliably save and restore toolbar and dock positions. The factory reset functionality works correctly, restoring the application to its default layout state. This provides users with a reliable way to recover from layout corruption or experimentation.

Dock restoration logic was hardened to handle edge cases where Qt's restoreState() might leave docks in an invisible state. The detection and fallback mechanism ensures users always have a usable interface, even when layout restoration encounters issues.

## Internationalization (EN/DE)

Complete internationalization was implemented for Sprint 4.1, with English and German translations covering all UI surfaces: menus, splash screen, dialogs, status bar, and window titles. The translation system works correctly across all platforms, with proper locale detection and runtime language switching.

Early `tr()` calls in the splash screen constructor were fixed, ensuring translations are available when needed. The i18n system properly handles the application lifecycle, loading translations before UI components are created.

German translations were added for all Sprint 4.1 UI strings, providing a complete localized experience. The translation files are properly maintained, with line numbers updated after code refactoring to keep the translation system in sync with the codebase.

## Splash & Themes

The splash screen was improved with proper message initialization and translation support. The splash screen now displays localized messages correctly and provides accurate feedback during application startup.

Theme action consistency was enforced across menus and ribbons. A single-checked theme action system ensures that theme selections sync properly between the menu bar and side ribbon. Theme changes propagate correctly throughout the UI, with palette-aware rendering on macOS.

The side ribbon was refined with proper alignment, palette synchronization, and theme-aware rendering. Theme buttons use subtle styling that matches the overall application aesthetic while providing clear visual feedback.

## Linux Platform Strategy (XWayland Default)

Phoenix now defaults to X11/XWayland (`QT_QPA_PLATFORM=xcb`) on Linux to avoid Qt/Wayland docking limitations. Native Wayland docking in Qt is currently limited and produces warnings like "This plugin supports grabbing the mouse only for popup windows" when dragging toolbars or docks.

We removed `raise()` calls that were triggering popup-only mouse grab warnings on Wayland. These calls were attempting to force floating toolbars and docks to the front, but Qt's default z-ordering handles floating widgets correctly on all platforms without explicit `raise()` calls.

Advanced users can force native Wayland by setting `PHOENIX_FORCE_WAYLAND=1` before launching Phoenix, but docking behavior may be constrained on some compositors. This is a temporary, pragmatic choice until Qt's Wayland docking support improves.

The Linux platform strategy prioritizes functionality and user experience over native Wayland support. XWayland provides a stable, well-tested path that works reliably across all Linux distributions and desktop environments.

## Known Issues / Deferred Work

The following items were identified during Sprint 4.1 but deferred to future sprints:

**To Sprint 4.1.1 (Patch Release):**
- None identified at this time

**To Sprint 4.2 / Sprint 5:**
- **File I/O Implementation**: Complete implementation of file save/export functionality with proper path validation and error handling
- **Plotting Pipeline**: Full implementation of plotting functionality using QtGraphs (currently stubbed)
- **MainWindow Segmentation**: Refactor MainWindow into smaller, more maintainable components
- **Unit Test Scaffolding**: Establish unit test framework and add coverage for critical components
- **Performance Optimization**: Address any performance bottlenecks identified during startup timing measurements

## Next Steps

Sprint 4.2 will focus on building out core functionality now that the shell is stable. The plotting pipeline will be a primary focus, along with file I/O implementation. The stable foundation established in Sprint 4.1 provides a solid base for these features.

The internationalization infrastructure is now in place and can be extended to additional languages as needed. The layout system is reliable and can support more complex UI arrangements as features are added.

The Linux platform strategy provides a clear path forward, and we'll continue to monitor Qt's Wayland improvements for future native Wayland support when the docking limitations are resolved.

