#!/usr/bin/env bash
set -euo pipefail

: "${Qt6_DIR:?Qt6_DIR must be set (source .underlord/env.d/20-qt6.sh)}"
: "${CMAKE_PREFIX_PATH:?CMAKE_PREFIX_PATH must be set (source .underlord/env.d/20-qt6.sh)}"

work="${1:-/tmp/qt-smoke}"
rm -rf "$work"
mkdir -p "$work"

cat >"$work/CMakeLists.txt" <<'CMK'
cmake_minimum_required(VERSION 3.16)
project(QtSmoke LANGUAGES CXX)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
find_package(Threads REQUIRED)
find_package(Qt6 6.6 REQUIRED COMPONENTS Core)
message(STATUS "Qt6 Core found at ${Qt6_DIR}")
message(STATUS "Qt6 Core include dirs: ${Qt6Core_INCLUDE_DIRS}")
CMK

if ! cmake -S "$work" -B "$work/build" \
  -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON \
  -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}" \
  -DQt6_DIR="${Qt6_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -Wno-dev >"$work/configure.log" 2>&1; then
  echo "[smoke] ❌ Configure failed. See $work/configure.log"
  grep -E "Qt6 Core found|Found Qt6|Qt6Config\\.cmake|CMAKE_PREFIX_PATH|Qt6_DIR" -R "$work/build" "$work/configure.log" || true
  exit 1
fi

grep -E "Qt6 Core found|Found Qt6|Qt6Config\\.cmake" -R "$work/build" "$work/configure.log" || true
echo "[smoke] ✅ Qt6 configure OK"

rm -rf "$work"
exit 0

