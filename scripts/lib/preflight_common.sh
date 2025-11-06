#!/usr/bin/env bash
set -euo pipefail

print_truth_lines() {
  echo "=== TRUTH LINES ==="
  echo "QT_VERSION=${QT_VERSION:-<unset>}"
  echo "CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH:-<unset>}"
  echo "PR required checks: gate (CodeQL runs on main only)"
  echo "Runner: dev-01 labels expected: [self-hosted, Linux, X64, dev-01, qt6]"
  echo "Build job runs-on required labels: [self-hosted, Linux, X64, dev-01, qt6]"
  echo "===================="
}

resolve_qt_prefix() {
  local cpp="${CMAKE_PREFIX_PATH:-}"
  echo "Searching for Qt6Config.cmake in CMAKE_PREFIX_PATH: ${cpp:-<unset>}" >&2
  IFS=':' read -ra paths <<< "$cpp"
  for p in "${paths[@]}"; do
    [[ -z "$p" ]] && continue
    if [[ -f "$p/lib/cmake/Qt6/Qt6Config.cmake" ]]; then
      echo "$p"; return 0
    fi
  done
  for p in /opt/Qt/6.10.0/gcc_64 /opt/Qt/6.9.3/gcc_64; do
    if [[ -f "$p/lib/cmake/Qt6/Qt6Config.cmake" ]]; then
      echo "$p"; return 0
    fi
  done
  echo "❌ Could not locate Qt6Config.cmake" >&2
  echo "   Searched CMAKE_PREFIX_PATH: ${cpp:-<unset>}" >&2
  echo "   Tried fallbacks: /opt/Qt/6.10.0/gcc_64, /opt/Qt/6.9.3/gcc_64" >&2
  echo "   Please set CMAKE_PREFIX_PATH or install Qt6." >&2
  return 1
}

build_cmake_prefix_list() {
  local qt_prefix="$1"
  local cpp="${2:-}"
  local list="$qt_prefix"
  IFS=':' read -ra extras <<< "$cpp"
  for p in "${extras[@]}"; do
    [[ -z "$p" || "$p" == "$qt_prefix" ]] && continue
    list="${list};${p}"
  done
  echo "$list"
}

# ---
# Sanitize path-like environment variables: strip newlines, CRs, collapse multiple colons.
# Usage: clean_var="$(sanitize_path_var "$RAW_VAR")"
sanitize_path_var() {
  local raw="${1:-}"
  echo "$raw" | tr -d '\n\r' | tr -s ':' | sed 's/^://;s/:$//'
}
# ---

preflight_configure_build() {
  local build_dir="${1:-build/dev-01-relwithdebinfo}"
  local verbose="${2:-false}"
  [[ "$verbose" == "true" ]] && set -x

  # Clean CMAKE_PREFIX_PATH using helper
  if [[ -n "${CMAKE_PREFIX_PATH:-}" ]]; then
    CMAKE_PREFIX_PATH="$(sanitize_path_var "$CMAKE_PREFIX_PATH")"
  else
    CMAKE_PREFIX_PATH="/opt/Qt/6.10.0/gcc_64"
  fi
  export CMAKE_PREFIX_PATH

  local qt_prefix
  qt_prefix="$(resolve_qt_prefix)" || exit 1
  echo "Qt prefix: $qt_prefix"

  local cmake_prefix_list
  cmake_prefix_list="$(build_cmake_prefix_list "$qt_prefix" "$CMAKE_PREFIX_PATH")"
  cmake_prefix_list="$(sanitize_path_var "$cmake_prefix_list")"  # final safety check
  export CMAKE_PREFIX_PATH="$cmake_prefix_list"

  # Timestamped dir avoids partial-clean cache collisions
  build_dir="${build_dir}-$(date +%s)"
  rm -rf "$build_dir"
  cmake -S . -B "$build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_PREFIX_PATH="$cmake_prefix_list"

  cmake --build "$build_dir" -j "$(nproc 2>/dev/null || echo 4)"

  if [[ -f "$build_dir/CTestTestfile.cmake" ]]; then
    ctest --test-dir "$build_dir" --output-on-failure
  else
    echo "No tests detected; skipping."
  fi
  
  # Prune old builds after successful build
  prune_old_builds "build/dev-01-relwithdebinfo-*" 2
}

# Build pruning: keep only the last 2 timestamped builds
prune_old_builds() {
  local build_pattern="${1:-build/dev-01-relwithdebinfo-*}"
  local keep_count="${2:-2}"
  
  if [[ -d build ]]; then
    # Find timestamped builds, sort by modification time (newest first)
    local builds
    builds=($(find build -maxdepth 1 -name "$(basename "$build_pattern")" -type d -printf '%T@ %p\n' | sort -nr | cut -d' ' -f2-))
    
    # Remove all but the last $keep_count builds
    if [[ ${#builds[@]} -gt $keep_count ]]; then
      echo "Pruning old builds (keeping last $keep_count)..."
      for ((i=keep_count; i<${#builds[@]}; i++)); do
        echo "  Removing: ${builds[i]}"
        rm -rf "${builds[i]}"
      done
    else
      echo "No pruning needed (${#builds[@]} builds <= $keep_count)"
    fi
  fi
}

# Helper: detect if we're on dev-01 (explicit signals only)
is_dev01() {
  # Authoritative marker file
  [[ -f /etc/dev-01.host ]] && { _DEV01_SRC="marker"; return 0; }

  # Optional environment fallback (set via systemd/env if we choose to)
  [[ "${DEV_INSTANCE:-}" == "dev-01" ]] && { _DEV01_SRC="env"; return 0; }

  # Disable hostname pattern to avoid false positives on other EC2 hosts
  # Enable only if explicitly allowed:
  if [[ "${ENABLE_HOSTNAME_FALLBACK:-0}" == "1" ]]; then
    local hn="$(hostname -f 2>/dev/null || hostname)"
    [[ "$hn" == *"compute.internal" ]] && { _DEV01_SRC="hostname"; return 0; }
  fi

  return 1
}
