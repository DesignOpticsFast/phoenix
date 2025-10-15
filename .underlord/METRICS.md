# UnderLord Operations Metrics

**Purpose:** Track AI operations performance and drive continuous improvement  
**Last Updated:** 2025-10-15T18:00:00Z  
**Sprint:** 3A - Workflow Maturation & Clean Slate (Phase 1)  
**Status:** ABORTED (Stop-the-Line Event #2)

**Review Schedule:**
- **Daily:** During morning startup routine
- **Weekly:** Sprint progress review
- **Monthly:** Trend analysis and goal adjustment
- **Quarterly:** Strategic performance assessment

---

## Key Performance Indicators (KPIs)

### 1. Right First Time (RFT) Rate
**Definition:** Percentage of changes that work correctly without requiring revision

**Calculation:**
```
RFT Rate = (Successful Changes / Total Changes) × 100
```

**What Counts:**
- ✅ Code that builds and passes tests on first commit
- ✅ PRs merged without change requests
- ✅ Features that meet requirements immediately
- ❌ Code requiring rework
- ❌ Failed builds or tests
- ❌ PRs needing revision

**Target Thresholds:**
- 🟢 Good: ≥85%
- 🟡 Warning: 70-84%
- 🔴 Critical: <70%

---

### 2. Autonomy Score
**Definition:** Percentage of decisions made by AI without human intervention

**Calculation:**
```
Autonomy Score = (Autonomous Decisions / Total Decisions) × 100
```

**What Counts:**
- ✅ Implementation choices made independently
- ✅ Library selections
- ✅ Code structure decisions
- ✅ Test strategies
- ❌ Architecture decisions (always human)
- ❌ Merge approvals (always human)
- ❌ Breaking changes (requires approval)

**Target Thresholds:**
- 🟢 Good: 60-80% (sweet spot)
- 🟡 Warning: 40-59% or 81-90%
- 🔴 Critical: <40% or >90%

**Note:** Too high (>90%) may indicate insufficient oversight; too low (<40%) reduces efficiency

---

### 3. Sprint Velocity
**Definition:** Story points completed per sprint

**Calculation:**
```
Velocity = Sum of Completed Story Points
```

**Measurement:**
- Track points per sprint
- Calculate 3-sprint rolling average
- Identify trends (increasing/stable/decreasing)

**Target Thresholds:**
- 🟢 Good: Steady increase or stable high velocity
- 🟡 Warning: Declining trend
- 🔴 Critical: Velocity drops >30%

---

### 4. Test Coverage
**Definition:** Percentage of code covered by automated tests

**Calculation:**
```
Coverage = (Lines Executed in Tests / Total Lines) × 100
```

**Measurement:**
- Measured by pytest-cov
- Reported in CI/CD pipeline
- Tracked per module

**Target Thresholds:**
- 🟢 Good: ≥85%
- 🟡 Warning: 75-84%
- 🔴 Critical: <75%

---

### 5. Build Success Rate
**Definition:** Percentage of CI/CD builds that pass

**Calculation:**
```
Build Success = (Successful Builds / Total Builds) × 100
```

**What Counts:**
- ✅ All CI checks pass
- ❌ Any CI check fails (test, lint, build, security)

**Target Thresholds:**
- 🟢 Good: ≥95%
- 🟡 Warning: 85-94%
- 🔴 Critical: <85%

---

### 6. Mean Time to Recovery (MTTR)
**Definition:** Average time to fix broken builds or failed deployments

**Calculation:**
```
MTTR = Sum of Recovery Times / Number of Incidents
```

**Measurement:**
- Time from build failure to green build
- Time from deployment issue to resolution
- Tracked in incident reports

**Target Thresholds:**
- 🟢 Good: <30 minutes
- 🟡 Warning: 30-60 minutes
- 🔴 Critical: >60 minutes

---

## Current Metrics (January 2025)

### Sprint 001: Infrastructure Setup
**Date:** 2025-01-02 to 2025-01-06

| Metric | Value | Target | Status |
|--------|-------|--------|--------|
| RFT Rate | 75% | ≥85% | 🟡 Warning |
| Autonomy Score | 65% | 60-80% | 🟢 Good |
| Velocity | 21 points | Baseline | 🟢 Good |
| Test Coverage | 78% | ≥85% | 🟡 Warning |
| Build Success | 94% | ≥95% | 🟡 Warning |
| MTTR | 18 min | <30 min | 🟢 Good |

**Analysis:**
- First sprint, establishing baseline
- RFT and coverage below target but acceptable for infrastructure setup
- Build success good considering setup complexity
- MTTR excellent

---

### Sprint 002: Chat Features (In Progress)
**Date:** 2025-01-08 to 2025-01-12 (Day 3)

| Metric | Current | Target | Status |
|--------|---------|--------|--------|
| RFT Rate | 82% | ≥85% | 🟡 Warning |
| Autonomy Score | 70% | 60-80% | 🟢 Good |
| Velocity | TBD | 22+ points | In Progress |
| Test Coverage | 81% | ≥85% | 🟡 Warning |
| Build Success | 96% | ≥95% | 🟢 Good |
| MTTR | 15 min | <30 min | 🟢 Good |

**Trend:** ↗️ Improving across all metrics

**Notes:**
- RFT rate improving (+7% from Sprint 001)
- Autonomy score in optimal range
- Build success rate above target
- Coverage improving but still below target

---

## Historical Trends

### Week-over-Week Comparison

**Week 1 (Jan 2-6):**
- RFT: 75%
- Autonomy: 65%
- Coverage: 78%
- Velocity: 21 points

**Week 2 (Jan 8-12, partial):**
- RFT: 82% (↗️ +7%)
- Autonomy: 70% (↗️ +5%)
- Coverage: 81% (↗️ +3%)
- Velocity: In progress

**Trend:** Steady improvement across all metrics

---

### Monthly Trends

**January 2025 (partial):**
- Average RFT: 78.5%
- Average Autonomy: 67.5%
- Average Coverage: 79.5%
- Total Velocity: 21+ points

**Targets for February:**
- RFT: ≥85%
- Autonomy: 65-75%
- Coverage: ≥85%
- Velocity: ≥25 points/sprint

---

## Improvement Actions

### When RFT < 80%
**Triggers:**
- More than 20% of changes need revision
- Frequent test failures
- Build failures on first attempt

**Actions:**
1. Review requirements more thoroughly before implementation
2. Add pre-commit checks (lint, type check)
3. Write tests before implementation (TDD)
4. Increase code review rigor
5. Update GOTCHAS.md with lessons learned

---

### When Autonomy < 60%
**Triggers:**
- Too many decisions escalated to human
- Frequent blocking on approvals
- Uncertainty about requirements

**Actions:**
1. Create decision frameworks for common choices
2. Document preferred patterns in ARCHITECTURE_NOTES.md
3. Add examples to templates
4. Clarify autonomy boundaries in config.json
5. Review and update AI_OPERATIONS_GUIDE.md

---

### When Coverage < 75%
**Triggers:**
- Test coverage drops below threshold
- New code added without tests
- Tests not running in CI

**Actions:**
1. Require tests for all new features
2. Add coverage gates in CI/CD
3. Focus on under-covered modules
4. Write integration tests
5. Review test quality, not just quantity

---

### When MTTR > 30min
**Triggers:**
- Takes too long to fix broken builds
- Debugging is difficult
- Rollback procedures unclear

**Actions:**
1. Improve error messages and logging
2. Add better diagnostic tools
3. Create runbooks for common issues
4. Practice rollback procedures
5. Reduce change batch sizes

---

## Metrics Collection

### Automated Collection
**Source:** CI/CD Pipeline (GitHub Actions)

**Metrics:**
- Build success rate: GitHub Actions API
- Test coverage: pytest-cov reports
- Build times: GitHub Actions logs
- Deployment success: Deployment logs

**Frequency:** Per commit/PR

---

### Manual Collection
**Source:** Sprint reports and human observation

**Metrics:**
- RFT rate: Tracked in sprint reports
- Autonomy score: Decision tracking in plan.md
- Sprint velocity: Story point completion
- Qualitative feedback: Retrospectives

**Frequency:** Per sprint

---

### Tools & Scripts

**Coverage Collection:**
```bash
pytest --cov=src --cov-report=term --cov-report=html
```

**Build Metrics:**
```bash
gh api /repos/{org}/{repo}/actions/runs \
  --jq '.workflow_runs[] | {conclusion, created_at}'
```

**Custom Metrics:**
```python
# .underlord/scripts/collect_metrics.py
# Aggregates metrics from various sources
```

---

## Metric Definitions (Detailed)

### RFT Rate Scenarios

**Counts as Success (RFT = 1):**
- Commit builds and all tests pass
- PR approved and merged without changes
- Feature works as specified on first try
- Code review finds no issues

**Counts as Failure (RFT = 0):**
- Build fails on first attempt
- Tests fail after commit
- PR requires changes
- Bug found after merge requiring fix

**Edge Cases:**
- Minor formatting fixes: Still counts as failure
- Documentation-only changes: Not counted
- Reverts: Count as failures for original commit

---

### Autonomy Score Scenarios

**Autonomous Decisions (Score +1):**
- Chose library from approved list
- Structured code following patterns
- Named variables/functions appropriately
- Wrote tests without guidance
- Fixed simple bugs independently

**Requires Human Input (Score 0):**
- New architecture pattern needed
- Breaking API change proposed
- New dependency addition
- Security-related decision
- Merge approval

**Boundary Cases:**
- Refactoring existing code: Autonomous if patterns are clear
- Adding new endpoint: Autonomous if follows existing structure
- Changing config: Requires approval

---

## Dashboard (Future)

**Planned Features:**
- Real-time metric visualization
- Trend charts
- Automated alerts
- Comparative analysis
- Predictive insights

**Implementation:** Sprint 005+

---

**Version:** 1.0  
**Last Updated:** 2025-01-10  
**Next Review:** 2025-01-12 (End of Sprint 002)

## Infrastructure Certification (Sprint 3A-Infra)

**Date:** 2025-10-15  
**Status:** ✅ CERTIFIED  
**Duration:** ~2.5 hours (includes discovery and remediation)

### Issues Discovered & Resolved
1. **Fork Architecture:** Phoenix was forked (MarkBedrock/phoenix) - RESOLVED
2. **Duplicate Clones:** Two sets of repos (workspace/ and -check/) - CONSOLIDATED
3. **OAuth Scope:** Token lacked workflow scope - SSH implemented

### Final Architecture
- **Location:** `/home/ec2-user/workspace/{phoenix,bedrock}`
- **Remote:** Single SSH origin to `DesignOpticsFast/*` canonical repos
- **Authentication:** SSH keys (no forks, no OAuth issues)

### CI Verification
- **Phoenix CI Run:** https://github.com/DesignOpticsFast/phoenix/actions/runs/18537869143
  - Branch: infra-test-real-1760550632
  - Status: ✅ SUCCESS (1m 15s)
  - Triggered: Push to feature branch
  
- **Bedrock CI Run:** https://github.com/DesignOpticsFast/bedrock/actions/runs/18538030999
  - Branch: main
  - Status: ✅ SUCCESS (manual trigger)
  - Triggered: workflow_dispatch

### Constitutional Changes
- **Canonical-Clone Principle:** One working clone per repo at canonical location
- **No Forks Rule:** Forks prohibited for CI-validated sprint work
- **Stage 0 Gate:** Infrastructure validation required before sprint start

**Infrastructure certified sprint-ready.**

## 2025-10-15 — Sprint 3A-Infra Final Summary

sprint_3a_infra:
  status: COMPLETE
  duration: 4.0 hours
  start: 2025-10-15T09:40:00-07:00
  end: 2025-10-15T13:30:00-07:00

infrastructure:
  canonical_clone: true
  location: /home/ec2-user/workspace/phoenix
  ssh_auth: true
  ci_trigger_verified: true
  phoenix_ci_run: https://github.com/DesignOpticsFast/phoenix/actions/runs/18537869143
  bedrock_ci_run: https://github.com/DesignOpticsFast/bedrock/actions/runs/18538030999
  forks_present: false
  duplicate_clones: false
  certification: PASS

issues_resolved: 3 (fork architecture, duplicate clones, OAuth scope)
constitutional_changes: 4 (Canonical-Clone, No Forks, Stage 0 Gate, Autonomy Policy)
stop_the_line_events: 2 (missing push invariant, auth blocker)
ready_for_sprint_3: true

