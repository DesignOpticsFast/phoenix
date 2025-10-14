#!/bin/bash
# Close current sprint and archive

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📦 Closing Current Sprint"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo

# Check if current sprint exists
if [ ! -d .underlord/sprints/current ] || [ ! -f .underlord/sprints/current/goals.md ]; then
    echo "❌ No active sprint found in .underlord/sprints/current/"
    exit 1
fi

# Extract sprint number from goals
SPRINT_NUM=$(grep "^# Sprint" .underlord/sprints/current/goals.md | sed 's/# Sprint \([0-9]*\):.*/\1/' | head -1)
SPRINT_TITLE=$(grep "^# Sprint" .underlord/sprints/current/goals.md | sed 's/# Sprint [0-9]*: \(.*\)/\1/' | head -1)

if [ -z "$SPRINT_NUM" ]; then
    echo "⚠️  Could not determine sprint number from goals.md"
    read -p "Enter sprint number: " SPRINT_NUM
fi

ARCHIVE_DIR=".underlord/sprints/archive/sprint-${SPRINT_NUM}"
CURRENT_BRANCH=$(git branch --show-current)

echo "Sprint: #$SPRINT_NUM - $SPRINT_TITLE"
echo "Branch: $CURRENT_BRANCH"
echo

# Archive the sprint
echo "📦 Archiving sprint to $ARCHIVE_DIR..."
mkdir -p "$ARCHIVE_DIR"
cp .underlord/sprints/current/*.md "$ARCHIVE_DIR/" 2>/dev/null || true

# Update sprint history
echo "📚 Updating SPRINT_HISTORY.md..."
{
    echo ""
    echo "## Sprint $SPRINT_NUM: $SPRINT_TITLE"
    echo "**Date:** $(date +%Y-%m-%d)"
    echo "**Branch:** \`$CURRENT_BRANCH\`"
    echo "**Status:** Complete"
    echo ""
    echo "[Full Report]($ARCHIVE_DIR/report.md)"
    echo ""
} >> .underlord/docs/SPRINT_HISTORY.md

# Clean up current sprint
echo "🧹 Cleaning up current sprint workspace..."
rm -rf .underlord/sprints/current

# Return to main
echo "🔄 Returning to main branch..."
git checkout main
git pull --ff-only

echo
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ Sprint $SPRINT_NUM closed successfully"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo
echo "📁 Archived to: $ARCHIVE_DIR"
echo "📚 History updated: .underlord/docs/SPRINT_HISTORY.md"
echo
echo "Ready to start next sprint with:"
echo "  .underlord/sprint-init.sh <number> <slug>"
