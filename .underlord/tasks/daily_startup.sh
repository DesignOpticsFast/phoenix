#!/usr/bin/env bash
set -euo pipefail

root="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
mkdir -p "$root/.underlord/state" "$root/.underlord/logs"

# Clean previous temp artifacts (they will be recreated)
rm -rf /tmp/qt-smoke /tmp/preflight.log 2>/dev/null || true

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

env_error=0
source "$root/.underlord/env.d/20-qt6.sh"
echo "[env] cmake=$(command -v cmake) ; $(cmake --version | head -1)"
echo "[env] Qt6_DIR=${Qt6_DIR}"
echo "[env] CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
echo "[env] git branch=$(git rev-parse --abbrev-ref HEAD) commit=$(git rev-parse --short HEAD)"
git diff --quiet || echo "[env] WARNING: uncommitted changes present"

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

echo "[preflight] Qt smoke check…"
smoke_status=0
if "$root/scripts/dev/qt6_smoke.sh" /tmp/qt-smoke; then
  echo "[preflight] Qt smoke: OK"
else
  echo "[preflight] Qt smoke: FAILED (continuing); see /tmp/qt-smoke/configure.log"
  smoke_status=1
fi

set +e
"$root/.underlord/preflight.sh" |& tee /tmp/preflight.log
preflight_rc=${PIPESTATUS[0]}
set -e

failures=0
[[ $smoke_status -ne 0 ]] && failures=1
[[ $preflight_rc -ne 0 ]] && failures=1

if [[ $failures -ne 0 ]]; then
  stamp="$(date +%Y%m%d-%H%M%S)"
  log_dir="$root/.underlord/logs/$stamp"
  mkdir -p "$log_dir"
  [[ -f /tmp/preflight.log ]] && cp /tmp/preflight.log "$log_dir/"
  [[ -f /tmp/qt-smoke/configure.log ]] && cp /tmp/qt-smoke/configure.log "$log_dir/" || true
  [[ -f "$root/build/CMakeCache.txt" ]] && cp "$root/build/CMakeCache.txt" "$log_dir/" || true
  echo "$(date -Iseconds) | $(git rev-parse --abbrev-ref HEAD)@$(git rev-parse --short HEAD) | PRELIGHT FAIL | logs=$log_dir" >> "$root/.underlord/logs/stop_the_line.log"
  echo "[STOP-THE-LINE] See $log_dir and /tmp/preflight.log"
  exit 1
fi

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

echo "✅ Phoenix Preflight Complete
Qt6: ${qtver} @ ${qt_prefix}
CMake: $(cmake --version | head -1 | awk '{print $3}')
Build/Test: PASS
Docs: $([[ "$current_sha" != "$prev_sha" ]] && echo "updated (${files_count} files)" || echo "no changes (${files_count} files)")
Timestamp: $(date -Iseconds)"

# Rotate logs (keep last 30 entries)
find "$root/.underlord/logs" -maxdepth 1 -type d -name '20*' -printf '%T@ %p\n' 2>/dev/null \
  | sort -nr | awk 'NR>30 {print $2}' | xargs -r rm -rf

rm -rf /tmp/qt-smoke /tmp/preflight.log 2>/dev/null || true

