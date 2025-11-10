#!/usr/bin/env zsh
set -euo pipefail

SOAK_BIN=${SOAK_BIN:-build/dev-01-relwithdebinfo/phoenix_app}
SOAK_APP_LOG=${SOAK_APP_LOG:-/tmp/phoenix_soak_app.log}
SOAK_CSV=${SOAK_CSV:-/tmp/phoenix_soak_memory.csv}
DURATION_MIN=${DURATION_MIN:-60}
SOAK_ARGS_DEFAULT=("--test-i18n" "--lang=en")

if ! [[ $DURATION_MIN == <-> ]]; then
  print -u2 "Error: DURATION_MIN must be an integer number of minutes"
  exit 2
fi

if [[ ! -x $SOAK_BIN ]]; then
  print -u2 "Error: SOAK_BIN '$SOAK_BIN' not found or not executable. Run 'make soak-build' or set SOAK_BIN to the Phoenix binary."
  exit 2
fi

if [[ -n ${SOAK_ARGS-} ]]; then
  SOAK_CMD=($SOAK_BIN ${=SOAK_ARGS})
else
  SOAK_CMD=($SOAK_BIN $SOAK_ARGS_DEFAULT)
fi

rm -f "$SOAK_APP_LOG" "$SOAK_CSV"
mkdir -p "${SOAK_APP_LOG:h}" "${SOAK_CSV:h}"
print "timestamp,elapsed_sec,rss_kb,rss_mb,threads,fds,handles" > "$SOAK_CSV"

duration_sec=$((DURATION_MIN * 60))
start_epoch=$(date -u +%s)
end_epoch=$((start_epoch + duration_sec))

sample() {
  if ! kill -0 $pid 2>/dev/null; then
    return
  fi
  local now_epoch rss_kb rss_mb threads timestamp elapsed
  now_epoch=$(date -u +%s)
  elapsed=$((now_epoch - start_epoch))
  timestamp=$(date -u +%Y-%m-%dT%H:%M:%SZ)
  rss_kb=$(ps -o rss= -p $pid 2>/dev/null | tr -d ' ')
  threads=$(ps -o thcount= -p $pid 2>/dev/null | tr -d ' ')
  if [[ -n $rss_kb ]]; then
    rss_mb=$(python3 -c 'import sys; value=float(sys.argv[1]); print(f"{value/1024:.3f}")' "$rss_kb")
  else
    rss_mb=""
  fi
  printf '%s,%s,%s,%s,%s,,\n' "$timestamp" "$elapsed" "$rss_kb" "$rss_mb" "${threads:-}" >> "$SOAK_CSV"
}

cleanup() {
  if kill -0 $pid 2>/dev/null; then
    kill $pid 2>/dev/null || true
    sleep 2
    kill -9 $pid 2>/dev/null || true
  fi
  wait $pid 2>/dev/null || true
}

trap cleanup EXIT

env QT_QPA_PLATFORM=${QT_QPA_PLATFORM:-offscreen} ${SOAK_CMD[@]} >"$SOAK_APP_LOG" 2>&1 &
pid=$!

sample
sleep_interval=5

while kill -0 $pid 2>/dev/null; do
  now=$(date -u +%s)
  if (( now >= end_epoch )); then
    print "Reached configured duration (${DURATION_MIN} min); terminating process." >> "$SOAK_APP_LOG"
    kill $pid 2>/dev/null || true
    break
  fi
  sleep $sleep_interval
  sample
done

wait $pid 2>/dev/null || true
sample
