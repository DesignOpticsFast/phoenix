#!/bin/bash
# GUI Smoke Test Script for Phoenix
# Semi-automated: Launches Phoenix, checks for crashes, then prompts for manual verification

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PHOENIX_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_TYPE="${1:-debug}"
PHOENIX_BIN="$PHOENIX_DIR/build/$BUILD_TYPE/Phoenix.app/Contents/MacOS/Phoenix"
LOG_FILE="$PHOENIX_DIR/build/$BUILD_TYPE/smoke-test.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "Phoenix GUI Smoke Test"
echo "=========================================="
echo "Build type: $BUILD_TYPE"
echo "Binary: $PHOENIX_BIN"
echo ""

# Check if binary exists
if [ ! -f "$PHOENIX_BIN" ]; then
    echo -e "${RED}ERROR: Phoenix binary not found at $PHOENIX_BIN${NC}"
    echo "Please build Phoenix first:"
    echo "  cd $PHOENIX_DIR"
    echo "  source ~/workspace/scripts/setup_qt_env.sh"
    echo "  cmake --build build/$BUILD_TYPE"
    exit 1
fi

# Setup Qt environment
if [ -f "$HOME/workspace/scripts/setup_qt_env.sh" ]; then
    source "$HOME/workspace/scripts/setup_qt_env.sh"
fi

echo "Launching Phoenix..."
echo "Log file: $LOG_FILE"
echo ""

# Launch Phoenix in background and capture PID
"$PHOENIX_BIN" > "$LOG_FILE" 2>&1 &
PHOENIX_PID=$!

echo "Phoenix launched (PID: $PHOENIX_PID)"
echo "Waiting 5 seconds for startup..."
sleep 5

# Check if process is still running (indicates no immediate crash)
if ps -p $PHOENIX_PID > /dev/null; then
    echo -e "${GREEN}✓ Phoenix is running (no immediate crash)${NC}"
    CRASHED=0
else
    echo -e "${RED}✗ Phoenix process terminated (likely crashed)${NC}"
    CRASHED=1
fi

# Check log for error messages
if [ -f "$LOG_FILE" ]; then
    ERROR_COUNT=$(grep -i "error\|fatal\|crash\|exception" "$LOG_FILE" | wc -l | tr -d ' ')
    if [ "$ERROR_COUNT" -gt 0 ]; then
        echo -e "${YELLOW}⚠ Found $ERROR_COUNT potential error messages in log${NC}"
        echo "Last 10 lines of log:"
        tail -10 "$LOG_FILE"
    fi
fi

echo ""
echo "=========================================="
echo "Manual Verification Checklist"
echo "=========================================="
echo "Please verify the following items manually:"
echo ""
echo "Main Window:"
echo "  [ ] Phoenix launches without crashes"
echo "  [ ] Main window displays correctly"
echo "  [ ] Toolbars are visible and functional"
echo "  [ ] Menus are accessible (File, Editors, Analysis, Tools, View, Help)"
echo "  [ ] Dock widgets are visible (if applicable)"
echo ""
echo "Analysis Windows:"
echo "  [ ] XY Plot analysis window opens from menu (Analysis → XY Plot)"
echo "  [ ] Analysis window displays graph correctly (if applicable)"
echo "  [ ] Analysis window has expected UI elements"
echo ""
echo "Window Management:"
echo "  [ ] Multiple analysis windows can be opened"
echo "  [ ] Windows can be closed individually"
echo "  [ ] Closing main window closes all analysis windows"
echo ""
echo "Preferences & Settings:"
echo "  [ ] Preferences dialog opens (File → Preferences)"
echo "  [ ] Theme switching works (View → Theme)"
echo ""
echo "Application Exit:"
echo "  [ ] Application quits cleanly (File → Exit or Cmd+Q)"
echo ""
echo "=========================================="
echo ""

if [ $CRASHED -eq 1 ]; then
    echo -e "${RED}TEST FAILED: Phoenix crashed during startup${NC}"
    echo "Check log file: $LOG_FILE"
    exit 1
fi

echo "Waiting for manual verification..."
echo "Press Enter when you've completed the manual checklist..."
read -r

# Ask for overall result
echo ""
echo "Did all manual checks pass? (y/n)"
read -r MANUAL_RESULT

if [ "$MANUAL_RESULT" = "y" ] || [ "$MANUAL_RESULT" = "Y" ]; then
    echo -e "${GREEN}✓ All smoke tests passed${NC}"
    
    # Clean up: kill Phoenix if still running
    if ps -p $PHOENIX_PID > /dev/null; then
        echo "Terminating Phoenix (PID: $PHOENIX_PID)..."
        kill $PHOENIX_PID 2>/dev/null || true
        sleep 1
        # Force kill if still running
        if ps -p $PHOENIX_PID > /dev/null; then
            kill -9 $PHOENIX_PID 2>/dev/null || true
        fi
    fi
    
    exit 0
else
    echo -e "${RED}✗ Some manual checks failed${NC}"
    echo "Please review the checklist and fix issues before proceeding."
    
    # Clean up: kill Phoenix if still running
    if ps -p $PHOENIX_PID > /dev/null; then
        echo "Terminating Phoenix (PID: $PHOENIX_PID)..."
        kill $PHOENIX_PID 2>/dev/null || true
    fi
    
    exit 1
fi

