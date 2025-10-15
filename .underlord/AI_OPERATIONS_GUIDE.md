
## Assisted Autonomy Between Breakpoints

**Authority:** UnderLord may **execute commands and edit files without per-line approvals** between approved breakpoints.

**What UnderLord Can Do Autonomously:**
- Create, modify, and commit files
- Execute git commands (add, commit, branch operations)
- Push branches to origin
- Run build/test commands
- Create and manage sprint branches
- Update documentation and metrics

**Boundaries (Must Ask First - "Restricted Actions"):**
- Changing branch protection rules or rulesets
- Rotating credentials or secrets
- Deleting remote branches or tags
- Force pushes (`git push --force` or `--force-with-lease`)
- Modifying release or version tags
- Any operation affecting main branch protection
- Changes to workflow permissions or secrets

**Breakpoint Requirements:**
- At each breakpoint, UnderLord must present:
  - Commit SHAs created
  - Branch names used
  - CI run URLs for validation
  - Summary of work completed

**Logging:**
- Between breakpoints: Batch command logs recorded in `.underlord/METRICS.md`
- No line-by-line approval prompts unless restricted action required
- Stop-the-Line events always pause for human decision

**Stop-the-Line Triggers (Always Ask):**
- Test failures
- CI failures
- Merge conflicts
- Unexpected errors
- Restricted actions encountered
- Uncertainty about correct approach


## Stage 0 Infrastructure Gate (Hard Requirement)

**Definition:** Infrastructure is NOT certified until `.underlord/preflight.sh` 
exits **0 in BOTH repositories** (phoenix, bedrock) on the target build host.

**Enforcement:**
- No sprint may be marked "complete" without a passing preflight in both repos.
- The full preflight stdout/stderr must be logged under `.underlord/logs/`.
- Each run must append a one-line summary to `METRICS.md`.
- Any preflight failure = infrastructure incomplete = sprint continues.

**What preflight validates:**
1. Git auth + remotes + recursive submodules
2. Toolchain: cmake, ninja, c/c++ compiler (versions recorded)
3. Third-party dependencies: OpenCASCADE >= 7.9.1, TBB >= 2021.5
4. CMake **configure** succeeds
5. Minimal **build** target compiles
6. (Optional) smoke tests run and pass

**Output artifacts:**
- `.underlord/logs/cmake-config-<timestamp>.log`
- `.underlord/logs/build-<timestamp>.log`
- `METRICS.md` line item with host, repo, durations, result

**Example METRICS entry:**
```
| 2025-10-15T22:01:29Z | ip-10-0-8-68 | phoenix | Stage0 | PASS | OCCT=7.9.1 TBB=2021.5 | Build certified |
```
