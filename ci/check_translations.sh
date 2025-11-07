#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build/dev-01-relwithdebinfo}"

cd "${ROOT_DIR}"

if [[ ! -d "${BUILD_DIR}" ]]; then
  echo "::group::Configure translations build directory"
  cmake -S . -B "${BUILD_DIR}" -G "Ninja"
  echo "::endgroup::"
fi

echo "::group::Update translation source catalogs (.ts)"
cmake --build "${BUILD_DIR}" --target translations-update
echo "::endgroup::"

echo "::group::Compile translation bundles (.qm)"
cmake --build "${BUILD_DIR}" --target translations
echo "::endgroup::"

if ! git diff --quiet -- translations/*.ts; then
  echo "❌ Uncommitted .ts changes detected after running translations-update."
  git --no-pager diff -- translations/*.ts
  exit 1
fi

if git grep -RIn "tr(.*icon" src; then
  echo "❌ Icon keys must not be wrapped in tr()."
  exit 1
fi

echo "✅ Translation drift and icon guard checks passed."
