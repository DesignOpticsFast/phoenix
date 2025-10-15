#!/bin/bash
# Display current sprint status and progress

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 Sprint Status Report"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo

# Check if current sprint exists
if [ ! -d .underlord/sprints/current ] || [ ! -f .underlord/sprints/current/goals.md ]; then
    echo "❌ No active sprint"
    echo
    echo "Start a new sprint with:"
    echo "  .underlord/sprint-init.sh <number> <slug>"
    exit 0
fi

# Extract sprint info
SPRINT_TITLE=$(grep "^# Sprint" .underlord/sprints/current/goals.md | head -1 | sed 's/^# //')
CURRENT_BRANCH=$(git branch --show-current)
PROJECT=$(grep -o '"project": "[^"]*"' .underlord/config.json | cut -d'"' -f4)

echo "📋 Sprint: $SPRINT_TITLE"
echo "📍 Branch: $CURRENT_BRANCH"
echo "🏗️  Project: $PROJECT"
echo

# Git status
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🔧 Working Directory"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ -n "$(git status --porcelain)" ]; then
    echo "Modified files:"
    git status --short | sed 's/^/  /'
else
    echo "✅ Working directory clean"
fi

COMMITS_AHEAD=$(git rev-list --count main..HEAD 2>/dev/null || echo "0")
echo
echo "Commits ahead of main: $COMMITS_AHEAD"

if [ "$COMMITS_AHEAD" -gt 0 ]; then
    echo "Recent commits:"
    git log --oneline main..HEAD | head -5 | sed 's/^/  /'
fi

echo

# PR status
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📬 Pull Requests"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

PR_COUNT=$(gh pr list -R "DesignOpticsFast/$PROJECT" --search "head:$CURRENT_BRANCH" --json number --jq '. | length' 2>/dev/null || echo "0")

if [ "$PR_COUNT" -eq 0 ]; then
    echo "No open PRs for this branch"
    
    if [ "$COMMITS_AHEAD" -gt 0 ]; then
        echo
        echo "💡 Ready to create PR:"
        echo "   gh pr create --title \"Sprint X: Description\" --body \"...\""
    fi
else
    gh pr list -R "DesignOpticsFast/$PROJECT" --search "head:$CURRENT_BRANCH" --json number,title,url,state,isDraft --jq '.[] | "  #\(.number): \(.title)\n  State: \(.state)\(.isDraft and " (Draft)" or "")\n  URL: \(.url)\n"'
fi

echo
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "💡 Next Actions"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ "$COMMITS_AHEAD" -eq 0 ]; then
    echo "  • No commits yet - start implementing from plan.md"
elif [ "$PR_COUNT" -eq 0 ]; then
    echo "  • Create PR: gh pr create"
else
    echo "  • Watch CI: gh pr checks --watch"
    echo "  • Review PR and request approval"
fi

echo
