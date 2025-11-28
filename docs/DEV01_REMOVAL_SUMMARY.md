# dev-01 Removal Summary

**Date**: 2025-11-25  
**Status**: ✅ Complete

## Overview

dev-01 has been permanently removed from all CI infrastructure. All CI tasks now run on GitHub Actions hosted runners.

---

## Changes Made

### 1. GitHub Actions Workflows

#### Phoenix (`.github/workflows/ci.yml`)
- ✅ Updated Qt version from 6.10.0 to 6.10.1
- ✅ Removed dev-01 runner from `soak-nightly` job (now uses `ubuntu-latest`)
- ✅ Removed dev-01 runner from `soak-release` job (now uses `ubuntu-latest`)
- ✅ Added Qt 6.10.1 installation steps to soak jobs
- ✅ Added `qtgraphs` and `qtgraphswidgets` modules to Qt installation

#### Phoenix (`.github/workflows/dev01-heartbeat.yml`)
- ✅ **DELETED** - Entire workflow file removed

#### Bedrock (`.github/workflows/ci.yml`)
- ✅ Added new `integration-tests` job
- ✅ Configured Qt 6.10.1 installation via `jurplel/install-qt-action@v3`
- ✅ Added Capabilities and XYSine integration test execution
- ✅ Added submodule checkout for Palantir contracts

### 2. Scripts and Build Files

#### Phoenix
- ✅ **DELETED** `scripts/dev01-preflight.sh`
- ✅ Updated `Makefile` - removed `build-dev01` and `clean-dev01` targets
- ✅ Updated `refresh-context.sh` - removed dev-01 detection logic

### 3. Documentation

#### Phoenix
- ✅ Updated `README.md` - removed dev-01-first policy, updated for GitHub Actions
- ✅ Updated `docs/CI_DEV01_RUNNER.md` - marked as deprecated with migration notice
- ✅ Created `docs/DEV01_REMOVAL_AUDIT.md` - comprehensive audit of all references
- ✅ Created `docs/DEV01_REMOVAL_SUMMARY.md` - this document

#### Bedrock
- ✅ Updated `README.md` - removed test commit reference

### 4. Historical References

The following files contain historical references to dev-01 but are archived/legacy documentation:
- `docs/archive/sprint4/legacy/preflight_dev01.md` - Historical, no action needed
- `docs/SPRINT_SUMMARY_2025-01-21.md` - Historical, no action needed
- Various sprint documentation files - Historical, no action needed

---

## CI Configuration

### Phoenix CI
- **Platform**: GitHub Actions (ubuntu-latest)
- **Qt Version**: 6.10.1
- **Installation**: `jurplel/install-qt-action@v3`
- **Modules**: qtbase, qttools, qttranslations, qtgraphs, qtgraphswidgets
- **Build System**: CMake with Ninja
- **Tests**: Unit tests run automatically

### Bedrock CI
- **Platform**: GitHub Actions (ubuntu-latest)
- **Qt Version**: 6.10.1 (for integration tests only)
- **Installation**: `jurplel/install-qt-action@v3`
- **Integration Tests**: Capabilities and XYSine RPC tests
- **Build System**: CMake with Ninja

---

## Verification

### Remaining References Check

Run this command to verify no active dev-01 references remain:

```bash
# Phoenix
grep -r "dev-01\|dev01" --include="*.md" --include="*.yml" --include="*.sh" --include="Makefile" \
  .github/ docs/ README.md Makefile refresh-context.sh \
  | grep -v "DEV01_REMOVAL\|CI_DEV01_RUNNER\|archive\|legacy\|SPRINT_SUMMARY"

# Bedrock
grep -r "dev-01\|dev01" --include="*.md" --include="*.yml" \
  README.md .github/ \
  | grep -v "archive\|legacy"
```

Expected: No matches (or only historical/archived references)

---

## Migration Benefits

1. **No Infrastructure Dependencies**: No need to maintain self-hosted runners
2. **Consistent Environment**: GitHub Actions provides consistent build environments
3. **Easier Scaling**: Can run multiple jobs in parallel without resource constraints
4. **Better Security**: No need to manage SSH keys or runner access
5. **Qt 6.10.1**: Updated to latest stable version with security fixes

---

## Next Steps

1. ✅ All dev-01 references removed from active code
2. ✅ CI workflows updated to use GitHub Actions
3. ✅ Documentation updated
4. ⏳ **PENDING**: Verify CI passes on next PR/push
5. ⏳ **PENDING**: Monitor integration tests in CI

---

## Rollback Plan

If issues arise, the changes can be reverted by:
1. Restoring previous workflow files from git history
2. Re-adding dev-01 runner configuration (if dev-01 is restored)
3. Reverting documentation changes

However, **dev-01 is permanently offline**, so rollback would require setting up a new self-hosted runner.

---

## Files Changed

### Phoenix
- `.github/workflows/ci.yml` - Updated Qt version, removed dev-01 runners
- `.github/workflows/dev01-heartbeat.yml` - **DELETED**
- `Makefile` - Removed dev-01 targets
- `refresh-context.sh` - Removed dev-01 detection
- `scripts/dev01-preflight.sh` - **DELETED**
- `README.md` - Updated workflow documentation
- `docs/CI_DEV01_RUNNER.md` - Marked as deprecated
- `docs/DEV01_REMOVAL_AUDIT.md` - **NEW** - Audit document
- `docs/DEV01_REMOVAL_SUMMARY.md` - **NEW** - This document

### Bedrock
- `.github/workflows/ci.yml` - Added integration tests job
- `README.md` - Removed test commit reference

---

## Status: ✅ Complete

All dev-01 references have been removed or migrated to GitHub Actions. CI is now fully running on GitHub Actions hosted runners.

