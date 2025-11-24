# Phoenix Sprint 4.4 Control Document Hygiene Audit

**Date**: 2025-11-24  
**Auditor**: UnderLord  
**Purpose**: Ensure all governance and control documents are present, complete, consistent, and aligned with Sprint 4.4 goals before WP0 begins.

---

## Executive Summary

**Overall Status**: ⚠️ **MOSTLY HEALTHY** with **3 HIGH-PRIORITY ISSUES** requiring fixes before WP0.

**Key Findings**:
- ✅ All mandatory policy documents exist and are complete
- ⚠️ Some documents contain outdated Sprint 4.2/4.3 references
- ❌ Missing architecture documentation directory structure
- ⚠️ ADR 0001 has placeholder date (`2025-01-XX`)
- ⚠️ Qt path in `UNDERLORD_MAC_ENVIRONMENT.md` may be outdated (references `/Users/mark/Qt/6.10.0` but actual is `/Users/underlord/Qt/6.10.1`)

---

## 1. Document-by-Document Audit Table

| File | Exists? | Non-empty? | Contains Required Policy? | Has Outdated Content? | Script References Correct? | Notes / Issues |
|------|---------|------------|---------------------------|----------------------|---------------------------|---------------|
| `docs/UNDERLORD_MAC_ACCESS.md` | ✅ Yes | ✅ Yes (33 lines) | ✅ Yes | ❌ No | ✅ Yes | Complete and current. Script references correct. |
| `docs/SPRINT_HYGIENE.md` | ✅ Yes | ✅ Yes (42 lines) | ✅ Yes | ⚠️ Minor | ✅ Yes | References Sprint 4.2/4.3 in examples (acceptable). Script references correct. |
| `docs/UNDERLORD_MAC_ENVIRONMENT.md` | ✅ Yes | ✅ Yes (56 lines) | ✅ Yes | ⚠️ **ISSUE** | ✅ Yes | **Qt path may be outdated**: References `/Users/mark/Qt/6.10.0/macos` but actual installation is `/Users/underlord/Qt/6.10.1/macos`. Script references correct. |
| `docs/architecture/overview.md` | ❌ No | N/A | N/A | N/A | ❌ N/A | **MISSING**: Directory `docs/architecture/` does not exist. |
| `docs/architecture/analysis_loop.md` | ❌ No | N/A | N/A | N/A | ❌ N/A | **MISSING**: Directory `docs/architecture/` does not exist. |
| `docs/architecture/transport_overview.md` | ❌ No | N/A | N/A | N/A | ❌ N/A | **MISSING**: Directory `docs/architecture/` does not exist. |
| `docs/architecture/analysis_run_modes.md` | ❌ No | N/A | N/A | N/A | ❌ N/A | **MISSING**: Directory `docs/architecture/` does not exist. |
| `docs/architecture/licensing.md` | ❌ No | N/A | N/A | N/A | ❌ N/A | **MISSING**: Directory `docs/architecture/` does not exist. |
| `docs/adr/ADR-S4.2-Plotting-01.md` | ❌ No | N/A | N/A | N/A | ❌ N/A | **MISSING**: File does not exist. Only `0001-feature-boundary-palantir.md` exists in `docs/adr/`. |
| `docs/build/phoenix_build.md` | ❌ No | N/A | N/A | N/A | ❌ N/A | **MISSING**: Directory `docs/build/` does not exist. |
| `docs/build/bedrock_build.md` | ❌ No | N/A | N/A | N/A | ❌ N/A | **MISSING**: Directory `docs/build/` does not exist. |
| `PHOENIX_PHASE2E_CHECKPOINT.md` | ✅ Yes | ✅ Yes (45 lines) | ✅ Yes | ❌ No | ❌ N/A | Complete and current. Not referenced by scripts (checkpoint document). |
| `docs/KNOWN_TEST_FAILURES.md` | ✅ Yes | ✅ Yes | ✅ Yes | ⚠️ Minor | ✅ Yes | References "Sprint 4.3" in header. Script auto-creates if missing. |
| `docs/SPRINT_TASKS.md` | ✅ Yes | ✅ Yes | ✅ Yes | ⚠️ Minor | ✅ Yes | References "Sprint 4.2" in header (current branch). Script auto-creates if missing. |

---

## 2. Cross-Document Health Summary

### ✅ Aligned Documents

- **Policy Documents**: All three mandatory policy documents (`UNDERLORD_MAC_ACCESS.md`, `SPRINT_HYGIENE.md`, `UNDERLORD_MAC_ENVIRONMENT.md`) are present, complete, and internally consistent.
- **Zero-Autonomy Policy**: Consistently referenced across all policy documents.
- **Sprint Branch Pattern**: `SPRINT_HYGIENE.md` correctly documents both `sprint/X.Y` and `sprintX.Y-*` patterns, matching `daily_startup.sh` implementation.

### ⚠️ Outdated Content References

**Sprint Numbering**:
- `docs/KNOWN_TEST_FAILURES.md`: Header says "Sprint 4.3" (should be updated to current sprint or made generic)
- `docs/SPRINT_TASKS.md`: Header says "Sprint 4.2" (acceptable if current branch is 4.2, but should be dynamic)
- `docs/TECHNICAL_DEBT.md`: References "Sprint 4.4" (correct for future work)

**QtCharts References**:
- ✅ **No active references found** in current docs
- ⚠️ Legacy references exist only in `docs/archive/sprint4/legacy/` (acceptable, archived)

**dev-01 References**:
- `docs/CI_DEV01_RUNNER.md`: Contains dev-01 workflow documentation (acceptable if still in use)
- `Makefile`: References `build-dev01` and `scripts/dev01-preflight.sh` (acceptable if still in use)

**QCustomPlot References**:
- ✅ **None found** in active documentation

### ❌ Missing Architecture Documentation

The following architecture documents are **missing** and may be needed for Sprint 4.4:
- `docs/architecture/overview.md`
- `docs/architecture/analysis_loop.md`
- `docs/architecture/transport_overview.md`
- `docs/architecture/analysis_run_modes.md`
- `docs/architecture/licensing.md`

**Impact**: These may be needed to document Sprint 4.4 architecture decisions, but they are not currently referenced by scripts. **Decision needed**: Are these required for WP0, or can they be created during Sprint 4.4?

### ⚠️ ADR Issues

**ADR 0001 (`docs/adr/0001-feature-boundary-palantir.md`)**:
- ✅ Content is current and relevant for Sprint 4.4
- ⚠️ **Placeholder date**: Line 5 says `Updated: 2025-01-XX` (should be actual date)
- ✅ References Palantir IPC architecture (aligned with Sprint 4.4 goals)

**Missing ADR**:
- `docs/adr/ADR-S4.2-Plotting-01.md` does not exist (may not be required if plotting decisions are documented elsewhere)

---

## 3. Script Consistency Review

### `daily_startup.sh` References

| Document Referenced | Path in Script | File Exists? | Required Content Present? | Status |
|---------------------|----------------|--------------|---------------------------|--------|
| `UNDERLORD_MAC_ACCESS.md` | `$root/docs/UNDERLORD_MAC_ACCESS.md` | ✅ Yes | ✅ Yes | ✅ **PASS** |
| `SPRINT_HYGIENE.md` | `$root/docs/SPRINT_HYGIENE.md` | ✅ Yes | ✅ Yes | ✅ **PASS** |
| `UNDERLORD_MAC_ENVIRONMENT.md` | `$root/docs/UNDERLORD_MAC_ENVIRONMENT.md` | ✅ Yes | ✅ Yes | ✅ **PASS** |
| `KNOWN_TEST_FAILURES.md` | `$root/docs/KNOWN_TEST_FAILURES.md` | ✅ Yes | ✅ Yes | ✅ **PASS** (auto-created if missing) |
| `SPRINT_TASKS.md` | `$root/docs/SPRINT_TASKS.md` | ✅ Yes | ✅ Yes | ✅ **PASS** (auto-created if missing) |

**All script references are correct and files exist.**

### `Makefile` References

- `daily` target: References `.underlord/tasks/daily_startup.sh` ✅ **PASS**
- `build-dev01` target: References `scripts/dev01-preflight.sh` (existence not verified, but not critical for Sprint 4.4)

### Missing Scripts

- `scripts/validate_qml.sh`: **NOT FOUND** (not referenced by `daily_startup.sh`, may not be required)

---

## 4. High-Priority Issues (Must Fix Before WP0)

### 🔴 **ISSUE #1: Qt Path Mismatch in UNDERLORD_MAC_ENVIRONMENT.md**

**File**: `docs/UNDERLORD_MAC_ENVIRONMENT.md`  
**Problem**: Document references `/Users/mark/Qt/6.10.0/macos` but actual Qt installation is at `/Users/underlord/Qt/6.10.1/macos` (as confirmed by build system and `20-qt6.sh`).

**Impact**: 
- Low (script uses actual path detection), but creates confusion
- Document should reflect actual environment

**Fix Required**: Update Qt path references to match actual installation location, or document both possible locations.

---

### 🟡 **ISSUE #2: ADR 0001 Placeholder Date**

**File**: `docs/adr/0001-feature-boundary-palantir.md`  
**Problem**: Line 5 contains `Updated: 2025-01-XX` (placeholder date).

**Impact**: Low (cosmetic), but unprofessional.

**Fix Required**: Replace `2025-01-XX` with actual update date (or remove if never updated).

---

### 🟡 **ISSUE #3: KNOWN_TEST_FAILURES.md Sprint Reference**

**File**: `docs/KNOWN_TEST_FAILURES.md`  
**Problem**: Header says "Sprint 4.3" but we're on Sprint 4.2 branch (or moving to 4.4).

**Impact**: Low (informational only), but should be current.

**Fix Required**: Update to current sprint number or make generic (e.g., "Current Sprint").

---

## 5. Optional Improvements List

### Clarity Fixes

1. **SPRINT_TASKS.md**: Consider making sprint number dynamic or generic to avoid constant updates.
2. **UNDERLORD_MAC_ENVIRONMENT.md**: Add note about Qt path detection fallback logic (script checks multiple locations).
3. **PHOENIX_PHASE2E_CHECKPOINT.md**: Add cross-reference to Sprint 4.4 planning document (if it exists).

### Cross-Linking Suggestions

1. **ADR 0001**: Add cross-reference to Sprint 4.4 transport integration plan (when available).
2. **TECHNICAL_DEBT.md**: Add cross-reference to Sprint 4.4 WP0/WP1 planning.
3. **Policy Documents**: Add cross-reference to `AI_OPERATIONS_GUIDE.md` for broader context.

### Missing ADR Candidates

Consider creating ADRs for:
- **Sprint 4.4 WP0**: Build flag isolation strategy (`PHX_WITH_TRANSPORT_DEPS`)
- **Sprint 4.4 WP1**: Executor/Strategy pattern for analysis run modes
- **Sprint 4.4**: Transport integration approach (if not already covered by ADR 0001)

### Architecture Documentation

**Decision Needed**: Are the following architecture documents required for Sprint 4.4 WP0?
- `docs/architecture/overview.md`
- `docs/architecture/analysis_loop.md`
- `docs/architecture/transport_overview.md`
- `docs/architecture/analysis_run_modes.md`
- `docs/architecture/licensing.md`

If required, these should be created before WP0 begins. If not required, they can be created incrementally during Sprint 4.4.

---

## 6. Sprint 4.4 Alignment Check

### ✅ Aligned with Sprint 4.4 Goals

- **Zero-Autonomy Policy**: ✅ Consistently enforced across all documents
- **Local-Only Phoenix Baseline**: ✅ `PHOENIX_PHASE2E_CHECKPOINT.md` documents this clearly
- **Transport Integration Planning**: ✅ ADR 0001 provides foundation
- **Build Flag Isolation**: ⚠️ Not yet documented (may need ADR)

### ⚠️ Gaps for Sprint 4.4

1. **Build Flag Strategy**: No document describes `PHX_WITH_TRANSPORT_DEPS` flag strategy
2. **Executor Pattern**: No document describes Strategy/Executor pattern for analysis run modes
3. **WP0/WP1 Planning**: No Sprint 4.4 planning document found (may exist outside `docs/`)

---

## 7. Recommendations

### Before WP0 Begins

1. ✅ **Fix Issue #1**: Update Qt path in `UNDERLORD_MAC_ENVIRONMENT.md` to reflect actual installation
2. ✅ **Fix Issue #2**: Replace placeholder date in ADR 0001
3. ✅ **Fix Issue #3**: Update Sprint reference in `KNOWN_TEST_FAILURES.md`
4. ⚠️ **Decision**: Determine if architecture docs are required for WP0 (if yes, create them)

### During Sprint 4.4

1. Create Sprint 4.4 planning document (if not exists)
2. Create ADR for build flag isolation strategy
3. Create ADR for executor/strategy pattern
4. Create architecture documentation as needed
5. Update cross-references between documents

---

## 8. Audit Conclusion

**Overall Assessment**: ✅ **READY FOR WP0** with minor fixes.

**Critical Path**: Fix the 3 high-priority issues listed above before beginning WP0.

**Documentation Health**: Strong foundation with all mandatory policy documents in place. Architecture documentation can be created incrementally during Sprint 4.4 if not required for WP0.

---

**End of Audit Report**

