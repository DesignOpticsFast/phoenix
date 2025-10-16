# Sprint History

**Purpose:** Chronological index of all completed sprints

**Format:** Newest first

---

## Sprint 003: Ccache Integration & Build Performance
**Date:** 2025-10-15  
**Branch:** `feat/ccache-integration` (both repos)  
**Status:** Complete

**Goal:** Install and configure ccache for dramatic build speed improvements

**Deliverables:**
- ✅ ccache 4.9.1 installed and configured (20G cache, AL2023)
- ✅ CMake integration via compiler launcher variables
- ✅ GitHub Actions CI cache integration (both repos)
- ✅ Two-pass build verification: 100% cache hit rate achieved
- ✅ Documentation updated (DEPENDENCIES.md, SPRINT_HISTORY.md)

**Performance Results:**
- **Phoenix:** 100% cache hit rate on rebuild (46/46 compilations cached)
- **Bedrock:** 100% cache hit rate on rebuild (18/18 compilations cached)
- **Cache Size:** 20GB configured, ~0.01GB used
- **Build Time Improvement:** Near-instantaneous rebuilds

**Technical Implementation:**
- Built ccache from source (not available in AL2023 repos)
- CMake launcher variables: `CMAKE_C_COMPILER_LAUNCHER=ccache`
- CI cache keying: `ccache-${{ runner.os }}-${{ matrix.compiler }}-${{ hashFiles('**/CMakeLists.txt') }}`
- Environment: `CCACHE_DIR=/var/cache/ccache`, `CCACHE_BASEDIR=/home/ec2-user/workspace`

**Lessons Learned:**
- ccache significantly improves developer experience
- CI cache integration essential for consistent performance
- Source build required for AL2023 (package not available)

**Policy Change:** 2025-10-16: PR Guard updated to allow internal, infra-ci–labeled edits to .github/workflows/ci.yml; retains blocks for .env, secrets, CMakePresets.json.

---

## Sprint 007 (Phase 7): Workflow Validation
**Date:** 2025-10-14  
**Branch:** `chore/identity-sample` (Bedrock), `chore/about-menu` (Phoenix)  
**Status:** Complete (Bedrock) / 90% (Phoenix - CodeQL pending)

**Goal:** Prove Cursor + UnderLord autonomous development workflow

**Deliverables:**
- ✅ Bedrock: `identity()` function + unit test - [PR #39](https://github.com/DesignOpticsFast/bedrock/pull/39)
- ⚠️ Phoenix: Help → About dialog - [PR #28](https://github.com/DesignOpticsFast/phoenix/pull/28)

**Outcomes:**
- Full development loop validated
- UnderLord operated autonomously
- Infrastructure proven production-ready
- Discovered CodeQL configuration difference

**Challenges:**
- GTest not pre-installed (resolved)
- Push permissions required fork workflow
- Phoenix CodeQL failing (investigation ongoing)

**Lessons Learned:**
- Pre-flight automation critical
- Fork PR workflow effective fallback
- Need CodeQL config alignment

[Full Report](../sprints/archive/phase-07/report.md)

---

## Phase 1-6: Infrastructure Setup
**Date:** 2025-10-01 through 2025-10-11  
**Status:** Complete

**Summary:** Established CI/CD, branch protection, CodeQL, and development workflows

**Key Achievements:**
- GitHub Actions CI pipelines
- Ruleset-based branch protection
- CodeQL Advanced mode
- PR Guard workflows
- Authentication configured

**Documentation:** See `underlord-state.json`

---

## Template for Future Sprints

```markdown
## Sprint {NUMBER}: {TITLE}
**Date:** {YYYY-MM-DD}  
**Branch:** `sprint-{NUMBER}-{slug}`  
**Status:** Complete / Partial / Blocked

**Goal:** {One-sentence objective}

**Deliverables:**
- ✅ {Item} - [PR #{num}](url)

**Outcomes:**
- {Key result}

**Challenges:**
- {Challenge} (resolution: {how solved})

**Lessons Learned:**
- {Insight}

[Full Report](../sprints/archive/sprint-{NUMBER}/report.md)

---
```

---

**Maintenance:** Add entries in reverse chronological order (newest first)

---

## Sprint 3A-Infra - Infrastructure Readiness Validation

Date: 2025-10-15 (9:40 AM - 1:30 PM PT)
Duration: 4 hours
Status: COMPLETE - Infrastructure Certified

Issues Resolved:
1. Phoenix fork architecture (blocked CI)
2. Duplicate repository clones (confusion)
3. OAuth token insufficient scope (blocked workflow changes)

Final Architecture: /home/ec2-user/workspace/{phoenix,bedrock} with SSH auth and single origin remotes

Constitutional Changes:
1. Canonical-Clone Principle
2. No Forks Rule
3. Stage 0 Infrastructure Gate
4. Assisted Autonomy Policy

Evidence:
- Phoenix CI: https://github.com/DesignOpticsFast/phoenix/actions/runs/18537869143
- Bedrock CI: https://github.com/DesignOpticsFast/bedrock/actions/runs/18538030999

Outcome: Environment certified sprint-ready. Sprint 3 cleared to proceed.

