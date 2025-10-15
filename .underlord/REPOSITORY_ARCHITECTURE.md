# Repository Architecture - Phoenix
Last Updated: 2025-10-15
Status: CERTIFIED for sprint execution

Canonical Configuration:
- Location: /home/ec2-user/workspace/phoenix
- Remote: origin git@github.com:DesignOpticsFast/phoenix.git
- Authentication: SSH key ~/.ssh/github_phoenix
- Default Branch: main

Constitutional Rules:
- Canonical Clone Only (single working clone, no forks, no duplicates)
- SSH Authentication Required (no OAuth tokens)
- CI Integration Verified (Evidence: https://github.com/DesignOpticsFast/phoenix/actions/runs/18537869143)

Validation: Run .underlord/preflight.sh - Expected: Stage 0 PASS, Stage 1 PASS

History:
2025-10-15: Initial certification (Sprint 3A-Infra)
- Fork removed, duplicate clones consolidated, SSH configured, CI verified, autonomy policy established

Do not modify this architecture without updating this document and running full infrastructure validation.
