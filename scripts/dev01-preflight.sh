#!/usr/bin/env bash
set -euo pipefail

# Thin wrapper that sources the shared preflight library
ROOT="$(git rev-parse --show-toplevel)"
# shellcheck disable=SC1091
source "$ROOT/scripts/lib/preflight_common.sh"

echo "=== dev-01 Preflight Build ==="
preflight_configure_build "build/dev-01-relwithdebinfo" "${1:-false}"
echo "✅ dev-01 preflight succeeded."