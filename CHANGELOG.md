# DesignOpticsFast Changelog

All notable changes to the **Phoenix** and **Bedrock** projects are documented here.  
This changelog follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) conventions and uses
[Semantic Versioning](https://semver.org/).

---

## [0.0.4] – 2025-01-26

**Sprint 4.5 Fixup** — Envelope Protocol & IPC Hardening

This release represents a foundational overhaul of the Phoenix ↔ Bedrock IPC layer, establishing a stable, well-tested, and well-documented foundation for future development.

### Added

#### Transport & Protocol
- **Envelope-Based Palantir Protocol**: Migrated to envelope-based wire format for all IPC communication. All messages now use `MessageEnvelope` protobuf with version, type, payload, and metadata fields. Wire format: `[4-byte length][serialized MessageEnvelope]`. See `ADR-0002-Envelope-Based-Palantir-Framing.md` for details.
- **Envelope Helpers**: Added reusable `makeEnvelope()` and `parseEnvelope()` helpers in both Phoenix and Bedrock for creating and parsing envelope messages. Helpers include version validation, type checking, and error reporting.
- **XY Sine RPC Client**: Implemented envelope-based XY Sine RPC client in Phoenix, enabling remote computation of XY sine wave analysis via Bedrock.

#### Error Handling
- **Centralized Error Mapping**: Added `mapErrorResponse()` function in Phoenix to provide consistent, user-friendly error messages from Bedrock's standardized error codes.
- **Standardized Error Codes**: Normalized error semantics across Phoenix and Bedrock with consistent error codes: `INTERNAL_ERROR`, `MESSAGE_TOO_LARGE`, `INVALID_MESSAGE_FORMAT`, `PROTOBUF_PARSE_ERROR`, `UNKNOWN_MESSAGE_TYPE`, `INVALID_PARAMETER_VALUE`.

#### Testing & CI
- **Envelope Helper Tests**: Comprehensive unit tests for envelope encoding/decoding, version validation, metadata round-trip, and malformed data handling in both Phoenix and Bedrock.
- **Error Mapping Tests**: Unit tests for error response encoding/decoding and client-side error mapping.
- **Integration Test Suite**: Complete integration test coverage including:
  - Capabilities RPC end-to-end
  - XY Sine RPC end-to-end with mathematical correctness validation
  - Error cases (unknown type, oversize messages, malformed payload, invalid version, invalid parameters)
  - Edge cases (multiple concurrent clients, rapid connect/disconnect, large/minimal valid requests, mixed-mode sequences)
- **CI Integration**: GitHub Actions workflows now run unit and integration tests on Linux and macOS for both Phoenix and Bedrock.
- **Coverage Reporting**: Code coverage measurement integrated into Linux CI using gcov/lcov, generating HTML reports as artifacts. Focus on Palantir-related code with ~80% coverage target.

#### Documentation
- **VERSIONS.md**: Created single source of truth for toolchain versions (C++ standard, Qt, CMake, Protobuf) in both Phoenix and Bedrock.
- **Documentation Harmonization**: Comprehensive documentation updates across Phoenix, Bedrock, and Palantir:
  - Removed all stale dev-01 references
  - Updated protocol descriptions to envelope-based
  - Consolidated duplicate documentation
  - Created comprehensive architecture, build, testing, and deployment guides
- **Threading Documentation**: Complete threading model documentation for Bedrock, covering PalantirServer, OpenMP/TBB usage, and concurrency guidelines.

### Changed

#### Transport & IPC
- **IPC Transport Refactor**: Complete migration from legacy `[length][type][payload]` format to envelope-only framing. All RPCs now use envelope-based transport with proper versioning and extensibility.
- **Message Size Limits**: Implemented 10 MB message size limit enforced on both client and server, with clear error messages for oversize messages.
- **Input Validation**: Added comprehensive input validation at RPC boundaries in Bedrock, ensuring invalid parameters fail early with clear error codes.

#### Concurrency & Validation
- **IPC Hardening**: Eliminated deadlocks in transport layer by refactoring lock scope. Narrowed mutex protection to buffer manipulation only, with dispatch and I/O operations outside critical sections. Transport layer now safe under concurrency.
- **Error Semantics**: Normalized error response generation and mapping across Phoenix and Bedrock for consistent, user-meaningful error handling.

#### Documentation
- **Documentation Overhaul**: Complete documentation refresh across Phoenix, Bedrock, and Palantir:
  - Phoenix: Removed dev-01 workflow, updated to Crucible + CI model, protocol descriptions updated
  - Palantir: Clear separation of current (envelope-based) vs future (Arrow Flight, gRPC) transports
  - Bedrock: Comprehensive architecture, build, testing, deployment, and repository structure documentation
- **Toolchain Normalization**: All toolchain version references now point to VERSIONS.md instead of hard-coding versions.

#### Toolchain
- **Qt Baseline Verification**: Qt 6.10.1 verified and documented as baseline for Phoenix development and CI.

### Fixed

#### Protocol & Documentation
- **Legacy Protocol Descriptions**: Replaced all legacy "PLTR magic" protocol descriptions with accurate envelope-based protocol documentation.
- **Stale Documentation**: Fixed multiple stale documentation issues:
  - Removed dev-01 workflow references
  - Removed demo-mode references (clarified as local compute)
  - Updated Qt version references to use VERSIONS.md
  - Sanitized personal paths in documentation
- **Transport Description Consistency**: Ensured Phoenix, Bedrock, and Palantir docs consistently describe current (envelope-based) vs future (Arrow Flight, gRPC) transports.

#### Error Handling
- **Error Semantics Consistency**: Fixed inconsistent error handling between Phoenix and Bedrock. Error responses now use standardized codes and messages consistently.

### Removed

#### Legacy Code
- **Legacy Protocol Format**: Removed all legacy `[length][type][payload]` header-based protocol code. Bedrock is now envelope-only.
- **Old dev-01 Content**: Removed all dev-01 workflow documentation and references from active docs.
- **Demo-Mode References**: Removed misleading "demo mode" references from documentation. Clarified that `XYSineDemo` is legitimate local compute, not a demo mode.

---

### Bedrock (Backend)

#### Added
- **Envelope-Based Palantir Transport**: Server-side migration to envelope-based protocol matching Phoenix client. All message handling now uses `MessageEnvelope` with proper validation.
- **Integration Test Harness**: Comprehensive integration test framework with in-process server fixture and minimal test client. Validates end-to-end envelope transport for RPCs.
- **Input Validation**: RPC boundary validation for all request types, ensuring invalid inputs fail early with clear error codes.
- **Threading Documentation**: Complete threading model documentation covering PalantirServer, OpenMP/TBB usage patterns, and concurrency guidelines.

#### Changed
- **Deadlock Elimination**: Fixed deadlock in `parseIncomingData()` by refactoring to `extractMessage()` helper (no locking) and narrowing lock scope. Removed mutex from `sendMessage()` to prevent nested locking.
- **Error Response Standardization**: Centralized error response generation via `sendErrorResponse()` helper with normalized error codes.

#### Fixed
- **C++ Standard Consistency**: Fixed inconsistency where `bedrock_core` was explicitly set to C++17, overriding the top-level C++20 setting. All Bedrock code now consistently builds with C++20.

---

**For detailed implementation notes, see `bedrock/docs/sprint4.5/` directory.**

---

## [0.0.3] – 2025-01-XX
### Phoenix (Frontend)
- **PalantirClient Async FSM**: Non-blocking connection state machine with exponential backoff (1s, 2s, 4s, 8s, 16s). Connection states: Idle → Connecting → Connected → ErrorBackoff → PermanentFail. All socket operations are event-driven; GUI thread never blocks.
- **Framed Protocol Header**: Binary protocol with PLTR magic (0x504C5452), BigEndian byte order, and 12-byte header (magic: u32, version: u16, type: u16, length: u32). Protocol validation with version range checks and 8 MiB payload size limit. Malformed frames trigger immediate disconnect and backoff.
- **Message Dispatcher Scaffold**: Type→handler registration system for protocol frames. Handlers are called after `messageReceived()` signal emission, allowing both signal/slot and callback patterns. Unregistered message types log debug output.
- **Version 0.0.3 Display**: Version now appears in splash screen, main window title ("Phoenix 0.0.3 - Optical Design Studio"), and About dialog. All locations use single source of truth from CMake `PROJECT_VERSION`.
- **Prototype Removal**: Removed STEP prototype code and Bedrock adapter. Deleted obsolete files: MainWindow.cpp/hpp/ui (prototype), StepViewer.cpp/hpp, bedrock_client.cpp/hpp, noop.cpp. Cleaned CMakeLists.txt references. No Bedrock/STEP references remain in Phoenix codebase.
- **Sprint 4.1 – Shell Stability & Platform Cohesion**:
  - **Shell Stability & macOS Crash Fixes**: Fixed macOS startup crashes related to early status bar access and deferred initialization. Guarded theme refresh until UI is ready, preventing null pointer dereferences during startup.
  - **Restart Now / Restart Later UX**: Implemented robust restart dialog with "Restart Now" and "Restart Later" options. Added macOS-specific restart relaunch logic. Dialog properly wires to application lifecycle and handles window resolution/modal behavior.
  - **Startup Timing Refactor**: Real startup timing measurement from splash screen display to application ready state. Status bar semantics finalized with proper "Starting..." → "Ready" transition. Startup duration displayed in status bar.
  - **Canonical Layout & Dock Restore Fixes**: Implemented canonical layout detection and restoration. Fixed invisible dock detection after restore, falling back to canonical layout when needed. Layout persistence and factory reset functionality working correctly.
  - **Internationalization (EN/DE)**: Complete i18n integration for menus, splash screen, dialogs, status bar, and window titles. German translations added for all Sprint 4.1 UI strings. Fixed early `tr()` calls in splash screen constructor. Translation system working across all platforms.
  - **Theme Action Consistency**: Enforced single-checked theme action across menus and ribbons. Theme actions sync properly between menu bar and side ribbon. Palette-aware ribbon rendering on macOS. Theme changes propagate correctly throughout UI.
  - **Linux XWayland Default**: Phoenix now defaults to X11/XWayland (`QT_QPA_PLATFORM=xcb`) on Linux to avoid Qt/Wayland docking limitations. Removed `raise()` calls that triggered popup-only mouse grab warnings. Advanced users can force native Wayland via `PHOENIX_FORCE_WAYLAND=1` environment variable.

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
- **Production Defaults**: Finalized production form for icon system. Guarded startup probes and diagnostics behind `PHX_DEV_DIAG` compile-time flag. Downgraded routine logs from `qCInfo` to `qCDebug`. Removed unconditional cache clearing at startup (only clears when `PHX_ICON_NOCACHE` is set). Added `src/app/BuildFlags.h` for discoverable developer diagnostics. Created diagnostics documentation in `docs/DEVELOPMENT.md` covering environment variables (`PHX_ICON_NOCACHE`, `PHX_ICON_DEBUG_OUTLINE`), logging rules, and compile-time flags. Icon system now runs quietly by default with minimal runtime overhead.

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
