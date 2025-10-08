# DesignOpticsFast Changelog

All notable changes to the **Phoenix** and **Bedrock** projects are documented here.  
This changelog follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) conventions and uses
[Semantic Versioning](https://semver.org/).

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
