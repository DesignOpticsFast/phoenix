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
