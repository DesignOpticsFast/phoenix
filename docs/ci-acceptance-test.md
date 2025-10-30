# Test CI Workflow - Acceptance Test

This is a test to verify the CI workflow is working correctly.

## Test Purpose

- Verify CI workflow triggers on push to main
- Check if workflow_dispatch is working
- Ensure all protection layers are active

## Expected Behavior

- CI should run successfully
- All protection layers should be active
- Workflow should complete without errors

## Acceptance Checklist

1. ✅ Docs-only PR → only docs jobs run, Gate = success
2. ✅ Code PR touching src/ → build job runs, Gate = success (or fails for real issues)
3. ✅ Push to main → build job runs (by design)
4. ✅ Conflict guard trip test → open a PR with dummy workflow containing <<<<<<< → CI fails loudly in guard step
5. ✅ Weekly audit dry-run → manually trigger workflow_dispatch; confirm lint + guard run clean
