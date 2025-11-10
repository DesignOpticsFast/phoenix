#!/usr/bin/env bash
set -euo pipefail

SOAK_BIN="${SOAK_BIN:-build/dev-01-relwithdebinfo/phoenix_app}"
SOAK_APP_LOG="${SOAK_APP_LOG:-/tmp/phoenix_soak_app.log}"
SOAK_CSV="${SOAK_CSV:-/tmp/phoenix_soak_memory.csv}"
DURATION_MIN="${DURATION_MIN:-60}"
SOAK_ARGS_DEFAULT=("--test-i18n" "--lang=en")

if ! [[ "$DURATION_MIN" =~ ^[0-9]+$ ]]; then
  echo "Error: DURATION_MIN must be an integer number of minutes" >&2
  exit 2
fi

if [[ ! -x "$SOAK_BIN" ]]; then
  echo "Error: SOAK_BIN '$SOAK_BIN' not found or not executable. Run 'make soak-build' or set SOAK_BIN to the Phoenix binary." >&2
  exit 2
fi

# Allow callers to override soak arguments; default to self-test to avoid heavy workloads.
if [[ -n "${SOAK_ARGS:-}" ]]; then
  # shellcheck disable=SC2206
  SOAK_CMD=("${SOAK_BIN}" ${SOAK_ARGS})
else
  SOAK_CMD=("${SOAK_BIN}" "${SOAK_ARGS_DEFAULT[@]}")
fi

rm -f "$SOAK_APP_LOG" "$SOAK_CSV"
mkdir -p "$(dirname "$SOAK_APP_LOG")" "$(dirname "$SOAK_CSV")"

echo "timestamp,elapsed_sec,rss_kb,rss_mb,threads,fds,handles" > "$SOAK_CSV"

duration_sec=$((DURATION_MIN * 60))
start_epoch=$(date -u +%s)
end_epoch=$((start_epoch + duration_sec))

sample() {
  local now_epoch rss_kb rss_mb threads fds timestamp elapsed
  if [[ ! -r "/proc/$pid/status" ]]; then
    return
  fi
  now_epoch=$(date -u +%s)
  elapsed=$((now_epoch - start_epoch))
  timestamp=$(date -u +%Y-%m-%dT%H:%M:%SZ)
  rss_kb=$(awk '/VmRSS:/{print $2; exit}' "/proc/$pid/status" 2>/dev/null || true)
  threads=$(awk '/Threads:/{print $2; exit}' "/proc/$pid/status" 2>/dev/null || true)
  fds=""
  if [[ -d "/proc/$pid/fd" ]]; then
    fds=$(ls -U "/proc/$pid/fd" 2>/dev/null | wc -l | tr -d ' ')
  fi
  if [[ -n "$rss_kb" ]]; then
    rss_mb=$(awk -v rss="$rss_kb" 'BEGIN{printf "%.3f", rss/1024}')
  else
    rss_kb=""
    rss_mb=""
  fi
  printf '%s,%s,%s,%s,%s,%s,%s\n' "$timestamp" "$elapsed" "$rss_kb" "$rss_mb" "${threads:-}" "${fds:-}" "" >> "$SOAK_CSV"
}

cleanup() {
  if kill -0 "$pid" 2>/dev/null; then
    kill "$pid" 2>/dev/null || true
    sleep 2
    kill -9 "$pid" 2>/dev/null || true
  fi
  wait "$pid" 2>/dev/null || true
}

trap cleanup EXIT

env QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-offscreen}" "${SOAK_CMD[@]}" >"$SOAK_APP_LOG" 2>&1 &
pid=$!

# Ensure we always capture one sample immediately.
sample

sleep_interval=5

while kill -0 "$pid" 2>/dev/null; do
  now=$(date -u +%s)
  if (( now >= end_epoch )); then
    echo "Reached configured duration (${DURATION_MIN} min); terminating process." >> "$SOAK_APP_LOG"
    kill "$pid" 2>/dev/null || true
    break
  fi
  sleep "$sleep_interval"
  sample
done

wait "$pid" 2>/dev/null || true

# Capture a final sample after exit to record last known value.
sample

