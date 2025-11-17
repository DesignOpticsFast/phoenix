# Phoenix Sprint 4.1 – Stable Shell Release

## Highlights

- **Stable startup** across macOS, Windows, and Linux
- **Complete internationalization** with English and German translations
- **Improved restart workflow** with clear user options
- **Reliable layout management** with automatic fallback to canonical layout
- **Linux platform improvements** with XWayland default for stable docking

## UX & Stability

- Fixed macOS startup crashes related to early UI access
- Implemented robust restart dialog with "Restart Now" and "Restart Later" options
- Added accurate startup timing measurement and display
- Improved layout persistence and factory reset functionality
- Enhanced status bar with proper "Starting..." → "Ready" transitions

## Internationalization

- Complete i18n integration for menus, splash screen, dialogs, status bar, and window titles
- German translations added for all UI strings
- Runtime language switching supported
- Translations work correctly across all platforms

## Linux Behavior

- Phoenix now defaults to X11/XWayland on Linux for stable toolbar and dock dragging
- Advanced users can force native Wayland via `PHOENIX_FORCE_WAYLAND=1` environment variable
- Resolved Wayland docking warnings and drag behavior issues

## Technical Improvements

- Theme action consistency across menus and ribbons
- Improved splash screen with localized messages
- Enhanced side ribbon with proper alignment and palette synchronization
- Canonical layout system with automatic fallback for invisible docks

