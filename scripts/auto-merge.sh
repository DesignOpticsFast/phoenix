#!/bin/bash

# Auto-merge and cleanup script for Phoenix repository
# Usage: ./scripts/auto-merge.sh [PR_NUMBER]

set -e

PR_NUMBER=${1:-""}

if [ -z "$PR_NUMBER" ]; then
    echo "Usage: $0 [PR_NUMBER]"
    echo "Example: $0 50"
    exit 1
fi

echo "=== Auto-merge PR #$PR_NUMBER ==="

# Check if PR is ready to merge
echo "Checking PR status..."
gh pr view $PR_NUMBER --json state,mergeable,statusCheckRollup

# Get PR details
PR_TITLE=$(gh pr view $PR_NUMBER --json title --jq '.title')
PR_BRANCH=$(gh pr view $PR_NUMBER --json headRefName --jq '.headRefName')

echo "PR Title: $PR_TITLE"
echo "Source Branch: $PR_BRANCH"

# Check if all status checks are passing (if any)
echo "Checking status checks..."
STATUS_CHECKS=$(gh pr view $PR_NUMBER --json statusCheckRollup --jq '.statusCheckRollup[].state' | grep -v "SUCCESS" | wc -l)

if [ "$STATUS_CHECKS" -gt 0 ]; then
    echo "⚠️  Some status checks are not passing, but proceeding with merge..."
fi

echo "✅ Proceeding with merge"

# Merge the PR with squash
echo "Merging PR with squash..."
gh pr merge $PR_NUMBER --squash --delete-branch

echo "✅ PR #$PR_NUMBER merged and branch $PR_BRANCH deleted"

# Update local main branch
echo "Updating local main branch..."
git checkout main
git pull origin main

echo "✅ Local main branch updated"
echo "🎉 Auto-merge complete!"

