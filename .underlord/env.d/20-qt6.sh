#!/usr/bin/env bash
set -euo pipefail

# Detect platform and set Qt path accordingly
if [[ "$(uname -s)" == "Darwin" ]]; then
  # macOS: Try multiple possible Qt locations
  if [ -f "/Users/underlord/Qt/6.10.1/macos/lib/cmake/Qt6/Qt6Config.cmake" ]; then
    QT_PREFIX="/Users/underlord/Qt/6.10.1/macos"
  elif [ -f "/Users/mark/Qt/6.10.0/macos/lib/cmake/Qt6/Qt6Config.cmake" ]; then
    QT_PREFIX="/Users/mark/Qt/6.10.0/macos"
  else
    echo "[qt6] ERROR: Qt6Config.cmake not found in expected macOS locations" >&2
    echo "[qt6] Checked: /Users/underlord/Qt/6.10.1/macos" >&2
    echo "[qt6] Checked: /Users/mark/Qt/6.10.0/macos" >&2
    exit 1
  fi
  QT_CMAKE_DIR="${QT_PREFIX}/lib/cmake/Qt6"
else
  # Linux: Qt at /opt/Qt/6.10.0/gcc_64
  QT_PREFIX="/opt/Qt/6.10.0/gcc_64"
  QT_CMAKE_DIR="${QT_PREFIX}/lib/cmake/Qt6"
fi

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

if [ ${#KEPT_PREFIXES[@]} -gt 0 ]; then
    raw_path="$(join_colon "$QT_PREFIX" "${KEPT_PREFIXES[@]}" "${CMAKE_PREFIX_PATH:-}")"
else
    raw_path="$(join_colon "$QT_PREFIX" "${CMAKE_PREFIX_PATH:-}")"
fi

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
