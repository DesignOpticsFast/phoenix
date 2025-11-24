# Sprint Hygiene (Phoenix)

**Last Updated**: 2025-01-27  
**Status**: Active

## Branch Rules

1. **All sprint work must occur on a sprint branch** (e.g., `sprint/4.3`, `sprint4.2-bedrock-restart`)
2. **Never work directly on `main` branch**
3. **Branch naming pattern**: `sprint/X.Y` or `sprintX.Y-*` (e.g., `sprint4.2-bedrock-restart`)

## Working Tree Rules

1. **Tree must be clean before starting new work**
   - Commit or stash changes before beginning new tasks
   - No uncommitted changes allowed during daily ritual

2. **No work on main branch**
   - Even if on a sprint branch, check that main has no uncommitted changes

## Commit Hygiene

1. **Atomic commits**: Each commit should represent a single logical change
2. **Clear commit messages**: Use conventional commit format when possible
3. **No force-push**: Preserve history, use normal merges

## Daily Ritual Checks

The daily ritual (`make daily`) verifies:
- ✅ Current branch matches sprint pattern
- ✅ Tree is clean (no uncommitted changes)
- ✅ No work detected on main branch
- ✅ Branch is correct sprint branch

## Violations

Violations trigger FATAL errors in the daily ritual and require immediate correction before proceeding.

---

**This hygiene policy is mandatory and enforced during every daily ritual.**

