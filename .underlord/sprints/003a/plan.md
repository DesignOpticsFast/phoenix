# Sprint 3A Execution Plan

## Environment Summary

Both repositories are on `main` branch with clean working trees. UnderLord system is deployed in both repos with comprehensive tooling. Preflight checks reveal CMake configuration issues in both repos (likely dependency-related), which aligns with the known CodeQL architecture problems. Phoenix has upstream/origin remote setup, Bedrock has direct origin access. Both repos have active CodeQL workflows but Phoenix is experiencing architecture mismatch failures (x86_64 vs arm64) as identified in preflight analysis. The system is ready for Sprint 3A workflow maturation and clean slate operations.

## Pre-Flight Validation
- [x] Repos on main (both bedrock and phoenix)
- [x] State valid (UnderLord system operational)
- [x] Remotes reachable (both repos accessible)
- [x] Preflight script passes (with expected CMake issues noted)
- [x] Clean working tree in both repos

## Stage 1: CodeQL + CI Infrastructure [BP1] [≤2h]

### Workpackages

#### WP1.1: Phoenix CodeQL Architecture Fix - [branch: sprint-003/codeql-phoenix] - [est: 1.5h]
- **Tasks:**
  - Investigate CMake architecture detection in Phoenix
  - Update CodeQL workflow to use correct runner architecture
  - Test CodeQL workflow locally if possible
  - Update CI path filters to include `.underlord/**`
  - Adjust PR Guard thresholds for large deployments
- **Validation:**
  - CodeQL Analysis passes on Phoenix
  - CI workflow triggers for `.underlord/` changes
  - PR Guard accepts reasonable deployment sizes
- **Success criteria:**
  - Phoenix CodeQL Analysis check passes
  - CI path filters updated and tested
  - PR Guard thresholds adjusted
- **Rollback:** Restore previous CodeQL workflow, revert path filter changes
- **Git artifacts:** `.github/workflows/codeql.yml`, `.github/workflows/ci.yml`, `.github/workflows/pr-guard.yml`
- **State updates:** Record CodeQL fix completion, CI filter updates

#### WP1.2: Bedrock CI Validation - [branch: sprint-003/codeql-bedrock] - [est: 0.5h]
- **Tasks:**
  - Verify Bedrock CodeQL continues working
  - Update CI path filters to include `.underlord/**`
  - Ensure CI consistency between repos
- **Validation:**
  - Bedrock CodeQL Analysis still passes
  - CI triggers for `.underlord/` changes
- **Success criteria:**
  - Bedrock CodeQL remains green
  - CI path filters consistent across repos
- **Rollback:** Restore previous CI configuration
- **Git artifacts:** `.github/workflows/codeql.yml`, `.github/workflows/ci.yml`
- **State updates:** Record Bedrock CI validation

### Breakpoint 1 Criteria
- [ ] CodeQL issues resolved (Phoenix passing)
- [ ] CI path filters updated (both repos)
- [ ] CI green both repos
- [ ] State updated with BP1 validation

## Stage 2: Dependabot Automation [BP2] [≤2h]

### Workpackages

#### WP2.1: Dependabot Configuration Repair - [branch: sprint-003/dependabot] - [est: 1.5h]
- **Tasks:**
  - Diagnose current Dependabot issues
  - Update Dependabot configuration files
  - Configure auto-merge policies
  - Test with existing Dependabot PRs
- **Validation:**
  - Dependabot PRs flow correctly
  - Auto-merge works under policy
  - Adaptive toggling based on CI health
- **Success criteria:**
  - At least one test Dependabot PR successfully auto-merges
  - Adaptive auto-merge policy enabled
  - Dependabot configuration documented
- **Rollback:** Restore previous Dependabot configuration
- **Git artifacts:** `.github/dependabot.yml`, auto-merge settings
- **State updates:** Record Dependabot repair completion

#### WP2.2: Dependabot Integration Testing - [branch: sprint-003/dependabot-test] - [est: 0.5h]
- **Tasks:**
  - Create test Dependabot scenario
  - Validate auto-merge behavior
  - Document Dependabot workflow
- **Validation:**
  - Dependabot PRs auto-merge when CI passes
  - Adaptive behavior works correctly
- **Success criteria:**
  - Dependabot automation fully functional
  - Integration testing complete
- **Rollback:** Disable auto-merge, restore manual review
- **Git artifacts:** Dependabot test commits
- **State updates:** Record Dependabot testing results

### Breakpoint 2 Criteria
- [ ] Dependabot configuration repaired
- [ ] At least one test Dependabot PR successfully auto-merges
- [ ] Adaptive auto-merge policy enabled
- [ ] State updated with BP2 validation

## Stage 3: Doxygen + Documentation [Continuous] [≤3h]

### Workpackages

#### WP3.1: Doxygen Configuration - [branch: sprint-003/doxygen] - [est: 2h]
- **Tasks:**
  - Configure Doxygen for Phoenix (C++ application)
  - Configure Doxygen for Bedrock (C++20 library)
  - Add CMake build targets for documentation
  - Generate initial API documentation
- **Validation:**
  - Doxygen generates HTML successfully in both repos
  - At least one API element documented per repo
  - No Doxygen errors (warnings acceptable)
- **Success criteria:**
  - `cmake --build build --target docs` works in both repos
  - HTML documentation generated in `/docs/api`
  - API documentation covers public interfaces
- **Rollback:** Remove Doxygen configuration, restore CMake files
- **Git artifacts:** `Doxyfile`, CMake documentation targets, `/docs/api/` (ignored by Git)
- **State updates:** Record Doxygen configuration completion

#### WP3.2: Documentation Self-Test - [branch: sprint-003/docs-test] - [est: 1h]
- **Tasks:**
  - Create scripted documentation validation
  - Test all commands in `OPERATIONS.md`
  - Verify all referenced paths exist
  - Test all `.underlord/` scripts
  - Validate documentation URLs
- **Validation:**
  - All documentation commands execute without error
  - All scripts respond to `--help`
  - All documentation URLs return 200
  - Version numbers are current
- **Success criteria:**
  - Documentation self-test passes completely
  - All operational documentation verified
- **Rollback:** Restore previous documentation state
- **Git artifacts:** Documentation test scripts, updated documentation
- **State updates:** Record documentation validation results

### Stage 3 Criteria (No Breakpoint)
- [ ] Doxygen configured in both repos
- [ ] Documentation self-test passes
- [ ] All operational docs verified
- [ ] State updated with Stage 3 completion

## Stage 4: Sprint Close [Final Review] [≤1h]

### Workpackages

#### WP4.1: System Integrity Review (Light Mode) - [branch: sprint-003/sir-light] - [est: 0.5h]
- **Tasks:**
  - Run light mode SIR checks
  - Verify CI stability after all changes
  - Confirm all workflows operational
- **Validation:**
  - All CI checks pass
  - System integrity confirmed
  - No regressions detected
- **Success criteria:**
  - Light SIR passes cleanly
  - CI stability confirmed
- **Rollback:** Revert to pre-sprint state
- **Git artifacts:** SIR report, stability metrics
- **State updates:** Record SIR completion

#### WP4.2: Sprint Close and Merge - [branch: sprint-003] - [est: 0.5h]
- **Tasks:**
  - Merge all sub-branches to `sprint-003`
  - Prepare final merge to `main`
  - Update state and metrics
  - Generate sprint report
- **Validation:**
  - All changes integrated
  - Final CI run passes
  - Documentation complete
- **Success criteria:**
  - Sprint 3A complete
  - All done-done criteria met
  - Ready for human approval of `sprint-003` → `main` merge
- **Rollback:** Keep `sprint-003` branch, defer merge
- **Git artifacts:** Final sprint-003 branch, sprint report
- **State updates:** Final sprint metrics, completion status

### Final Review Criteria
- [ ] All done-done criteria met
- [ ] CI passes after final changes + one re-run
- [ ] All sprint branches ready for merge
- [ ] Human approval for `sprint-003` → `main` merge

## Risk Mitigation Strategies

### CodeQL Architecture Issues
- **Risk:** Phoenix CodeQL fix requires >2 days effort
- **Mitigation:** Timebox to 1.5h, escalate if not resolved
- **Fallback:** Document issue, defer to Sprint 3B

### Dependabot Deep Issues
- **Risk:** Dependabot problems deeper than configuration
- **Mitigation:** Timebox to 1 day, then escalate
- **Fallback:** Manual Dependabot management, document for future

### Cross-Repo Integration Issues
- **Risk:** Changes in one repo affect the other
- **Mitigation:** Test integration points, document dependencies
- **Fallback:** Defer complex integration to Sprint 3B

### Branch Model Overhead
- **Risk:** Too many branches for trivial changes
- **Mitigation:** Allow direct commits to `sprint-003` for typos/readme
- **Fallback:** Simplify branch model if overhead excessive

## Estimated Timeline

- **Stage 1:** 2h (CodeQL + CI fixes)
- **Stage 2:** 2h (Dependabot automation)
- **Stage 3:** 3h (Doxygen + documentation)
- **Stage 4:** 1h (Sprint close)
- **Total:** 8h (maximum allowed)

## Expected Git Artifacts

### Stage 1
- `sprint-003/codeql-phoenix` branch with CodeQL fixes
- `sprint-003/codeql-bedrock` branch with CI updates
- Updated workflow files in both repos

### Stage 2
- `sprint-003/dependabot` branch with configuration
- `sprint-003/dependabot-test` branch with testing
- Dependabot configuration files

### Stage 3
- `sprint-003/doxygen` branch with Doxygen setup
- `sprint-003/docs-test` branch with validation
- Documentation build targets and generated docs

### Stage 4
- `sprint-003/sir-light` branch with SIR results
- Final `sprint-003` branch ready for merge
- Sprint report and metrics

## State Management Schedule

- **Start of each stage:** Record intent in `.underlord/state.json`
- **After each breakpoint:** Record validation in state
- **End of sprint:** Final metrics snapshot
- **Risk events:** Log in `METRICS.md`

## Rollback Plans

### Stage 1 Rollback
- Restore previous CodeQL workflows
- Revert CI path filter changes
- Restore PR Guard thresholds

### Stage 2 Rollback
- Disable Dependabot auto-merge
- Restore previous Dependabot configuration
- Revert to manual review process

### Stage 3 Rollback
- Remove Doxygen configuration
- Restore previous CMake files
- Revert documentation changes

### Stage 4 Rollback
- Keep `sprint-003` branch unmerged
- Document completion status
- Prepare for Sprint 3B continuation

---

**PLAN READY FOR HUMAN REVIEW**
