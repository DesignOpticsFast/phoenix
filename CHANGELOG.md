# DesignOpticsFast Changelog

All notable changes to the **Phoenix** and **Bedrock** projects are documented here.  
This changelog follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) conventions and uses
[Semantic Versioning](https://semver.org/).

---

## [Unreleased] – Sprint 4
### Phoenix (Frontend)
- **Icon System Refactor (Sprint 4)**: Font Awesome glyph refactor complete; SVGs limited to branding. All toolbar/menu icons now use FA glyphs through IconProvider with theme-aware tinting. Removed FA UI chrome SVG duplicates from QRC. Added CI pre-build checks for QRC path validation and SVG currentColor enforcement.
- **Toolbar Floating Z-Order**: Floating toolbars and dock widgets now stay above the Phoenix MainWindow when floated. Tool/dock windows remain above their parent window (not globally) and automatically raise when MainWindow regains focus. Re-docking clears window flags for normal docking behavior.
- **Dialog Cleanup**: Removed abandoned dialogs (XYWindow, EnvironmentPrefs, MainWindow variants) and moved them to `experiments/` directory. Fixed PreferencesDialog lifetime management by replacing `std::unique_ptr` with `QPointer` and Qt parent ownership. Added `Qt::WA_DeleteOnClose` for automatic cleanup. Dialog now raises/activates when reopened instead of creating duplicates.
- **Settings Provider Injection**: Replaced scattered QSettings instances with a single injected SettingsProvider. Created SettingsProvider class wrapping QSettings, created in main() and parented to QApplication. All components (MainWindow, PreferencesDialog, ThemeManager, etc.) now receive QSettings& via dependency injection. Standardized settings keys with PhxKeys namespace constants (ui/theme, ui/geometry, i18n/language, etc.). Fixed LanguagePage app name inconsistency (now uses "Phoenix" instead of "PhoenixApp").
- **I/O Hardening**: Added FileIO utility namespace with safe helpers for file operations (canonicalize, ensureDir, readTextFile, writeTextFileAtomic). Hardened ThemeManager::loadStyleSheet() and IconProvider::loadManifest() with explicit error handling and logging. Added save/export scaffolding to MainWindow with path validation. All file operations now use centralized error handling; failures are logged with clear messages and do not crash the application.
- **Constants Header & Plotting Stub**: Created `PhxConstants.h` with centralized constants for UI sizes, timers, and plotting parameters. Replaced magic numbers throughout MainWindow and PreferencesDialog with named constants (window sizes, icon sizes, dock widths, performance thresholds). Added plotting stub to QtGraphsPlotView that logs a deferred warning; plotting implementation deferred to Sprint 5.
- **Assets & Licensing Cleanup**: Removed Font Awesome UI chrome SVG duplicates (using glyphs instead). Normalized QRC file paths and organized resources into separate prefixes (/fonts, /icons). Added Font Awesome Pro license file at `assets/icons/fontawesome-pro/LICENSE.txt`. Created `docs/LICENSING.md` documenting asset licensing. Verified About dialog uses FA glyph icon that tints correctly with theme.

---

## [v0.1.0-sprint1] – 2025-10-08
### Status
**Sprint 1 Complete — Baseline Release**

This release closes Sprint 1 and establishes the first cross-platform stable baseline of the DesignOpticsFast toolchain.

### Phoenix (Frontend)
- Implemented lifecycle-correct Linux file-dialog handling  
  – Pre-app environment detection (`PlatformGuard`) for Wayland, Flatpak/Snap, VirtualBox.  
  – Runtime D-Bus portal health check post-`QApplication`.  
  – Automatic fallback to Qt’s non-native dialogs when portals are unavailable.  
  – CLI flag `--force-fallback-dialogs` for CI/headless runs.  
- Verified dialog stability across:
  - Ubuntu 24.04 Wayland (native portal path)  
  - Ubuntu 24.04 VirtualBox (XCB forced)  
  - CI (headless `xvfb-run`)  
  - Windows 11 and macOS Sonoma (unchanged)  
- STEP export confirmed functional in all Linux variants.  
- Updated CMake to link `Qt6::DBus`; added `$ORIGIN` RPATH for Linux.  
- Introduced structured logging (`qInfo/qWarning`) for platform decisions.

### Bedrock (Engine)
- Ensured compatibility with OCCT 7.7 and TBB linking on Ubuntu 24.04.  
- Verified STEP export and inter-process communication with Phoenix.  
- No functional changes required for Sprint 1 code; environment stabilization only.

### Infrastructure / Tooling
- Linux VM re-provisioned with:
  - Qt 6.10.1 via aqtinstall  
  - `xdg-desktop-portal` stack installed  
  - Guest Additions (3D accel, bidirectional clipboard)  
- Build validated via CMake + Ninja on AWS Ubuntu 24.04 and VirtualBox Ubuntu 24.04.  
- Repository hygiene:
  - Branches merged and closed  
  - Tags `v0.1.0-sprint1` pushed for Phoenix and Bedrock  

### Summary
> **Result:** Phoenix and Bedrock build and run cleanly on Linux, Windows, and macOS.  
> **Next Step:** Begin Sprint 2 (UnderLord Integration and Automated AI Development Workflow).

---
