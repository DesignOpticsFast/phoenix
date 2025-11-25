# Fixup Sprint Plan v2 - Review & Recommendations

**Reviewer**: UnderLord  
**Date**: 2025-01-XX  
**Status**: Pre-Implementation Review

---

## Executive Summary

The v2 plan **addresses all critical gaps** from the previous review and is **well-structured**. The Phase 0 decisions are clear, and the workstreams are logically sequenced. However, there are **5 areas that need clarification or refinement** before implementation begins.

**Overall Assessment**: ✅ **APPROVE WITH MINOR REFINEMENTS**

---

## ✅ Strengths

1. **Phase 0 Decisions Locked In**: All 4 critical gaps from previous review are resolved
2. **Clear Gates**: Each workstream has explicit, testable gates
3. **Realistic Scope**: C++23 deferred, demo mode clarified, no thermal diffusion
4. **Good Sequencing**: Transport → Concurrency → Testing → Toolchain → Docs → Versioning
5. **Chunk Sizing Defined**: Clear thresholds for small/medium/large chunks

---

## 🟡 Areas Needing Clarification

### Clarification 1: Demo Mode Scope & Implementation

**Issue**: Plan mentions "demo mode" with `PHOENIX_DEMO_MODE=1` environment variable, but:
- Code review said demo mode was removed (LocalExecutor deleted)
- `TECHNICAL_DEBT.md` mentions demo mode as opt-in
- Current codebase shows `XYSineDemo` is test-only

**Questions**:
1. **What is "demo mode" in this context?**
   - Is it a fallback when Bedrock is unavailable?
   - Is it a test-only path?
   - Is it something else entirely?

2. **Does demo mode still exist, or should it be removed entirely?**
   - If it exists: Where is it implemented? (I couldn't find it in current code)
   - If it should be removed: Should 1.4 be "Remove demo mode entirely" instead of "Clarify and constrain"?

**Recommendation**:
- **Add Phase 0.6**: Demo Mode Audit
  - Search codebase for any demo mode implementation
  - If found: Document current behavior, then clarify/constrain per plan
  - If not found: Update 1.4 to "Remove demo mode references from docs" (no code changes needed)

---

### Clarification 2: Old Format Removal Coordination Details

**Issue**: Plan says "Remove Bedrock's old-format fallback" but doesn't specify:
- What happens to `handleMessage(client, message)` function (line 230)?
- Should `#ifdef BEDROCK_WITH_TRANSPORT_DEPS` logic be simplified?
- What about the `#else` branch (line 794) that handles old format when transport deps are disabled?

**Current State** (from code inspection):
- `parseIncomingData()` has old-format fallback (line 785-793)
- `readMessage()` function exists for backward compatibility (line 682)
- `handleMessage()` exists for old format (line 230)
- `#else` branch handles old format when `BEDROCK_WITH_TRANSPORT_DEPS` is OFF

**Recommendation**:
- **Expand 1.1 with explicit steps**:
  1. Verify Phoenix only sends new format (audit `LocalSocketChannel`)
  2. Remove old-format fallback in `parseIncomingData()` (delete lines 785-793)
  3. Delete `readMessage()` function entirely (line 682-718)
  4. Delete `handleMessage(client, message)` overload (old format handler)
  5. Simplify `#ifdef BEDROCK_WITH_TRANSPORT_DEPS` logic (remove `#else` branch if transport deps are always required)
  6. Remove all "backward compatibility" comments

- **Add verification step**: Integration test that sends old format and verifies it's rejected (not silently ignored)

---

### Clarification 3: PalantirServer Code Review Fixes Scope

**Issue**: Plan mentions "all residual findings from the earlier PalantirServer code review" but doesn't specify:
- What are the specific findings?
- Are they documented somewhere?
- What about the job tracking maps (`jobClients_`, `jobCancelled_`, `jobThreads_`)?

**Current State** (from code inspection):
- Job tracking maps exist and are used
- Thread safety appears to be implemented (mutexes present)
- But the plan mentions "consistent handling of m_jobs / job maps" - what inconsistency exists?

**Recommendation**:
- **Add explicit list** to 1.3:
  - [ ] Verify job tracking maps are consistently protected by `jobMutex_`
  - [ ] Ensure `jobCancelled_` is checked before starting new jobs
  - [ ] Verify job cleanup on client disconnect
  - [ ] Add input validation to all RPC entry points
  - [ ] Reduce QByteArray copies in hot paths (if any exist)

- **Or**: Reference a specific code review document/issue list

---

### Clarification 4: Integration Test Harness Architecture

**Issue**: Plan says "C++ integration test harness (not Python)" but doesn't specify:
- Where does it live? (Phoenix repo? Bedrock repo? Separate test repo?)
- How does it start Bedrock? (In-process? Subprocess? Separate binary?)
- How does it handle Bedrock lifecycle? (Start/stop/cleanup)

**Recommendation**:
- **Add architecture decision** to 3.1:
  - **Location**: Recommend Phoenix `tests/integration/` (since Phoenix is the client)
  - **Bedrock startup**: 
    - Option A: In-process `PalantirServer` (simpler, faster)
    - Option B: Subprocess `bedrock` binary (more realistic, tests full stack)
    - **Recommendation**: Option A for speed, Option B for realism (or both?)
  - **Lifecycle**: Use RAII test fixtures to ensure cleanup

- **Add explicit test structure**:
  ```
  tests/integration/
    test_bedrock_phoenix_rpc.cpp
    test_fixtures.hpp  // Bedrock server fixture
    CMakeLists.txt
  ```

---

### Clarification 5: CI Platform & Matrix

**Issue**: Plan says "at least one Linux and one macOS environment" but doesn't specify:
- Which CI platform? (GitHub Actions? GitLab CI? Jenkins? Local runners?)
- What about Windows? (Is it in scope or explicitly out?)
- What about Crucible/dev-01? (Are these CI runners or just dev machines?)

**Recommendation**:
- **Add explicit CI specification** to 3.3:
  - **Platform**: [GitHub Actions / GitLab CI / Jenkins / Other]
  - **Matrix**:
    - Linux (Ubuntu 22.04? Other?)
    - macOS (latest? Specific version?)
    - Windows (in scope or deferred?)
  - **TSAN/ASAN**: Which platform gets TSAN? Which gets ASAN?
  - **Crucible/dev-01**: Are these CI runners or just documented dev environments?

---

## 🔧 Recommended Refinements

### Refinement 1: Add Explicit Dependencies Between Workstreams

**Current**: Plan implies sequencing but doesn't explicitly state dependencies.

**Recommendation**:
- **Add dependency graph**:
  - Workstream 1 → Workstream 3 (integration tests need correct protocol)
  - Workstream 2 → Workstream 3 (threading fixes needed before stress tests)
  - Workstream 1 + 2 → Workstream 5 (docs need stable code)
  - Workstream 4 → Workstream 5 (toolchain versions need to be locked before docs)

- **Or**: Add explicit note: "Workstreams 1-3 can proceed in parallel after Phase 0, but 3 depends on 1+2 being complete. Workstreams 4-6 should proceed sequentially."

---

### Refinement 2: Add Rollback Plans for High-Risk Changes

**Current**: Plan has gates but no explicit rollback plans.

**Recommendation**:
- **Add rollback plans** for:
  - **Workstream 1.1**: If old format removal breaks something, can we temporarily re-enable fallback?
  - **Workstream 4.1**: If Qt 6.10.1 causes issues, can we roll back to previous version?
  - **Workstream 3.3**: If CI integration breaks, can we disable it temporarily?

- **Format**: "Rollback plan: [specific steps to revert if gate fails]"

---

### Refinement 3: Clarify "Reasonable Coverage" Threshold

**Current**: Plan says "Target a reasonable coverage (not perfection, but enough to trust the core)" and "≥ agreed threshold (e.g. ~80%)".

**Recommendation**:
- **Lock in specific threshold**:
  - **Option A**: 80% for Palantir-related code
  - **Option B**: 70% for Palantir-related code
  - **Option C**: Per-file basis (critical paths ≥ 90%, helpers ≥ 60%)

- **Specify what counts**: Line coverage? Branch coverage? Function coverage?

---

### Refinement 4: Add Explicit Success Criteria for Each Gate

**Current**: Gates say "passes" or "green" but don't specify what that means.

**Recommendation**:
- **Add explicit success criteria** for key gates:
  - **Gate 1.1**: "Round-trip unit tests pass with 100% success rate, integration test rejects old format with ErrorResponse"
  - **Gate 2.1**: "TSAN reports zero data races, ASAN reports zero leaks or UAF"
  - **Gate 3.1**: "Integration suite runs 10 times consecutively with 100% pass rate (no flakiness)"
  - **Gate 4.1**: "Phoenix builds and runs on Crucible with Qt 6.10.1, all existing tests pass"

---

### Refinement 5: Add Time Estimates Per Workstream

**Current**: Plan doesn't estimate effort.

**Recommendation**:
- **Add rough time estimates** (for planning, not commitment):
  - Phase 0: 2-4 hours (audits and decisions)
  - Workstream 1: 8-12 hours (protocol cleanup)
  - Workstream 2: 6-10 hours (threading + security)
  - Workstream 3: 12-16 hours (integration tests + CI)
  - Workstream 4: 4-6 hours (toolchain)
  - Workstream 5: 16-24 hours (docs harmonization)
  - Workstream 6: 2-4 hours (versioning)
  - **Total**: ~50-76 hours

- **Or**: Mark as "TBD - will estimate during chunk breakdown"

---

## 📋 Specific Questions for Lyra/Mark

1. **Demo Mode**: What is the current state? Should 1.4 be "remove" or "clarify"?
2. **Old Format Removal**: Should we remove the `#else` branch when `BEDROCK_WITH_TRANSPORT_DEPS` is OFF?
3. **PalantirServer Fixes**: What are the specific "residual findings" that need to be addressed?
4. **Integration Tests**: In-process server or subprocess? Phoenix repo or Bedrock repo?
5. **CI Platform**: Which CI system? What's the matrix?
6. **Coverage Threshold**: What's the specific number? (80%? 70%? Per-file?)
7. **Time Estimates**: Should we add rough estimates now, or defer to chunk breakdown?

---

## ✅ Approval Status

**Overall**: ✅ **APPROVE WITH MINOR REFINEMENTS**

**Conditions**:
- Resolve 5 clarification questions before Workstream 1 begins
- Incorporate refinements 1-5 (dependencies, rollback plans, coverage threshold, success criteria, time estimates)
- Add explicit architecture decisions for integration tests and CI

**Ready to proceed once clarifications are resolved and refinements are incorporated.**

---

## Next Steps

1. **Lyra/Mark reviews this feedback** and resolves the 5 clarification questions
2. **Plan is updated** with refinements
3. **Architecture decisions are locked in** (integration tests, CI platform)
4. **UnderLord chat is opened** with finalized plan
5. **Implementation begins** with Phase 0

**Estimated additional time for clarifications**: 1-2 hours  
**Estimated total fixup sprint time**: 50-76 hours (unchanged, clarifications are mostly decision work)

