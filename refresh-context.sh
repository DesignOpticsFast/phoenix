#!/usr/bin/env bash
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
# shellcheck disable=SC1091
source "$ROOT/scripts/lib/preflight_common.sh"

echo "Refreshing AI context…"
print_truth_lines

# Minimal environment/runner info (no builds)
echo "Host: $(hostname)"
echo "Repo: $ROOT"
git -C "$ROOT" rev-parse --abbrev-ref HEAD
git -C "$ROOT" log -1 --pretty='%h %s' || true
echo "CI policy: Required check = gate; CodeQL runs on main only"
echo "Note: refresh-context is informational only; no builds are performed."
