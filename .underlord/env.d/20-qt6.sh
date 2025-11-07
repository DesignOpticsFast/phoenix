#!/usr/bin/env bash
set -euo pipefail

QT_PREFIX="/opt/Qt/6.10.0/gcc_64"
QT_CMAKE_DIR="${QT_PREFIX}/lib/cmake/Qt6"

if [ ! -f "${QT_CMAKE_DIR}/Qt6Config.cmake" ]; then
  echo "[qt6] Qt6Config.cmake not found at ${QT_CMAKE_DIR}" >&2
  exit 1
fi

# Optional preserved prefixes (leave empty unless needed)
KEPT_PREFIXES=(
  # "/usr/local"
  # "/opt/vcpkg/installed/x64-linux"
  # "/opt/phoenix/third_party"
)

join_colon() {
  local IFS=:
  printf '%s' "$*"
}

raw_path="$(join_colon "$QT_PREFIX" "${KEPT_PREFIXES[@]}" "${CMAKE_PREFIX_PATH:-}")"

dedup_path() {
  awk -v RS=: -v ORS=: 'length($0)>0 && !seen[$0]++' <<<"$1" | sed 's/:$//'
}

if [[ "${CMAKE_PREFIX_PATH:-}" == *"/opt/Qt/6.9."* ]]; then
  echo "[qt6] WARNING: Found Qt 6.9.x segments in CMAKE_PREFIX_PATH; Qt 6.10.0 will be prepended and duplicates removed."
fi

export CMAKE_PREFIX_PATH="$(dedup_path "$raw_path")"
export Qt6_DIR="${QT_CMAKE_DIR}"

echo "[qt6] Qt6_DIR=${Qt6_DIR}"
echo "[qt6] CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"

