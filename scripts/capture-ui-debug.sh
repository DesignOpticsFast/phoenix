#!/bin/bash
# Capture UI debug logs from Phoenix

TIMESTAMP=$(date +%Y%m%d_%H%M%S)
LOG_FILE="./debug-logs/ui/ui_debug_${TIMESTAMP}.log"
PHOENIX_APP="./build/debug/Phoenix.app/Contents/MacOS/Phoenix"

echo "=== Phoenix UI Debug Log Capture ==="
echo "Log file: $LOG_FILE"
echo ""
echo "Instructions:"
echo "1. Phoenix will launch with --debug-ui-log enabled"
echo "2. Open Analysis → XY Plot from the menu"
echo "3. Wait 5 seconds for logs to be captured"
echo "4. Close Phoenix (or press Ctrl+C here)"
echo ""

# Ensure log directory exists
mkdir -p ./debug-logs/ui

# Launch Phoenix with debug logging and capture output
echo "Launching Phoenix..."
$PHOENIX_APP --debug-ui-log 2>&1 | tee "$LOG_FILE"

echo ""
echo "=== Log Analysis ==="
echo "Log captured to: $LOG_FILE"
echo ""

# Extract key diagnostic sections
echo "=== PANEL CREATION ==="
grep -E "\[PANEL\]|\[XYWIN\].*Panel" "$LOG_FILE" || echo "No panel creation logs found"

echo ""
echo "=== FEATURE REGISTRY LOOKUP ==="
grep -E "\[REGISTRY\]" "$LOG_FILE" || echo "No registry lookup logs found"

echo ""
echo "=== WIDGET TREE ==="
grep -E "\[TREE" "$LOG_FILE" | head -50 || echo "No widget tree logs found"

echo ""
echo "=== VISIBILITY STATE ==="
grep -E "visible:|hidden:" "$LOG_FILE" | head -30 || echo "No visibility logs found"

echo ""
echo "=== SPLITTER STATE ==="
grep -E "SPLITTER|splitter" "$LOG_FILE" | head -20 || echo "No splitter logs found"

echo ""
echo "Full log available at: $LOG_FILE"

