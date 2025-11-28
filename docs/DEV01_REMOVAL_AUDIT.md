# dev-01 Removal Audit

This document tracks all dev-01 references found and their removal status.

## Summary

**Total References Found:** 207+ across Phoenix, Bedrock, and Palantir repos

**Status:** All references will be removed or migrated to GitHub Actions

---

## Phoenix Repository

### GitHub Actions Workflows

| File | Line | Reference | Action |
|------|------|-----------|--------|
| `.github/workflows/ci.yml` | 360 | `runs-on: [self-hosted, Linux, X64, dev-01, qt6]` | Remove soak-nightly job or migrate to GitHub Actions |
| `.github/workflows/ci.yml` | 400 | `runs-on: [self-hosted, Linux, X64, dev-01, qt6]` | Remove soak-release job or migrate to GitHub Actions |
| `.github/workflows/dev01-heartbeat.yml` | Entire file | dev-01 heartbeat workflow | Delete entire file |

### Documentation Files

| File | Status | Action |
|------|--------|--------|
| `docs/CI_DEV01_RUNNER.md` | Entire file | Mark as deprecated or delete |
| `README.md` | Multiple lines | Remove dev-01-first policy, update build instructions |
| `docs/DEVELOPMENT_WORKFLOW.md` | Multiple lines | Remove dev-01 references, update for GitHub Actions |
| `docs/SETUP_MAC_DEV_ENV.md` | Multiple lines | Remove dev-01 references |
| `docs/SPRINT_SUMMARY_2025-01-21.md` | Multiple lines | Historical - mark as archived |
| Various sprint docs | Multiple | Historical references - mark as archived |

### Scripts

| File | Status | Action |
|------|--------|--------|
| `scripts/dev01-preflight.sh` | Entire file | Delete or rewrite for GitHub Actions |
| `refresh-context.sh` | Lines 18-22 | Remove dev-01 detection logic |
| `Makefile` | Lines 1-9 | Remove build-dev01 target |

### Path References

| Pattern | Files | Action |
|---------|-------|--------|
| `/opt/Qt/6.10.0/gcc_64` | Multiple | Remove or replace with GitHub Actions Qt installation |
| `/opt/Qt/6.9.3/gcc_64` | Multiple | Remove (outdated) |

---

## Bedrock Repository

### Documentation Files

| File | Status | Action |
|------|--------|--------|
| `docs/sprint4/PHASE_0_5_ARTIFACTS.md` | Line 48 | Historical - mark as archived |
| `docs/sprint4/GATE_0_5_DECISION.md` | Line 16 | Historical - mark as archived |
| `docs/sprint4/CONTROL_REV_5.3.md` | Line 23 | Historical - mark as archived |
| `README.md` | Line 97 | Remove test commit reference |

---

## Palantir Repository

No dev-01 references found.

---

## Migration Plan

### 1. Phoenix CI Updates

- **Current:** Uses dev-01 for soak tests
- **New:** Use GitHub Actions ubuntu-latest with Qt 6.10.1 via aqtinstall
- **Integration Tests:** Add Bedrock integration tests to GitHub Actions

### 2. Build Matrix

- **Linux:** Required (ubuntu-latest)
- **macOS:** Preferred (macos-latest)
- **Windows:** Best-effort (windows-latest, non-blocking)

### 3. Qt Installation

- Use `aqtinstall` or `jurplel/install-qt-action@v3` with version 6.10.1
- Remove all `/opt/Qt/` path references

### 4. Integration Tests

- Move Bedrock integration tests to GitHub Actions
- Run on Linux (required) and macOS (preferred)

---

## Implementation Status

- [ ] Phoenix CI workflows updated
- [ ] Bedrock CI workflows updated
- [ ] Documentation updated
- [ ] Scripts cleaned up
- [ ] All references removed

