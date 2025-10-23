#!/usr/bin/env bash
set -euo pipefail

echo "=== dev-01 Preflight Build ==="

host="$(hostname)"
if [[ "$host" != *"dev-01"* && "$host" != *"ip-10-0-8-68"* ]]; then
  echo "❌ Not on dev-01 (host=$host). Run this preflight on dev-01."
  exit 1
fi

: "${CMAKE_PREFIX_PATH:?CMAKE_PREFIX_PATH not set}"
qt_cfg="$CMAKE_PREFIX_PATH/lib/cmake/Qt6/Qt6Config.cmake"
if [[ ! -f "$qt_cfg" ]]; then
  echo "❌ Qt6Config.cmake missing at $qt_cfg"
  exit 1
fi

export QTFRAMEWORK_BYPASS_LICENSE_CHECK=1
echo "Host: $host"
echo "Qt OK at: $qt_cfg"
echo "Using CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH"

build_dir="build/dev-01-relwithdebinfo"
rm -rf "$build_dir"
cmake -S . -B "$build_dir" -G "Ninja" \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH"
cmake --build "$build_dir" -j "$(nproc 2>/dev/null || echo 4)"

if [[ -f "$build_dir/CTestTestfile.cmake" ]]; then
  ctest --test-dir "$build_dir" --output-on-failure
else
  echo "No tests detected; skipping."
fi

echo "✅ dev-01 preflight succeeded."
