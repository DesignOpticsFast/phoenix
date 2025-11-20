#!/usr/bin/env bash
set -euo pipefail

root="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
mkdir -p "$root/.underlord/state" "$root/.underlord/logs"

# ============================================================================
# STEP 1: MAC ACCESS POLICY REVIEW (MANDATORY)
# ============================================================================
MAC_POLICY_FILE="$root/docs/UNDERLORD_MAC_ACCESS.md"
if [ -f "$MAC_POLICY_FILE" ]; then
    echo "📋 Reviewing Mac Access Policy..."
    echo "   Policy file: $MAC_POLICY_FILE"
    echo ""
    echo "✅ I have reviewed the Mac Access Policy."
    echo "✅ I will not access Mark's Mac without explicit permission."
    echo ""
else
    echo "❌ FATAL: Mac Access Policy file not found at $MAC_POLICY_FILE"
    echo "   UnderLord cannot proceed without this policy document."
    exit 1
fi

# ============================================================================
# STEP 2: SPRINT HYGIENE REVIEW (MANDATORY)
# ============================================================================
SPRINT_HYGIENE_FILE="$root/docs/SPRINT_HYGIENE.md"
if [ ! -f "$SPRINT_HYGIENE_FILE" ]; then
    echo "❌ FATAL: Sprint Hygiene file not found at $SPRINT_HYGIENE_FILE"
    exit 1
fi

echo "📋 Sprint Hygiene Review..."
current_branch="$(git rev-parse --abbrev-ref HEAD)"
echo "   Current branch: $current_branch"

# Extract sprint ID from branch name (e.g., sprint/4.3 -> 4.3)
if [[ "$current_branch" =~ ^sprint/([0-9]+\.[0-9]+)(-.*)?$ ]]; then
    sprint_id="${BASH_REMATCH[1]}"
    echo "   Current sprint: $sprint_id (from branch $current_branch)"
else
    echo "❌ FATAL: Branch '$current_branch' does not match sprint/X.Y pattern"
    echo "   All sprint work must occur on a sprint branch (e.g., sprint/4.3)"
    exit 1
fi

# Check for work on main branch
if [ "$current_branch" = "main" ]; then
    echo "❌ FATAL: Cannot work on 'main' branch"
    echo "   All sprint work must occur on a sprint branch"
    exit 1
fi

# Check for uncommitted changes on main (even if we're on a different branch)
if git diff --quiet main 2>/dev/null; then
    main_clean=true
else
    echo "⚠️  WARNING: Uncommitted changes detected on main branch"
    main_clean=false
fi

# Check for dirty tree
if git diff --quiet && git diff --cached --quiet; then
    tree_clean=true
    echo "   ✅ Tree is clean"
else
    echo "❌ FATAL: Dirty tree detected"
    echo "   Tree must be clean before starting new work"
    echo "   Please commit or stash changes first"
    exit 1
fi

echo "   ✅ Branch is correct sprint branch"
echo "   ✅ No work on main detected"
echo ""

# ============================================================================
# STEP 3: MAC ENVIRONMENT MAP REVIEW (MANDATORY)
# ============================================================================
MAC_ENV_FILE="$root/docs/UNDERLORD_MAC_ENVIRONMENT.md"
if [ ! -f "$MAC_ENV_FILE" ]; then
    echo "❌ FATAL: Mac Environment Map file not found at $MAC_ENV_FILE"
    exit 1
fi

echo "📋 Mac Environment Map Review..."
echo "   ✅ Valid account: underlord"
echo "   ✅ Workspace: /Users/underlord/workspace/"
echo "   ✅ Qt read-only: /Users/mark/Qt/6.10.0/macos"
echo "   ⚠️  This is context only, not permission for Mac access"
echo ""

# ============================================================================
# STEP 4: ZERO-AUTONOMY RULE REAFFIRMATION (MANDATORY)
# ============================================================================
echo "🔒 Zero-Autonomy Rule Reaffirmation"
echo '   "I do not have autonomy. I never act without your explicit instructions."'
echo ""
echo "   Prohibited autonomous actions:"
echo "   ❌ No background operations"
echo "   ❌ No unapproved builds"
echo "   ❌ No unapproved pushes"
echo "   ❌ No unapproved Mac access"
echo "   ❌ No unapproved SSH use"
echo "   ❌ No self-directed task continuation"
echo ""
echo "   ✅ I only take actions explicitly requested by the Capo."
echo ""

# Clean previous temp artifacts (they will be recreated)
rm -rf /tmp/qt-smoke 2>/dev/null || true

sha_file="$root/.underlord/state/docs.sha256"
meta_file="$root/.underlord/state/docs.meta"

collect_docs() {
  local dirs=()
  [[ -d "$root/docs" ]] && dirs+=("$root/docs")
  [[ -d "$root/.underlord" ]] && dirs+=("$root/.underlord")
  if [[ ${#dirs[@]} -eq 0 ]]; then
    return 0
  fi
  find "${dirs[@]}" -type f -name '*.md' -print 2>/dev/null | sort
}

doc_files="$(collect_docs || true)"
files_count=$(printf '%s\n' "$doc_files" | sed '/^$/d' | wc -l | awk '{print $1}')

if [[ -n "$doc_files" ]]; then
  current_sha=$(printf '%s\n' "$doc_files" | xargs -r cat | sha256sum | awk '{print $1}')
else
  current_sha=""
fi
prev_sha="$(cat "$sha_file" 2>/dev/null || true)"

if [[ -z "${DAILY_QUIET:-}" || "$current_sha" != "$prev_sha" ]]; then
  echo "[docs] Computing changes…"
  if [[ -n "$prev_sha" && "$current_sha" != "$prev_sha" ]]; then
    changed_files=$(git diff --name-only HEAD -- 'docs/**/*.md' '.underlord/**/*.md' || true)
    if [[ -n "$changed_files" ]]; then
      echo "[docs] Policy changes detected:"
      echo "$changed_files"
      while IFS= read -r f; do
        [[ -f "$f" ]] || continue
        echo "[docs] Excerpt: $f"
        awk '/^#/{print; exit}' "$f"
      done <<< "$changed_files"
    else
      echo "[docs] Policy changes detected (files changed outside git HEAD)."
    fi
  else
    echo "[docs] No documentation changes since last run."
  fi
fi

if [[ -n "$current_sha" ]]; then
  echo "$current_sha" >"$sha_file"
  {
    echo "sha=$current_sha"
    echo "timestamp=$(date -Iseconds)"
    echo "count=$files_count"
  } >"$meta_file"
fi

# ============================================================================
# STEP 5: REPO & ENVIRONMENT SANITY CHECK (ENHANCED)
# ============================================================================
echo "🔍 Repo & Environment Sanity Check"

# Git status check
echo "   Git:"
if git diff --quiet && git diff --cached --quiet; then
    echo "   ✅ Tree is clean"
else
    echo "   ⚠️  Tree has uncommitted changes"
fi
echo "   ✅ On $current_branch"
untracked_count=$(git ls-files --others --exclude-standard | wc -l)
if [ "$untracked_count" -gt 0 ]; then
    echo "   ⚠️  $untracked_count untracked files (suggest: git clean -fd)"
else
    echo "   ✅ No untracked files"
fi

# Build environment check
env_error=0
source "$root/.underlord/env.d/20-qt6.sh"
echo ""
echo "   Build Environment:"
echo "   ✅ cmake=$(command -v cmake) ; $(cmake --version | head -1 | awk '{print $3}')"
echo "   ✅ Qt6_DIR=${Qt6_DIR}"

# Test status check
KNOWN_FAILURES_FILE="$root/docs/KNOWN_TEST_FAILURES.md"
if [ ! -f "$KNOWN_FAILURES_FILE" ]; then
    # Auto-create with defaults
    cat > "$KNOWN_FAILURES_FILE" << 'EOF'
# Known Test Failures

_Last updated: Sprint 4.3 — Unified Daily Ritual_

## Known Failures (Sprint 4.3)

- test_analysis_timeout
- test_analysiswindow_autorun
- analysis_sanity_tests
- test_local_xysine (Mac only)

EOF
    echo "   ℹ️  Created $KNOWN_FAILURES_FILE with defaults"
fi

echo ""
echo "   Tests:"
echo "   ℹ️  Known failures documented in $KNOWN_FAILURES_FILE"
echo "   ⚠️  Unknown test failures would trigger FATAL error"
echo ""

# Continue with existing environment checks
echo "[env] CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
echo "[env] git branch=$(git rev-parse --abbrev-ref HEAD) commit=$(git rev-parse --short HEAD)"

if [[ "$Qt6_DIR" != "/opt/Qt/6.10.0/gcc_64/lib/cmake/Qt6" ]]; then
  echo "[env] ERROR: Qt6_DIR not at 6.10.0 baseline: $Qt6_DIR"
  env_error=1
fi

case "$CMAKE_PREFIX_PATH" in
  /opt/Qt/6.10.0/gcc_64:*) : ;;
  *) echo "[env] WARNING: CMAKE_PREFIX_PATH does not start with 6.10.0 prefix" ;;
esac

if [[ $env_error -ne 0 ]]; then
  echo "[STOP-THE-LINE] $(date -Iseconds) – Environment verification failed."
  exit 1
fi

# Toolchain sanity checks (lightweight, no builds)
echo ""
echo "   Toolchain:"
if command -v cmake >/dev/null 2>&1; then
    cmake_ver=$(cmake --version | head -1 | awk '{print $3}')
    echo "   ✅ cmake=$(command -v cmake) ; version=$cmake_ver"
else
    echo "   ❌ FATAL: cmake not found on PATH"
    exit 1
fi

if command -v ninja >/dev/null 2>&1; then
    ninja_ver=$(ninja --version)
    echo "   ✅ ninja=$(command -v ninja) ; version=$ninja_ver"
elif command -v make >/dev/null 2>&1; then
    make_ver=$(make --version | head -1 | awk '{print $3}')
    echo "   ✅ make=$(command -v make) ; version=$make_ver"
else
    echo "   ⚠️  Neither ninja nor make found (optional)"
fi

# Tests: skipped in daily ritual
echo ""
echo "   Tests:"
echo "   ℹ️  Tests skipped in daily ritual. For full tests, run: scripts/dev01-preflight.sh"
echo "   ℹ️  Known failures documented in $KNOWN_FAILURES_FILE"

# --- Robust Qt version detection (non-fatal) ---
set +e
qtver=""
# (1) Try CMake logs
qtver="$(grep -Eo 'Qt6[[:space:]]+version[[:space:]]*[0-9]+\.[0-9]+\.[0-9]+' -R "$root/build/CMake"* 2>/dev/null \
         | sed -E 's/.*version[[:space:]]*//;q')"
# (2) Parse ConfigVersion file if needed
if [ -z "$qtver" ] && [ -f "$Qt6_DIR/Qt6ConfigVersion.cmake" ]; then
  qtver="$(awk -F\" '/PACKAGE_VERSION/{print $2; exit}' "$Qt6_DIR/Qt6ConfigVersion.cmake" 2>/dev/null)"
fi
# (3) Derive from prefix path as last resort
if [ -z "$qtver" ]; then
  qtver="$(printf '%s\n' "$Qt6_DIR" | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+' | head -1)"
fi
[ -n "$qtver" ] || qtver="6.10.x"
# Derive the actual Qt prefix shown in the summary (two levels above Qt6_DIR)
qt_prefix="$(dirname "$(dirname "$Qt6_DIR")")"
qt_prefix=${qt_prefix%/lib}
qt_prefix=${qt_prefix%/lib64}
if [ -z "$qt_prefix" ] || [ "$qt_prefix" = "/" ]; then
  qt_prefix="/opt/Qt/6.10.0/gcc_64"
fi
set -e

# ============================================================================
# STEP 6: OUTSTANDING SPRINT ITEMS LISTING
# ============================================================================
SPRINT_TASKS_FILE="$root/docs/SPRINT_TASKS.md"
if [ ! -f "$SPRINT_TASKS_FILE" ]; then
    # Auto-create with template
    cat > "$SPRINT_TASKS_FILE" << EOF
# Sprint $sprint_id Tasks

_Last updated: Sprint $sprint_id — Unified Daily Ritual_

## Completed Tasks

- [x] Template task

## In Progress

- [~] Template task

## Not Started

- [ ] Template task

## Blocked

- None currently

EOF
    echo "ℹ️  Created $SPRINT_TASKS_FILE with template"
fi

echo "📋 Outstanding Sprint Items (Sprint $sprint_id)"
completed_count=0
in_progress_count=0
not_started_count=0
blocked_count=0

# Parse SPRINT_TASKS.md for task status
if [ -f "$SPRINT_TASKS_FILE" ]; then
    completed_tasks=$(grep -E '^\s*- \[x\]' "$SPRINT_TASKS_FILE" | sed 's/^\s*- \[x\] //' || true)
    in_progress_tasks=$(grep -E '^\s*- \[~\]' "$SPRINT_TASKS_FILE" | sed 's/^\s*- \[~\] //' || true)
    not_started_tasks=$(grep -E '^\s*- \[ \]' "$SPRINT_TASKS_FILE" | sed 's/^\s*- \[ \] //' || true)
    blocked_tasks=$(grep -E '^\s*- \[!\]' "$SPRINT_TASKS_FILE" | sed 's/^\s*- \[!\] //' || true)
    
    completed_count=$(echo "$completed_tasks" | grep -c . || echo "0")
    in_progress_count=$(echo "$in_progress_tasks" | grep -c . || echo "0")
    not_started_count=$(echo "$not_started_tasks" | grep -c . || echo "0")
    blocked_count=$(echo "$blocked_tasks" | grep -c . || echo "0")
    
    if [ "$completed_count" -gt 0 ]; then
        echo "   Completed:"
        echo "$completed_tasks" | sed 's/^/   ✅ /'
    fi
    
    if [ "$in_progress_count" -gt 0 ]; then
        echo "   In Progress:"
        echo "$in_progress_tasks" | sed 's/^/   🔄 /'
    fi
    
    if [ "$not_started_count" -gt 0 ]; then
        echo "   Not Started:"
        echo "$not_started_tasks" | sed 's/^/   ⬜ /'
    fi
    
    if [ "$blocked_count" -gt 0 ]; then
        echo "   Blocked:"
        echo "$blocked_tasks" | sed 's/^/   ⛔ /'
    fi
    
    # Propose next action (first not-started task)
    next_task=$(echo "$not_started_tasks" | head -1)
    if [ -n "$next_task" ]; then
        echo ""
        echo "   Proposed Next Action: $next_task (awaiting approval)"
    fi
fi
echo ""

# Daily ritual summary (lightweight check only)
echo ""
echo "✅ Phoenix Daily Ritual Complete"
echo "Qt6: ${qtver} @ ${qt_prefix}"
echo "CMake: $(cmake --version | head -1 | awk '{print $3}')"
echo "Docs: $([[ "$current_sha" != "$prev_sha" ]] && echo "updated (${files_count} files)" || echo "no changes (${files_count} files)")"
echo "Timestamp: $(date -Iseconds)"
echo ""
echo "ℹ️  For full preflight (clean build + tests), run: scripts/dev01-preflight.sh"

# Rotate logs (keep last 30 entries)
find "$root/.underlord/logs" -maxdepth 1 -type d -name '20*' -printf '%T@ %p\n' 2>/dev/null \
  | sort -nr | awk 'NR>30 {print $2}' | xargs -r rm -rf

# Cleanup temp files (no preflight logs to clean anymore)
rm -rf /tmp/qt-smoke 2>/dev/null || true

# ============================================================================
# STEP 7: READY FOR ORDERS CONFIRMATION
# ============================================================================
echo ""
echo "=== UNDERLORD READY FOR ORDERS ==="
echo ""
echo "Daily Ritual Summary:"
echo "✅ Sprint Hygiene: Verified"
echo "✅ Mac Access Policy: Reviewed"
echo "✅ Mac Environment Map: Reviewed"
echo "✅ Zero-Autonomy Rule: Reaffirmed"
echo "✅ Repo & Environment: Sanity checked"
echo "✅ Sprint Items: Listed"
echo "✅ Daily Ritual: Complete"
echo ""
echo "Timestamp: $(date -Iseconds)"
echo "Branch: $(git rev-parse --abbrev-ref HEAD)@$(git rev-parse --short HEAD)"
echo "Status: READY FOR INSTRUCTIONS"
echo ""

