#!/usr/bin/env bash

set -euo pipefail

# shellcheck disable=SC1091
source "$(dirname "$0")/preflight_common.sh"

fail() { echo "FAIL: $*"; exit 1; }
pass() { echo "PASS: $*"; }

test_sanitize_path_var() {
  local in out want
  in=':/opt/Qt/6.10.0/gcc_64::/usr/local/lib:
'
  want='/opt/Qt/6.10.0/gcc_64:/usr/local/lib'
  out="$(sanitize_path_var "$in")"
  [[ "$out" == "$want" ]] || fail "expected [$want], got [$out]"
  pass "sanitize_path_var normalized path"
}

test_empty() {
  local out
  out="$(sanitize_path_var '')"
  [[ -z "$out" ]] || fail "expected empty output, got [$out]"
  pass "sanitize_path_var empty input"
}

test_sanitize_path_var
test_empty

