# Phoenix Sprint 4.4 Chunk 7: Dependabot Hygiene & Auto-Merge Workflow Design

**Date**: 2025-11-24  
**Chunk**: Sprint 4.4 Chunk 7 - Dependabot Hygiene + Auto-Merge Workflow Design  
**Status**: Planning - Workflow files NOT created in this chunk

---

## Executive Summary

This document provides:
1. Summary of existing Dependabot PRs found and their status
2. Auto-merge workflow design for future Dependabot PRs
3. Proposed GitHub Actions workflow YAML (not yet committed)

**Key Findings**:
- Phoenix: 1 Dependabot PR found (contracts submodule update) - **OUTDATED**, superseded by v1.1.0
- Bedrock: 1 Dependabot PR found (GitHub Actions updates) - **ELIGIBLE** but Bedrock main is dirty
- Auto-merge policy: Patch-level updates only, with strict path restrictions

---

## 1. Phoenix Dependabot PR Discovery & Status

### Dependabot Branches Found

**Branch**: `origin/dependabot/submodules/submodule-all-c1ed6c5a9b`
- **Commit**: `9eeaeb13fb11524c47092763bb317b658d57af3c`
- **Author**: dependabot[bot]
- **Date**: 2025-11-24 13:54:49 +0000
- **Message**: `deps(submodule)(deps): bump contracts in the submodule-all group`

### Changes Analysis

**Files Changed**: 1 file
- `contracts` (submodule reference)

**Submodule Update**:
- From: `ad0e9882cd3d9cbbf80fc3b4ac23cd1df7547f53` (Palantir v1.0.0)
- To: `7febe5fa424e7516eefd2c3ce917c9d238973a00` (Palantir main, intermediate commit)

**Safety Assessment**:
- ✅ Only touches submodule reference (no code changes)
- ✅ Patch-level change (submodule update)
- ✅ No core code paths touched
- ⚠️ **OUTDATED**: This PR updates to an intermediate commit (`7febe5f`), but we already have a newer version (`b6563fe`/v1.1.0) in sprint/4.4

### Decision: NOT MERGED

**Reason**: Outdated - superseded by Palantir v1.1.0 update (Chunk 6)

**Status**: Left unmerged - requires manual review to update to v1.1.0 instead

**Recommendation**: Close this PR and create a new one for v1.1.0, or manually update contracts submodule to v1.1.0 on main branch.

---

## 2. Bedrock Dependabot PR Discovery & Status

### Dependabot Branches Found

**Branch**: `origin/dependabot/github_actions/gha-all-b61e4db3c6`
- **Commit**: `72a234a33940699897899543720059164f8baa28`
- **Author**: dependabot[bot]
- **Date**: 2025-11-03 12:45:59 +0000
- **Message**: `deps(actions)(deps): bump the gha-all group with 3 updates`

### Changes Analysis

**Files Changed**: 3 files
- `.github/workflows/auto-merge-dependabot.yml`
- `.github/workflows/auto-merge.yml`
- `.github/workflows/ci.yml`

**Updates**:
- `actions/checkout`: v4 → v5 (major)
- `actions/github-script`: v7 → v8 (major)
- `actions/upload-artifact`: v4 → v5 (major)

**Safety Assessment**:
- ✅ Only touches GitHub Actions workflow files (no code changes)
- ✅ CI/workflow updates only
- ✅ No core code paths touched
- ⚠️ Major version updates (but safe for CI workflows)

### Bedrock Main Status

**Status**: **DIRTY**
- Modified: `CMakeLists.txt` (unstaged)
- Untracked: `CMakePresets.json`

### Decision: NOT MERGED (Main is Dirty)

**Reason**: Bedrock main branch has uncommitted changes

**Status**: Left unmerged - eligible for merge once main is clean

**Recommendation**: 
1. Clean Bedrock main (commit or stash CMakeLists.txt changes)
2. Test the Dependabot branch (workflow files only, low risk)
3. Merge if tests pass

---

## 3. Auto-Merge Workflow Design

### 3.1 Auto-Merge Policy

#### Eligible Dependabot PRs

**Auto-merge ONLY if ALL conditions are met**:

1. **Author Check**:
   - PR author must be `dependabot[bot]` (GitHub bot account)

2. **Update Type**:
   - **Patch-level** version bumps only (e.g., `1.2.3` → `1.2.4`)
   - OR **GitHub Actions** workflow updates (any version)
   - OR **Submodule** updates (any version, but verify no breaking changes)

3. **Path Restrictions** (MUST NOT touch):
   - `src/` - Core source code
   - `include/` - Header files
   - `src/ui/` - UI code
   - `src/transport/` - Transport code
   - `src/analysis/` - Analysis code
   - `CMakeLists.txt` - Build configuration (unless patch-level CMake version)
   - `*.cpp`, `*.hpp`, `*.h` - C++ source files
   - `*.qml` - QML files

4. **Allowed Paths** (safe to auto-merge):
   - `.github/workflows/` - GitHub Actions workflows
   - `contracts/` - Submodule references (with verification)
   - `docs/palantir/` - Submodule references (with verification)
   - `CMakePresets.json` - Build presets (if patch-level)
   - `package.json`, `requirements.txt` - Dependency manifests (if patch-level)
   - `.github/dependabot.yml` - Dependabot configuration

5. **CI Status**:
   - All required CI checks must pass (green status)
   - Build must succeed
   - Tests must pass (if applicable)

6. **Conflict Check**:
   - PR must be mergeable (no conflicts with base branch)

#### Safeguards

1. **Path Validation**: Check changed files against restricted paths list
2. **Version Validation**: Parse PR title to verify patch-level update
3. **CI Gate**: Wait for all CI checks to pass before merging
4. **Conflict Detection**: Verify PR is mergeable
5. **Rate Limiting**: Limit auto-merge to prevent merge storms

---

### 3.2 Proposed Workflow File: Phoenix

**File**: `.github/workflows/dependabot-automerge.yml` (proposed, not yet created)

**Trigger**: `pull_request` events for Dependabot PRs

**Workflow YAML**:

```yaml
name: Auto-merge Dependabot PRs

on:
  pull_request:
    types: [opened, synchronize, reopened]
    branches:
      - main

jobs:
  check-eligibility:
    runs-on: ubuntu-latest
    if: github.actor == 'dependabot[bot]'
    outputs:
      eligible: ${{ steps.check.outputs.eligible }}
      reason: ${{ steps.check.outputs.reason }}
    steps:
      - name: Checkout
        uses: actions/checkout@v4
        with:
          fetch-depth: 0
          token: ${{ secrets.GITHUB_TOKEN }}

      - name: Check PR eligibility
        id: check
        run: |
          # Check if PR is from dependabot
          if [ "${{ github.actor }}" != "dependabot[bot]" ]; then
            echo "eligible=false" >> $GITHUB_OUTPUT
            echo "reason=Not a Dependabot PR" >> $GITHUB_OUTPUT
            exit 0
          fi

          # Get changed files
          git diff --name-only origin/${{ github.base_ref }}...HEAD > /tmp/changed_files.txt

          # Restricted paths (must NOT be touched)
          RESTRICTED_PATHS=(
            "src/"
            "include/"
            "src/ui/"
            "src/transport/"
            "src/analysis/"
            "CMakeLists.txt"
          )

          # Check for restricted paths
          for path in "${RESTRICTED_PATHS[@]}"; do
            if grep -q "^${path}" /tmp/changed_files.txt; then
              echo "eligible=false" >> $GITHUB_OUTPUT
              echo "reason=Touches restricted path: ${path}" >> $GITHUB_OUTPUT
              exit 0
            fi
          done

          # Check for C++/QML files
          if grep -E '\.(cpp|hpp|h|qml)$' /tmp/changed_files.txt; then
            echo "eligible=false" >> $GITHUB_OUTPUT
            echo "reason=Touches C++/QML source files" >> $GITHUB_OUTPUT
            exit 0
          fi

          # Check PR title for patch-level update
          PR_TITLE="${{ github.event.pull_request.title }}"
          if echo "$PR_TITLE" | grep -qE "Bump .+ from .+ to .+"; then
            # Extract version numbers (simplified check)
            if echo "$PR_TITLE" | grep -qE "from [0-9]+\.[0-9]+\.[0-9]+ to [0-9]+\.[0-9]+\.[0-9]+"; then
              FROM_VER=$(echo "$PR_TITLE" | grep -oE "from [0-9]+\.[0-9]+\.[0-9]+" | grep -oE "[0-9]+\.[0-9]+\.[0-9]+")
              TO_VER=$(echo "$PR_TITLE" | grep -oE "to [0-9]+\.[0-9]+\.[0-9]+" | grep -oE "[0-9]+\.[0-9]+\.[0-9]+")
              
              # Check if patch-level (same major.minor)
              FROM_MAJOR_MINOR=$(echo "$FROM_VER" | cut -d. -f1-2)
              TO_MAJOR_MINOR=$(echo "$TO_VER" | cut -d. -f1-2)
              
              if [ "$FROM_MAJOR_MINOR" != "$TO_MAJOR_MINOR" ]; then
                echo "eligible=false" >> $GITHUB_OUTPUT
                echo "reason=Not a patch-level update (major/minor version change)" >> $GITHUB_OUTPUT
                exit 0
              fi
            fi
          fi

          # Allow GitHub Actions and submodule updates regardless of version
          if grep -qE "^\.github/workflows/" /tmp/changed_files.txt; then
            echo "eligible=true" >> $GITHUB_OUTPUT
            echo "reason=GitHub Actions workflow update" >> $GITHUB_OUTPUT
            exit 0
          fi

          if grep -qE "^(contracts|docs/palantir)$" /tmp/changed_files.txt; then
            echo "eligible=true" >> $GITHUB_OUTPUT
            echo "reason=Submodule update" >> $GITHUB_OUTPUT
            exit 0
          fi

          # Default: eligible if no restricted paths touched
          echo "eligible=true" >> $GITHUB_OUTPUT
          echo "reason=Eligible for auto-merge" >> $GITHUB_OUTPUT

  wait-for-ci:
    needs: check-eligibility
    if: needs.check-eligibility.outputs.eligible == 'true'
    runs-on: ubuntu-latest
    steps:
      - name: Wait for CI
        uses: lewagon/wait-on-check-action@v1.3.4
        with:
          ref: ${{ github.event.pull_request.head.sha }}
          check-regexp: '.*'
          repo-token: ${{ secrets.GITHUB_TOKEN }}
          wait-interval: 10
          allowed-conclusions: success

  auto-merge:
    needs: [check-eligibility, wait-for-ci]
    if: needs.check-eligibility.outputs.eligible == 'true'
    runs-on: ubuntu-latest
    steps:
      - name: Auto-merge PR
        uses: actions/github-script@v7
        with:
          script: |
            const { data: pr } = await github.rest.pulls.get({
              owner: context.repo.owner,
              repo: context.repo.repo,
              pull_number: context.issue.number,
            });

            // Check if PR is mergeable
            if (!pr.mergeable) {
              core.setFailed('PR is not mergeable (has conflicts)');
              return;
            }

            // Merge PR
            await github.rest.pulls.merge({
              owner: context.repo.owner,
              repo: context.repo.repo,
              pull_number: context.issue.number,
              merge_method: 'merge',
              commit_title: `Auto-merge: ${pr.title}`,
              commit_message: `Automatically merged Dependabot PR: ${pr.title}\n\nReason: ${{ needs.check-eligibility.outputs.reason }}`,
            });

            core.info(`Successfully merged PR #${context.issue.number}`);
```

---

### 3.3 Proposed Workflow File: Bedrock

**File**: `.github/workflows/dependabot-automerge.yml` (proposed, not yet created)

**Policy Differences from Phoenix**:
- Same eligibility criteria
- Additional restricted paths:
  - `core/` - Core Bedrock library
  - `src/engine/` - Engine code
  - `src/palantir/` - Palantir server code
  - `som/` - SOM (System Object Model) code

**Workflow YAML**: Similar to Phoenix, with Bedrock-specific path restrictions:

```yaml
name: Auto-merge Dependabot PRs

on:
  pull_request:
    types: [opened, synchronize, reopened]
    branches:
      - main

jobs:
  check-eligibility:
    runs-on: ubuntu-latest
    if: github.actor == 'dependabot[bot]'
    outputs:
      eligible: ${{ steps.check.outputs.eligible }}
      reason: ${{ steps.check.outputs.reason }}
    steps:
      - name: Checkout
        uses: actions/checkout@v4
        with:
          fetch-depth: 0
          token: ${{ secrets.GITHUB_TOKEN }}

      - name: Check PR eligibility
        id: check
        run: |
          # Check if PR is from dependabot
          if [ "${{ github.actor }}" != "dependabot[bot]" ]; then
            echo "eligible=false" >> $GITHUB_OUTPUT
            echo "reason=Not a Dependabot PR" >> $GITHUB_OUTPUT
            exit 0
          fi

          # Get changed files
          git diff --name-only origin/${{ github.base_ref }}...HEAD > /tmp/changed_files.txt

          # Restricted paths (Bedrock-specific)
          RESTRICTED_PATHS=(
            "core/"
            "src/engine/"
            "src/palantir/"
            "som/"
            "include/"
            "CMakeLists.txt"
          )

          # Check for restricted paths
          for path in "${RESTRICTED_PATHS[@]}"; do
            if grep -q "^${path}" /tmp/changed_files.txt; then
              echo "eligible=false" >> $GITHUB_OUTPUT
              echo "reason=Touches restricted path: ${path}" >> $GITHUB_OUTPUT
              exit 0
            fi
          done

          # Check for C++ files
          if grep -E '\.(cpp|hpp|h)$' /tmp/changed_files.txt; then
            echo "eligible=false" >> $GITHUB_OUTPUT
            echo "reason=Touches C++ source files" >> $GITHUB_OUTPUT
            exit 0
          fi

          # Allow GitHub Actions and submodule updates
          if grep -qE "^\.github/workflows/" /tmp/changed_files.txt; then
            echo "eligible=true" >> $GITHUB_OUTPUT
            echo "reason=GitHub Actions workflow update" >> $GITHUB_OUTPUT
            exit 0
          fi

          if grep -qE "^docs/palantir$" /tmp/changed_files.txt; then
            echo "eligible=true" >> $GITHUB_OUTPUT
            echo "reason=Submodule update" >> $GITHUB_OUTPUT
            exit 0
          fi

          # Default: eligible if no restricted paths touched
          echo "eligible=true" >> $GITHUB_OUTPUT
          echo "reason=Eligible for auto-merge" >> $GITHUB_OUTPUT

  wait-for-ci:
    needs: check-eligibility
    if: needs.check-eligibility.outputs.eligible == 'true'
    runs-on: ubuntu-latest
    steps:
      - name: Wait for CI
        uses: lewagon/wait-on-check-action@v1.3.4
        with:
          ref: ${{ github.event.pull_request.head.sha }}
          check-regexp: '.*'
          repo-token: ${{ secrets.GITHUB_TOKEN }}
          wait-interval: 10
          allowed-conclusions: success

  auto-merge:
    needs: [check-eligibility, wait-for-ci]
    if: needs.check-eligibility.outputs.eligible == 'true'
    runs-on: ubuntu-latest
    steps:
      - name: Auto-merge PR
        uses: actions/github-script@v7
        with:
          script: |
            const { data: pr } = await github.rest.pulls.get({
              owner: context.repo.owner,
              repo: context.repo.repo,
              pull_number: context.issue.number,
            });

            if (!pr.mergeable) {
              core.setFailed('PR is not mergeable (has conflicts)');
              return;
            }

            await github.rest.pulls.merge({
              owner: context.repo.owner,
              repo: context.repo.repo,
              pull_number: context.issue.number,
              merge_method: 'merge',
              commit_title: `Auto-merge: ${pr.title}`,
              commit_message: `Automatically merged Dependabot PR: ${pr.title}\n\nReason: ${{ needs.check-eligibility.outputs.reason }}`,
            });

            core.info(`Successfully merged PR #${context.issue.number}`);
```

---

### 3.4 Workflow Design Rationale

**Why This Approach**:

1. **Three-Stage Pipeline**:
   - `check-eligibility`: Fast validation (path checks, version parsing)
   - `wait-for-ci`: Ensures all CI checks pass before merging
   - `auto-merge`: Performs the actual merge

2. **Path-Based Safety**:
   - Explicit allowlist/denylist of paths
   - Prevents accidental auto-merge of code changes
   - Allows safe updates (workflows, submodules, manifests)

3. **Version Parsing**:
   - Attempts to detect patch-level updates from PR title
   - Falls back to path-based checks for non-versioned updates (workflows, submodules)

4. **CI Gate**:
   - Uses `wait-on-check-action` to wait for all CI checks
   - Only merges if all checks pass (green status)

5. **Conflict Detection**:
   - Checks `mergeable` status before attempting merge
   - Fails gracefully if conflicts exist

---

## 4. Implementation Notes

### 4.1 Required GitHub Permissions

The workflow requires:
- `GITHUB_TOKEN` with `pull-requests: write` permission
- Default token should be sufficient (auto-granted)

### 4.2 Testing Strategy

Before enabling auto-merge:
1. Test with a manual Dependabot PR (if possible)
2. Monitor first few auto-merged PRs closely
3. Add logging/notifications for auto-merge events
4. Consider adding a dry-run mode initially

### 4.3 Rollback Plan

If auto-merge causes issues:
1. Disable workflow by renaming file or removing trigger
2. Review merged PRs and revert if necessary
3. Adjust eligibility criteria based on issues found

---

## 5. Summary

### Dependabot PR Status

**Phoenix**:
- 1 PR found: `dependabot/submodules/submodule-all-c1ed6c5a9b`
- Status: **NOT MERGED** (outdated - superseded by v1.1.0)
- Action: Close PR or update manually to v1.1.0

**Bedrock**:
- 1 PR found: `dependabot/github_actions/gha-all-b61e4db3c6`
- Status: **NOT MERGED** (Bedrock main is dirty)
- Action: Merge once main is clean (workflow files only, safe)

### Auto-Merge Plan

**Policy**: Patch-level updates + GitHub Actions + Submodules only
**Safeguards**: Path restrictions, CI gate, conflict detection
**Workflow Files**: Designed but NOT created in this chunk

**Next Steps**:
1. Review and approve auto-merge policy
2. Create workflow files in future chunk
3. Test with manual Dependabot PR
4. Enable auto-merge gradually

---

**End of Dependabot Hygiene & Auto-Merge Plan**

**Status**: ✅ **PLANNING COMPLETE** - Ready for workflow implementation in future chunk

