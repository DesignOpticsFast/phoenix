#!/bin/bash
# Initialize a new sprint from templates

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

# Check arguments
if [ $# -ne 2 ]; then
    echo "Usage: $0 <sprint-number> <slug>"
    echo
    echo "Example: $0 002 lens-calculator"
    echo "  Creates: sprint-002-lens-calculator"
    exit 1
fi

SPRINT_NUM="$1"
SLUG="$2"
BRANCH_NAME="sprint-${SPRINT_NUM}-${SLUG}"

echo "🚀 Initializing Sprint $SPRINT_NUM: $SLUG"
echo

# Check if current sprint exists
if [ -d .underlord/sprints/current ] && [ -f .underlord/sprints/current/goals.md ]; then
    echo "⚠️  Active sprint detected in .underlord/sprints/current/"
    echo
    echo "Please close current sprint first:"
    echo "  .underlord/sprint-close.sh"
    echo
    exit 1
fi

# Ensure we're on main
CURRENT_BRANCH=$(git branch --show-current)
if [ "$CURRENT_BRANCH" != "main" ]; then
    echo "⚠️  Not on main branch (currently on: $CURRENT_BRANCH)"
    echo
    read -p "Switch to main? [y/N] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        git checkout main
        git pull --ff-only
    else
        echo "Aborted."
        exit 1
    fi
fi

# Create branch
echo "📝 Creating branch: $BRANCH_NAME"
git checkout -b "$BRANCH_NAME"

# Create current sprint directory
echo "📁 Setting up sprint workspace..."
mkdir -p .underlord/sprints/current

# Copy templates
cp .underlord/sprints/templates/goals.template.md .underlord/sprints/current/goals.md
cp .underlord/sprints/templates/plan.template.md .underlord/sprints/current/plan.md
cp .underlord/sprints/templates/context.template.md .underlord/sprints/current/context.md

# Replace placeholders (cross-platform sed)
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    sed -i '' "s/{NUMBER}/$SPRINT_NUM/g" .underlord/sprints/current/*.md
    sed -i '' "s/{SLUG}/$SLUG/g" .underlord/sprints/current/*.md
    sed -i '' "s/{TITLE}/$(echo $SLUG | tr '-' ' ')/g" .underlord/sprints/current/*.md
else
    # Linux
    sed -i "s/{NUMBER}/$SPRINT_NUM/g" .underlord/sprints/current/*.md
    sed -i "s/{SLUG}/$SLUG/g" .underlord/sprints/current/*.md
    sed -i "s/{TITLE}/$(echo $SLUG | tr '-' ' ')/g" .underlord/sprints/current/*.md
fi

echo "✅ Sprint workspace created at .underlord/sprints/current/"
echo
echo "📋 Next steps:"
echo "  1. Edit .underlord/sprints/current/goals.md"
echo "     - Define objectives and success criteria"
echo
echo "  2. Edit .underlord/sprints/current/plan.md"
echo "     - Break down into executable phases"
echo "     - Add breakpoints for human review"
echo
echo "  3. Edit .underlord/sprints/current/context.md"
echo "     - Document architectural decisions"
echo "     - Note constraints and dependencies"
echo
echo "  4. Run .underlord/startup.sh to begin"
echo
echo "🎯 Sprint $SPRINT_NUM initialized on branch: $BRANCH_NAME"
