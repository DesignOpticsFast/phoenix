#!/bin/bash

# Document Review and Cleanup Script
# Usage: ./scripts/doc-review.sh [repository]
# Example: ./scripts/doc-review.sh phoenix
# Example: ./scripts/doc-review.sh bedrock
# Example: ./scripts/doc-review.sh both

set -e

REPO=${1:-"both"}

echo "=== UnderLord Document Review and Cleanup ==="
echo "Repository: $REPO"
echo "Date: $(date)"
echo ""

case $REPO in
    "phoenix")
        echo "🔍 Reviewing Phoenix documentation for internal and cross-consistency..."
        echo "📋 Tasks to perform:"
        echo "  - Check for outdated references (Qt 6.9, Qt Charts, LocalSocket)"
        echo "  - Verify all links and relative paths"
        echo "  - Ensure consistency with Phase 0.5 Gate PASS results"
        echo "  - Archive any superseded materials"
        echo "  - Update cross-references between documents"
        echo "  - Validate Markdown formatting"
        echo ""
        echo "🚀 Ready to begin Phoenix document review"
        ;;
    "bedrock")
        echo "🔍 Reviewing Bedrock documentation for internal and cross-consistency..."
        echo "📋 Tasks to perform:"
        echo "  - Check for outdated references (Qt 6.9, Qt Charts, LocalSocket)"
        echo "  - Verify all links and relative paths"
        echo "  - Ensure consistency with Phase 0.5 Gate PASS results"
        echo "  - Archive any superseded materials"
        echo "  - Update cross-references between documents"
        echo "  - Validate Markdown formatting"
        echo "  - Check Palantir implementation status"
        echo ""
        echo "🚀 Ready to begin Bedrock document review"
        ;;
    "both")
        echo "🔍 Reviewing both Phoenix and Bedrock documentation for internal and cross-consistency..."
        echo "📋 Tasks to perform:"
        echo "  - Check for outdated references (Qt 6.9, Qt Charts, LocalSocket)"
        echo "  - Verify all links and relative paths"
        echo "  - Ensure consistency with Phase 0.5 Gate PASS results"
        echo "  - Archive any superseded materials"
        echo "  - Update cross-references between documents"
        echo "  - Validate Markdown formatting"
        echo "  - Check cross-repository consistency"
        echo "  - Verify Palantir implementation status"
        echo ""
        echo "🚀 Ready to begin comprehensive document review"
        ;;
    *)
        echo "❌ Invalid repository. Use: phoenix, bedrock, or both"
        exit 1
        ;;
esac

echo ""
echo "📝 Please run the following command to initiate the review:"
echo ""
echo "UnderLord — Document Review and Cleanup"
echo ""
echo "🎯 Objective"
echo "Review all documentation for internal and cross-consistency, ensuring all docs reflect the current Phase 0.5 Gate PASS results (Qt 6.10 + Qt Graphs / gRPC UDS)."
echo ""
echo "🧩 Tasks"
echo "1. Scan for outdated references (Qt 6.9, Qt Charts, LocalSocket)"
echo "2. Verify all links and relative paths work correctly"
echo "3. Ensure consistency with Phase 0.5 Gate PASS results"
echo "4. Archive any superseded materials with proper banners"
echo "5. Update cross-references between documents"
echo "6. Validate Markdown formatting and structure"
echo "7. Check cross-repository consistency (if both repos)"
echo "8. Verify Palantir implementation status (Bedrock)"
echo ""
echo "🚦 Guardrails"
echo "- No code modifications — docs only"
echo "- All Qt Charts artifacts must be quarantined to legacy"
echo "- All references to Qt 6.9 or LocalSocket must be removed or marked historical"
echo "- Ensure every Markdown file opens cleanly and links resolve"
echo ""
echo "📊 Deliverable"
echo "A clean, consistent documentation set with all outdated references properly archived and all links validated."



