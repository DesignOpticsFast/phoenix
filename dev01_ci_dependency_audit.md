# dev-01 CI Dependency Audit

**Date**: 2025-10-30  
**Purpose**: Determine if dev-01 downtime is contributing to PR failures (specifically PR #56)

## Executive Summary

**CRITICAL FINDING**: The CI workflow has a **BLOCKING dependency** on dev-01 for PRs with code changes.

The required PR check "gate" depends on the "build" job, which runs exclusively on `[self-hosted, Linux, X64, dev-01, qt6]`. When dev-01 is offline, **all PRs with code changes will fail**, even if they're docs-only or submodule-only PRs that should pass independently.

## 1. Self-Hosted Runner Usage

### Workflows with dev-01 Dependencies

| Workflow | Job | Runner | Trigger | Status |
|----------|-----|--------|---------|--------|
| `ci.yml` | `build` | `[self-hosted, Linux, X64, dev-01, qt6]` | PRs + push | **BLOCKING** |
| `codeql.yml` | `analyze` | `[self-hosted, linux, x64, dev01, fastpath]` | Push to main + weekly | Non-blocking |
| `dev01-heartbeat.yml` | `ping` | `[self-hosted, linux, x64, dev-01, qt6]` | Schedule (daily) + manual | Non-blocking |

### File References

```
.github/workflows/ci.yml:156:    runs-on: [self-hosted, Linux, X64, dev-01, qt6]
.github/workflows/codeql.yml:17:    runs-on: [self-hosted, linux, x64, dev01, fastpath]
.github/workflows/dev01-heartbeat.yml:9:    runs-on: [self-hosted, linux, x64, dev-01, qt6]
```

## 2. Required PR Checks

Branch protection uses GitHub Rulesets (id: 8589451) with:
- **Required status check**: `gate`
- `gate` job depends on: `[changes, docs, build, ci_checks]`

This creates a **critical path dependency**: PRs require `gate` → which requires `build` → which requires dev-01.

### Conditional Execution

The `build` job has this condition:
```yaml
if: >
  (github.event_name == 'push') ||
  (github.event_name == 'pull_request' &&
   github.event.pull_request.head.repo.full_name == github.repository &&
   needs.changes.outputs.code_changed == 'true')
```

**Analysis**: The build job runs on PRs only when:
1. The PR comes from the same repository (not forks)
2. Code changes are detected

However, there's a **PROBLEM**: The job will **fail or hang** when dev-01 is offline, which causes the `gate` job to fail, **blocking all PRs**.

## 3. Network & Secret Dependencies

### No Hard Network Coupling Found
- No explicit SSH/SCP connections
- No internal IP references
- No `/srv/` path dependencies

### Secrets Used
- `GITHUB_TOKEN` - Standard GitHub Actions token (not dev-01 specific)
- `AWS_HEARTBEAT_ROLE` - Only in `aws-heartbeat.yml` (not PR-blocking)

## 4. Impact Assessment

### When dev-01 is DOWN:

| Scenario | Build Job | Gate Job | PR Result |
|----------|-----------|----------|-----------|
| PR with code changes | ❌ **Fails/Hangs** | ❌ **Fails** | ❌ **BLOCKED** |
| PR with docs-only changes | ⏭️ Skipped (correctly) | ✅ Passes | ✅ Allowed |
| PR with submodule-only changes | ⏭️ Skipped (but incorrectly triggers when submodule detection fails) | ❓ **Unknown** | ❓ **POTENTIALLY BLOCKED** |

### When dev-01 is ONLINE:

All PRs pass normally, creating an **intermittent failure pattern** that's difficult to diagnose.

## 5. Root Cause Analysis

**PR #56 (submodule bump) is failing** despite having no code changes because:

1. The change detection logic may not correctly identify submodules as "non-code"
2. OR the `build` job is incorrectly triggered and fails when dev-01 is down
3. OR the `gate` job fails for another reason related to dev-01 dependency

**Critical Issue**: The `gate` job has `if: always()` which means it runs even if `build` is skipped, but it fails if `build` fails.

Looking at gate logic:
```bash
if [[ "${{ needs.changes.outputs.code_changed }}" == "true" ]]; then
  [[ "${{ needs.build.result }}" == "success" ]] || { echo "Build failed"; exit 1; }
fi
```

This is **correct** - it only checks `build` when code_changed is true. However, if change detection is wrong or if there's a race condition, PRs can be blocked.

## 6. Risk Assessment Matrix

| Dependency | Risk Level | Impact | When Fails |
|------------|------------|--------|------------|
| `build` job on PRs with code changes | **CRITICAL** | Blocks PR merge | Always when dev-01 down |
| CodeQL workflow | LOW | Optional security scan | Never blocks PRs |
| dev-01 heartbeat | LOW | Monitoring only | Never blocks PRs |

## 7. Recommended Changes

### Option A: Make build job non-required for PRs (Quick Fix)

**Change the `gate` job logic to not fail on build failures for PRs:**

```yaml
# In .github/workflows/ci.yml, modify gate job
if [[ "${{ needs.changes.outputs.code_changed }}" == "true" ]]; then
  # Only require build for pushes to main, not PRs
  if [[ "${{ github.event_name }}" == "push" ]]; then
    [[ "${{ needs.build.result }}" == "success" ]] || { echo "Build failed"; exit 1; }
  else
    # For PRs, treat build as optional
    if [[ "${{ needs.build.result }}" != "success" && "${{ needs.build.result }}" != "skipped" ]]; then
      echo "⚠️ Build not available (dev-01 may be offline); allowing PR to proceed"
    fi
  fi
fi
```

### Option B: Add PR Guard to Skip Build for Non-code PRs

**Strengthen the change detection to ensure build never runs for docs/submodule PRs:**

Already implemented via:
```yaml
if: >
  (github.event_name == 'push') ||
  (github.event_name == 'pull_request' &&
   github.event.pull_request.head.repo.full_name == github.repository &&
   needs.changes.outputs.code_changed == 'true')
```

But should also add:
```yaml
continue-on-error: true  # In build job
```

### Option C: Migrate Build Job to GitHub-Hosted (Long-term Solution)

**Replace Qt6 build with a GitHub-hosted alternative:**

1. Use Docker image with Qt6 pre-installed
2. Or use a GitHub-hosted macOS runner (which has Qt6 available)
3. Or provide Qt6 installation steps for ubuntu-latest

**Problem**: This may be difficult due to Qt licensing requirements or complex build dependencies.

## 8. Immediate Action Items

1. ✅ **AUDIT COMPLETE** - Documented the issue
2. **TODO**: Implement Option A or B to unblock PRs immediately
3. **TODO**: Investigate Option C for long-term solution
4. **TODO**: Add monitoring/alerting for dev-01 availability
5. **TODO**: Consider making build job truly optional with `continue-on-error: true`

## Appendix: Full Workflow Details

See individual workflow files:
- `.github/workflows/ci.yml` (lines 148-227)
- `.github/workflows/codeql.yml`
- `.github/workflows/dev01-heartbeat.yml`
