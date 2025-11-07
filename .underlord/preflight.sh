#!/bin/bash
# UnderLord Pre-Flight Check
# Run before any coding session to validate environment

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🚀 UnderLord Pre-Flight Check"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

cmake_path="$(command -v cmake 2>/dev/null || true)"
[ -n "$cmake_path" ] || cmake_path="<missing>"
cmake_version="$(cmake --version 2>/dev/null | head -1 || true)"
[ -n "$cmake_version" ] || cmake_version="<unknown>"

echo "[preflight] cmake=$cmake_path ; version=$cmake_version"
echo "[preflight] Qt6_DIR=${Qt6_DIR:-<unset>}"
echo "[preflight] CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH:-<unset>}"
echo

# Check Cursor autonomy mode
echo "Checking Cursor autonomy mode..."
AUTON_OK=1
CURSOR_SETTINGS="$HOME/.config/Cursor/User/settings.json"

if [ ! -f "$CURSOR_SETTINGS" ]; then
    echo "⚠️  Cursor settings file not found at $CURSOR_SETTINGS"
    echo "   Assuming autonomy mode configured correctly"
else
    grep -q '"cursor.ai.autoApprove": true' "$CURSOR_SETTINGS" || AUTON_OK=0
    grep -q '"cursor.ai.batchMode": true' "$CURSOR_SETTINGS" || AUTON_OK=0
    grep -q '"cursor.terminal.approval": "batch"' "$CURSOR_SETTINGS" || AUTON_OK=0
    
    if [ $AUTON_OK -ne 1 ]; then
        echo "❌ Cursor autonomy mode not configured"
        echo "   Fix settings, restart Cursor, then re-run preflight"
        exit 1
    fi
fi
echo "✅ Cursor autonomy mode OK"
echo

FAILURES=0

# Function to check command
check_cmd() {
    if command -v "$1" >/dev/null 2>&1; then
        echo -e "${GREEN}✅${NC} $1: $(command -v $1)"
        return 0
    else
        echo -e "${RED}❌${NC} $1: NOT FOUND"
        ((FAILURES++))
        return 1
    fi
}

# Function to check optional command
check_optional() {
    if command -v "$1" >/dev/null 2>&1; then
        echo -e "${GREEN}✅${NC} $1: $(command -v $1) (optional)"
    else
        echo -e "${YELLOW}⚠️${NC}  $1: not installed (optional but recommended)"
    fi
}

echo "1️⃣  Environment Validation"
echo "━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Host: $(hostname)"
echo "User: $(whoami)"
echo "OS: $(uname -sr)"
echo "Working directory: $REPO_ROOT"
echo

echo "[preflight] Qt smoke check…"
if scripts/dev/qt6_smoke.sh /tmp/qt-smoke; then
    echo "[preflight] Qt smoke: OK"
else
    echo "[preflight] Qt smoke: FAILED (continuing); inspect /tmp/qt-smoke/configure.log if preserved"
fi
echo

echo "2️⃣  Build Toolchain"
echo "━━━━━━━━━━━━━━━━━━━━━━━━"
check_cmd cmake
check_cmd g++
check_optional ninja
check_optional ccache
echo

echo "3️⃣  Version Control"
echo "━━━━━━━━━━━━━━━━━━━━━━━━"
check_cmd git
check_cmd gh

echo
echo "Git status:"
git fetch --all --prune 2>&1 | sed 's/^/  /'

if [ -n "$(git status --porcelain)" ]; then
    echo -e "${YELLOW}⚠️${NC}  Uncommitted changes detected:"
    git status --short | sed 's/^/  /'
else
    echo -e "${GREEN}✅${NC} Working directory clean"
fi

CURRENT_BRANCH=$(git branch --show-current)
echo "Current branch: $CURRENT_BRANCH"
echo

echo "4️⃣  GitHub Authentication"
echo "━━━━━━━━━━━━━━━━━━━━━━━━"
if gh auth status >/dev/null 2>&1; then
    echo -e "${GREEN}✅${NC} GitHub CLI authenticated"
    gh auth status 2>&1 | grep "Logged in" | sed 's/^/  /'
else
    echo -e "${RED}❌${NC} GitHub CLI not authenticated"
    echo "  Run: gh auth login"
    ((FAILURES++))
fi
echo

echo "5️⃣  CodeQL Workflow"
echo "━━━━━━━━━━━━━━━━━━━━━━━━"
if [ -f .github/workflows/codeql.yml ]; then
    echo -e "${GREEN}✅${NC} CodeQL workflow present"
else
    echo -e "${YELLOW}⚠️${NC}  CodeQL workflow not found"
fi
echo

echo "6️⃣  Build Smoke Test"
echo "━━━━━━━━━━━━━━━━━━━━━━━━"
BUILD_DIR="build-preflight-$$"

# Check if we should run build test
if [ "${SKIP_BUILD:-0}" = "1" ]; then
    echo -e "${YELLOW}⚠️${NC}  Build test skipped (SKIP_BUILD=1)"
else
    echo "Creating test build in $BUILD_DIR..."
    
    GEN="cmake"
    if command -v qt-cmake >/dev/null 2>&1; then
        GEN="qt-cmake"
    fi

    mkdir -p "$BUILD_DIR"

    cmake_args=(-S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release)
    [ -n "${CMAKE_PREFIX_PATH:-}" ] && cmake_args+=("-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")
    [ -n "${Qt6_DIR:-}" ] && cmake_args+=("-DQt6_DIR=${Qt6_DIR}")

    if "$GEN" "${cmake_args[@]}"; then
        echo -e "${GREEN}✅${NC} CMake configuration successful"
    else
        echo "[preflight] Configure failed; retrying with CMAKE_FIND_DEBUG_MODE=ON..."
        rm -f /tmp/preflight.log
        if "$GEN" "${cmake_args[@]}" -DCMAKE_FIND_DEBUG_MODE=ON |& tee /tmp/preflight.log; then
            echo -e "${GREEN}✅${NC} CMake configuration successful (with debug)"
        else
            echo "[preflight] Configure still failing. See /tmp/preflight.log for search paths."
            ((FAILURES++))
            rm -rf "$BUILD_DIR"
            exit 1
        fi
    fi

    grep -E "Qt6Config\.cmake|Found Qt6|Qt6 version" -R --include='CMake*' "$BUILD_DIR" 2>/dev/null || true

    echo "Building..."
    if cmake --build "$BUILD_DIR" --target all -j >/dev/null 2>&1; then
        echo -e "${GREEN}✅${NC} Build successful"
    else
        echo -e "${RED}❌${NC} Build failed"
        ((FAILURES++))
        rm -rf "$BUILD_DIR"
        exit 1
    fi
    
    echo "Running tests..."
    if ctest --test-dir "$BUILD_DIR" --output-on-failure >/dev/null 2>&1; then
        echo -e "${GREEN}✅${NC} Tests passed"
    else
        echo -e "${YELLOW}⚠️${NC}  Some tests failed (review output above)"
    fi
    
    rm -rf "$BUILD_DIR"
fi
echo

echo "7️⃣  UnderLord Configuration"
echo "━━━━━━━━━━━━━━━━━━━━━━━━"
if [ -f .underlord/config.json ]; then
    echo -e "${GREEN}✅${NC} UnderLord config present"
    echo "  Project: $(grep -o '"project": "[^"]*"' .underlord/config.json | cut -d'"' -f4)"
    echo "  Autonomy: $(grep -o '"autonomy_level": "[^"]*"' .underlord/config.json | cut -d'"' -f4)"
else
    echo -e "${RED}❌${NC} UnderLord config missing"
    ((FAILURES++))
fi
echo

# Save status
echo "$(date -Iseconds)" > .underlord/last-preflight.txt
echo "status=$([[ $FAILURES -eq 0 ]] && echo 'pass' || echo 'fail')" >> .underlord/last-preflight.txt
echo "failures=$FAILURES" >> .underlord/last-preflight.txt

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [ $FAILURES -eq 0 ]; then
    echo -e "${GREEN}✅ Pre-flight complete - All systems operational${NC}"
    echo "🎯 Ready for UnderLord operations"
    exit 0
else
    echo -e "${RED}❌ Pre-flight failed with $FAILURES issue(s)${NC}"
    echo "🔧 Resolve issues above before proceeding"
    exit 1
fi

section "5️⃣.5  OCCT/TBB Version Assertions"
# Expect OCCT >= 7.9.1 and TBB >= 2021.5
req_occt="7.9.1"
req_tbb="2021.5"

occt_cfg="${OpenCASCADE_DIR:-/opt/occt-7.9.1/lib/cmake/opencascade}/OpenCASCADEConfig.cmake"
if [ -f "$occt_cfg" ]; then
  occt_ver="$(grep -m1 -E 'set\(OpenCASCADE_VERSION' "$occt_cfg" 2>/dev/null | sed 's/[^0-9.]*//g' || true)"
  [ -z "$occt_ver" ] && occt_ver="$(grep -m1 -E 'set\(Foundation_VERSION' "$occt_cfg" 2>/dev/null | sed 's/[^0-9.]*//g' || true)"
else
  occt_ver=""
fi

tbb_so="$(ls ${TBB_ROOT_DIR:-/opt/tbb-2021.5}/lib64/libtbb.so* 2>/dev/null | head -n1)"
if [ -n "$tbb_so" ]; then
  tbb_ver="$(strings "$tbb_so" 2>/dev/null | grep -m1 -E '^TBB_version' | sed 's/[^0-9.]*//g')"
else
  tbb_ver=""
fi

ver_ge() { [ "$(printf '%s\n' "$2" "$1" | sort -V | head -n1)" = "$2" ]; }

echo "Detected: OCCT=${occt_ver:-NONE} TBB=${tbb_ver:-NONE} (required: OCCT>=$req_occt TBB>=$req_tbb)"
if [ -z "$occt_ver" ] || ! ver_ge "$occt_ver" "$req_occt"; then 
  echo "❌ OpenCASCADE $req_occt or higher required"
  exit 1
fi
if [ -z "$tbb_ver" ] || ! ver_ge "$tbb_ver" "$req_tbb"; then 
  echo "❌ TBB $req_tbb or higher required"
  exit 1
fi
echo "✅ OCCT/TBB versions OK"

