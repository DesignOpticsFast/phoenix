#!/usr/bin/env bash
# NOTE: This script performs a full clean build and test suite.
# It is intended for explicit preflight checks, not the daily ritual.
# The daily ritual (make daily) does NOT invoke this script.
# Run this explicitly when you need a full build+test validation.
set -euo pipefail
ROOT="$(git rev-parse --show-toplevel)"
# shellcheck disable=SC1091
source "$ROOT/scripts/lib/preflight_common.sh"

echo "=== dev-01 Preflight Build ==="
preflight_configure_build "build/dev-01-relwithdebinfo"
echo "✅ Preflight build passed"