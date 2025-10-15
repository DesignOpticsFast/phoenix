# Infrastructure Briefing for UnderLord

**Date:** 2025-10-15
**Subject:** Repository architecture changes and new operational rules
**Status:** CRITICAL - Read before Sprint 3 execution

---

## CRITICAL CHANGES YOU MUST KNOW

### 1. Repository Locations CHANGED

OLD (deleted): /home/ec2-user/phoenix-check/
OLD (deleted): /home/ec2-user/bedrock-check/

NEW (canonical): /home/ec2-user/workspace/phoenix/
NEW (canonical): /home/ec2-user/workspace/bedrock/

Always use workspace/ directories. Never reference -check/ paths.

### 2. Remote Configuration CHANGED

Phoenix: Single origin remote to DesignOpticsFast/phoenix
Bedrock: Single origin remote to DesignOpticsFast/bedrock

All pushes go to origin. No upstream remote exists.

### 3. Authentication CHANGED

Now using SSH keys. No OAuth tokens. No scope restrictions.

### 4. Constitutional Rules (MANDATORY)

Rule 1: Canonical-Clone Principle
- Location: /home/ec2-user/workspace/{phoenix,bedrock} ONLY
- Single origin remote per repo
- No forks permitted in sprint execution

Rule 2: Stage 0 Infrastructure Gate  
- Run preflight.sh before EVERY sprint
- Validates infrastructure before allowing sprint work

Rule 3: No Forks
- Prohibited for CI-validated sprint work
- GitHub Actions security blocks CI on forks

Rule 4: Assisted Autonomy Between Breakpoints
- Execute commands without line-level approvals between breakpoints
- Ask before restricted actions (branch protection, credentials, force pushes, etc.)
- Always stop-the-line for failures or uncertainty

### 5. Sprint 3A Status

Sprint 3A: ABORTED due to infrastructure issues
Resolution: All issues fixed during Sprint 3A-Infra
Next: Sprint 3 (fresh start with certified infrastructure)

## Required Reading Before Sprint 3

1. INFRASTRUCTURE_BRIEFING.md (this document)
2. REPOSITORY_ARCHITECTURE.md
3. AI_OPERATIONS_GUIDE.md (constitutional rules section)

## Pre-Sprint Checklist

- Repository location: workspace/, not -check/
- Remote: origin only, no upstream
- Authentication: SSH (automatic)
- Preflight: Must pass Stage 0 and Stage 1
- Autonomy policy: Understood
- Constitutional rules: Internalized

## Infrastructure Certification Evidence

Phoenix CI: https://github.com/DesignOpticsFast/phoenix/actions/runs/18537869143
Bedrock CI: https://github.com/DesignOpticsFast/bedrock/actions/runs/18538030999
Status: CERTIFIED (2025-10-15)

Acknowledge this briefing before proceeding with Sprint 3.
