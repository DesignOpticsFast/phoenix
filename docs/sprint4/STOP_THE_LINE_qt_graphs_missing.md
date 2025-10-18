# 🛑 STOP-THE-LINE: Qt Graphs Not Installed

**Date:** 2025-10-17  
**Severity:** 🔴 Blocking – Foundation Issue  
**Owner:** Mark Nicholson  
**Status:** Active Halt  

---

## Issue

Qt Graphs — the required, non-deprecated plotting module — is missing from the Qt 6.9.3 installation on dev-01.  
Qt Charts (deprecated) is present but cannot be used under the current "no-deprecated-libraries" policy.

---

## Root Cause

During initial installation, the Qt Graphs add-on was not selected or installed.  
All development that depends on the renderer layer (Phase 1–2 plotting, Milestone A validation) therefore rests on an incomplete foundation.

---

## Decision

➡️ **STOP all development immediately.**  
No commits, merges, or tests until the foundation is corrected.

We will:
1. Re-install Qt 6.9.3 including the Qt Graphs add-on (headless or via virtual display).
2. Verify presence of Qt6GraphsConfig.cmake and headers under /opt/Qt/6.9.3/gcc_64/.
3. Confirm that CMake find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets Graphs) succeeds.

Only after successful verification will development resume.

---

## Rationale

- Testing with deprecated Qt Charts validates nothing for production.
- Preserves integrity of the "Right-First-Time / No-Deprecated-Libraries" policy.
- Ensures Milestone A validation reflects the actual production stack.
- Prevents downstream rework and technical debt.

---

## Resume Criteria

✅ Qt6GraphsConfig.cmake present and readable  
✅ Qt Graphs headers visible under include/QtGraphs/  
✅ find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets Graphs) succeeds  
✅ CI and CMake show no references to Qt Charts  

When all four conditions are met, mark this file as RESOLVED and notify Mark to issue the "Resume" prompt.

---

**No code changes, no merges, and no tests until the halt is cleared.**

---

## Update: 2025-10-17 - Technical Blockers Identified

**Status:** ACTIVE HALT - Technical installation blockers identified

**Issues Encountered:**
1. **Qt Installer Framework Limitations:** The Qt installer (IFW 4.10.0) does not support:
   - `--list-packages` option (returns "Unknown option")
   - `--addRepository` option (returns "Unknown option")
   - `QT_QPA_PLATFORM=offscreen` (offscreen platform plugin not available)

2. **Available Platform Plugins:** Only `minimal` and `xcb` are available, not `offscreen`

3. **Authentication Issues:** Previous attempts failed due to Qt Account authentication problems

**Technical Analysis:**
- Qt installer binary downloaded successfully (69.9MB ELF executable)
- Installer requires display and doesn't support headless command-line options needed
- Repository URLs are not accessible for direct package listing

**Next Steps Required:**
- Alternative Qt installation method needed
- System package manager investigation for Qt Graphs
- Manual Qt Graphs installation approach
- Or different Qt version/installation strategy

**Resume Criteria (unchanged):**
✅ Qt6GraphsConfig.cmake present and readable  
✅ Qt Graphs headers visible under include/QtGraphs/  
✅ find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets Graphs) succeeds  
✅ CI and CMake show no references to Qt Charts  

**Work Status:** Halted for the day - technical blockers require alternative approach

---

## Update: 2025-10-18 - aqtinstall Attempt Failed

**Status:** ACTIVE HALT - aqtinstall approach also blocked

**Issues Encountered with aqtinstall:**
1. **Module Availability:** Qt Graphs module not available for Qt 6.9.3 via aqtinstall
2. **Authentication Required:** aqtinstall requires Qt Account credentials for official installer
3. **Package Resolution:** aqtinstall cannot find required packages (qt_base, qtgraphs)

**Technical Analysis:**
- aqtinstall v3.3.0 installed successfully
- Qt 6.9.3 is available in the repository
- No modules available for Qt 6.9.3 gcc_64 architecture
- Official installer requires Qt Account authentication

**Alternative Approaches Needed:**
- Manual Qt Graphs installation from source
- System package manager investigation
- Different Qt installation method
- Or proceed with Qt Charts temporarily (policy violation)

**Resume Criteria (unchanged):**
✅ Qt6GraphsConfig.cmake present and readable  
✅ Qt Graphs headers visible under include/QtGraphs/  
✅ find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets Graphs) succeeds  
✅ CI and CMake show no references to Qt Charts  

**Work Status:** Halted for the day - multiple installation approaches blocked
