# Sprint Hygiene Rules

_Last updated: Sprint 4.3 — Unified Daily Ritual_

This document defines the mandatory hygiene rules for all sprint work in Phoenix.

---

## 1. Commit Message Format

**Always use this exact structure:**

```
S4.3-<ChunkName>: <Short Description>
```

**Examples:**
- `S4.3-Scale-1: Implement autoscaling`
- `S4.3-Graph-Style-1: Add plot styling`
- `S4.3-ValidationFix-1: Fix dangling ParamSpec pointers`

**Rules:**
- Sprint ID must match current sprint (e.g., `S4.3`)
- Chunk name should be descriptive and hyphenated
- Description should be concise but clear
- No trailing periods in commit messages

---

## 2. Branch Naming Rules

**Sprint work must occur only on a sprint branch:**

- Primary sprint branch: `sprint/4.3`
- Feature branches allowed: `sprint/4.3-scale`, `sprint/4.3-validation-fix`
- **NEVER work on `main`**

**Branch pattern:**
- Must match: `sprint/X.Y` or `sprint/X.Y-<feature>`
- Where `X.Y` is the sprint identifier (e.g., `4.3`)

---

## 3. Hygiene Violations (FATAL)

The following violations **must stop work immediately**:

1. **Active work on `main`**
   - No commits to `main`
   - No uncommitted changes on `main`
   - No merges to `main` without explicit approval

2. **Dirty tree when starting a chunk**
   - Tree must be clean before beginning new work
   - All previous work must be committed or stashed

3. **Wrong sprint branch**
   - Current branch must match active sprint pattern
   - Cannot work on `sprint/4.2` when active sprint is `4.3`

4. **Uncommitted changes on `main`**
   - Any uncommitted changes detected on `main` are fatal

**When a violation is detected:**
- Stop immediately
- Report the violation
- Do not proceed until resolved

---

## 4. Work Isolation Rules

- All sprint work happens inside the sprint branch
- No mixing of sprint work with `main` branch
- Each chunk should be atomic and self-contained
- Commits should be logical units (not "WIP" or "fix typo" commits)

---

## 5. Sprint Detection

The active sprint is determined by:
- Extracting the sprint ID from the current branch name
- Example: `sprint/4.3` → sprint ID is `4.3`
- If branch name does not match `sprint/X.Y` pattern → FATAL ERROR

---

## 6. Daily Ritual vs. Preflight

### `make daily` (Lightweight)
- **Purpose:** Quick connectivity and hygiene check
- **Runtime:** Seconds (not minutes)
- **What it does:**
  - Verifies sprint branch and clean tree
  - Checks toolchain availability (cmake, ninja/make)
  - Confirms environment paths are set
  - **Does NOT run builds or tests**
- **When to use:** Every day, before starting work

### Preflight Scripts (Heavy)
- **Purpose:** Full build and test validation
- **Runtime:** ~4 minutes (configure + build + tests)
- **What it does:**
  - Full clean build from scratch
  - Runs complete test suite (21 tests)
  - Validates build artifacts
- **When to use:** Before major changes, merges, or when explicitly needed
- **How to run:** `scripts/dev01-preflight.sh` or `.underlord/preflight.sh`

**Important:** `make daily` no longer invokes preflight. Preflight must be run explicitly when needed.

---

_End of Sprint Hygiene Rules_

---

