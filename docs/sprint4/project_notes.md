# Phoenix Sprint 4 - Project Notes

## Post-Incident Note — Qt Charts (Deprecated) Usage

**Date:** October 17, 2025  
**Issue:** We used Qt Charts (Deprecated) during Phase 1–2 because we did not identify its deprecated status before starting.  
**Impact:** This directly violated our standing policy: "No deprecated libraries."  

### Root Cause Analysis
- **Primary Cause:** Failure to verify library deprecation status before implementation
- **Secondary Cause:** Lack of automated checks for deprecated dependencies in CI/CD pipeline
- **Contributing Factor:** No pre-execution validation for deprecated module usage

### Corrective Actions
1. **Immediate:** Migration to Qt Graphs (non-deprecated alternative)
2. **Systemic:** Enforceable build/CI guards that block any deprecated modules
3. **Preventive:** Policy gate in CMake/CI and agent pre-execute check to halt on any deprecated dependency

### Policy Enforcement
- **CMake Policy:** `cmake/policy_no_deprecated.cmake` with fatal error on Qt6Charts detection
- **CI Pipeline:** `.github/workflows/policy.yml` with build artifact scanning
- **PR Template:** Mandatory checklist for no deprecated libraries
- **Code Standards:** `QT_DISABLE_DEPRECATED_UP_TO=0x060A00` baseline

### Lessons Learned
1. **Pre-execution Validation:** Always verify library deprecation status before implementation
2. **Automated Checks:** CI/CD must include deprecated dependency scanning
3. **Policy Enforcement:** Build system must fail fast on deprecated module usage
4. **Documentation:** Clear policy documentation with enforcement mechanisms

### Status
- **Phase 1-2:** Completed with Qt Charts (deprecated) - documented deviation
- **Migration:** In progress to Qt Graphs (non-deprecated)
- **Policy:** Enforced project-wide with automated checks
- **Prevention:** Pre-execution validation implemented

---

## Known Deviations

### Qt Charts Usage (Phase 1-2)
- **Deviation:** Used Qt Charts (deprecated) for plotting functionality
- **Rationale:** Failed to identify deprecation status before implementation
- **Mitigation:** Migration to Qt Graphs in progress
- **Policy Impact:** Violated "no deprecated libraries" policy
- **Reference:** See Post-Incident Note above

---

## Technical Debt

### Deprecated Dependencies
- **Qt Charts:** Will be replaced with Qt Graphs in next phase
- **Impact:** Low (plotting functionality only)
- **Timeline:** Migration planned for Phase 3-5 preparation

### Code Quality
- **Standards:** C++17 (Phoenix), C++20 (Bedrock) compliance maintained
- **Testing:** All acceptance criteria met despite deprecated dependency
- **Performance:** No performance impact from deprecated library usage

---

## Policy Updates

### No Deprecated Libraries Policy
- **Scope:** All Phoenix and Bedrock repositories
- **Enforcement:** CMake policy + CI scanning + PR template
- **Baseline:** Qt 6.10.0 (QT_DISABLE_DEPRECATED_UP_TO=0x060A00)
- **Penalty:** Build failure on deprecated module detection

### Pre-execution Validation
- **Agent Check:** Verify library deprecation status before implementation
- **CMake Check:** Fatal error on deprecated module detection
- **CI Check:** Build artifact scanning for deprecated dependencies
- **PR Check:** Mandatory checklist for policy compliance
