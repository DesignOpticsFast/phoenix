# Sprint {NUMBER} Execution Plan

**Branch:** `sprint-{NUMBER}-{SLUG}`  
**Status:** ⏸️ Awaiting execution

---

## Pre-Flight Checklist
Run before starting any work:

- [ ] `.underlord/preflight.sh` passes
- [ ] Review `goals.md` and `context.md`
- [ ] Confirm branch: `sprint-{NUMBER}-{SLUG}`
- [ ] Working directory clean (`git status`)
- [ ] Latest main pulled (`git pull origin main`)

---

## Phase 1: {Major Task Name}

**Goal:** _What this phase accomplishes_

**Breakpoint:** 🛑 Human review required after implementation

### Implementation Steps:

1. [ ] Create `{file-path}` with `{functionality}`

2. [ ] Add unit tests in `tests/test_{feature}.cpp`

3. [ ] Update `CMakeLists.txt` if needed

4. [ ] Build locally
   ```bash
   rm -rf build && cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
   cmake --build build -j
   ```

5. [ ] Run tests
   ```bash
   ctest --test-dir build --output-on-failure
   ```

### Verification Checklist:
- [ ] Code compiles without warnings
- [ ] All tests pass (existing + new)
- [ ] No regressions detected
- [ ] Code follows project standards

**Outcome:** _Expected result after this phase_

---

## Phase 2: {Major Task Name}

**Goal:** _What this phase accomplishes_

**Breakpoint:** 🛑 Human review required after CI passes

### Integration Steps:

1. [ ] Review changes
   ```bash
   git diff
   git status
   ```

2. [ ] Commit with descriptive message
   ```bash
   git add {files}
   git commit -m "feat({scope}): {description}"
   ```

3. [ ] Push to remote
   ```bash
   git push -u origin sprint-{NUMBER}-{SLUG}
   ```

4. [ ] Create Pull Request
   ```bash
   gh pr create --title "Sprint {NUMBER}: {TITLE}" --body "..."
   ```

5. [ ] Monitor CI checks
   ```bash
   gh pr checks --watch
   ```

### Verification Checklist:
- [ ] All CI checks green
- [ ] No merge conflicts
- [ ] PR description complete
- [ ] Code review requested

**Outcome:** _PR created and CI passing_

---

## Phase 3: Review & Merge

**Goal:** Complete sprint and integrate to main

**Breakpoint:** 🛑 Human approval required before merge

### Final Steps:

1. [ ] Human code review completed
2. [ ] Address review feedback (if any)
3. [ ] Verify all checks still green after updates
4. [ ] Approve and merge
   ```bash
   gh pr review --approve
   gh pr merge --squash --auto
   ```
5. [ ] Verify merge to main
   ```bash
   git checkout main
   git pull --ff-only
   ```

### Verification Checklist:
- [ ] Sprint goals met (reference goals.md)
- [ ] All success criteria satisfied
- [ ] No regressions in main
- [ ] Documentation current

**Outcome:** _Sprint complete, changes merged to main_

---

## Emergency Procedures

### If Build Fails Unexpectedly
```bash
rm -rf build
.underlord/preflight.sh
```

### If CI Fails But Local Passes
1. Check CI logs: `gh run view <run-id> --log`
2. Compare environments
3. Update CMake configuration if needed

---

## Notes
_Add any sprint-specific notes, discoveries, or decisions here as you progress_

---

**Last Updated:** {YYYY-MM-DD}
