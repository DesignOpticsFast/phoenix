#!/bin/bash
# UnderLord Morning Startup Routine
# Prepares environment for daily development session

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🌅 UnderLord Morning Routine"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo

# Step 1: Pre-flight check
echo "1️⃣  Running pre-flight checks..."
if ! .underlord/preflight.sh; then
    echo
    echo "❌ Pre-flight failed. Resolve issues before continuing."
    exit 1
fi

echo
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo

# Step 2: Sprint context
echo "2️⃣  Current Sprint Status"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ -f .underlord/sprints/current/goals.md ]; then
    echo "📋 Active Sprint:"
    head -n 3 .underlord/sprints/current/goals.md | sed 's/^/  /'
    echo
    
    echo "📍 Sprint Progress:"
    CURRENT_BRANCH=$(git branch --show-current)
    echo "  Branch: $CURRENT_BRANCH"
    
    LAST_COMMIT=$(git log -1 --format="%h - %s" 2>/dev/null || echo "No commits yet")
    echo "  Last commit: $LAST_COMMIT"
    
    # Check for open PRs
    PROJECT=$(grep -o '"project": "[^"]*"' .underlord/config.json | cut -d'"' -f4)
    PR_COUNT=$(gh pr list -R "DesignOpticsFast/$PROJECT" --head "$CURRENT_BRANCH" --json number --jq '. | length' 2>/dev/null || echo "0")
    
    if [ "$PR_COUNT" -gt 0 ]; then
        echo "  Open PRs: $PR_COUNT"
        gh pr list -R "DesignOpticsFast/$PROJECT" --head "$CURRENT_BRANCH" --json number,title,url --jq '.[] | "    #\(.number): \(.title)\n    \(.url)"'
    else
        echo "  Open PRs: None"
    fi
else
    echo "⚠️  No active sprint found in .underlord/sprints/current/"
    echo
    echo "To start a new sprint:"
    echo "  .underlord/sprint-init.sh <number> <slug>"
    echo
    echo "Example:"
    echo "  .underlord/sprint-init.sh 002 lens-calculator"
    exit 0
fi

echo
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo

# Step 3: Show next actions
echo "3️⃣  Next Actions"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📖 Review sprint documents:"
echo "  • goals.md    - What we're building"
echo "  • plan.md     - Step-by-step execution"
echo "  • context.md  - Background & constraints"
echo
echo "🚀 To begin development:"
echo "  1. Open Cursor in this directory"
echo "  2. Open .underlord/sprints/current/plan.md"
echo "  3. Enable Cursor Plan Mode"
echo "  4. Review and approve each phase"
echo
echo "💡 Useful commands:"
echo "  • .underlord/sprint-status.sh  - Check progress"
echo "  • .underlord/preflight.sh      - Re-run checks"
echo "  • gh pr checks --watch         - Monitor CI"
echo

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ Startup complete - Ready to code!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
