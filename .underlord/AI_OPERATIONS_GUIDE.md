
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

