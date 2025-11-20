# UNDERLORD MAC ACCESS POLICY

_Last updated: Sprint 4.3 — Mac demo enablement_

This policy governs how UnderLord may interact with Mark's Mac.

---

## 1. Access Boundaries

1. UnderLord **may NOT** access Mark's Mac at any time without **explicit, per-occasion authorization** from Mark.

2. Authorization must be:
   - Given in a ChatGPT prompt,
   - Explicit (not implied),
   - Specific to the task or chunk being executed,
   - Granted **after** UnderLord has presented a plan.

3. Previous authorizations **do not carry forward**:
   - Every session is independent.
   - No implicit permissions.
   - No assumptions.

If there is any ambiguity, UnderLord must **assume NO authorization**.

---

## 2. Allowed Operations (Only With Explicit Authorization)

Once Mark has explicitly authorized Mac access for a specific task, UnderLord may:

- SSH into the Mac as `underlord` using the configured SSH key.
- Work **only** under `/Users/underlord/`, specifically:
  - `/Users/underlord/workspace/`
  - Subdirectories like `/Users/underlord/workspace/phoenix`, `/Users/underlord/workspace/build-*`
- Run build and test commands related to the Phoenix project.
- Install development tooling via Homebrew **only if Mark explicitly approves in that session's prompt**.

UnderLord **MUST NOT**, under any circumstances:

- Access `/Users/mark/` or any other user's home directory.
- Read, modify, or copy personal files, documents, photos, or non-work artifacts.
- Modify system settings unrelated to development tooling.
- Access the macOS Keychain, passwords, tokens, or other credentials.
- Create, modify, or delete macOS user accounts.

If a requested action might cross these boundaries, UnderLord must stop and ask Mark for clarification.

---

## 3. Authentication Rules

- UnderLord uses **only** the SSH key configured for the `underlord` user on dev-01
  to connect to `underlord@marksmaudio1077` (or the Mac's Tailscale IP).
- UnderLord must **not** generate new SSH keys for the Mac or request Mac passwords
  without explicit authorization from Mark in that session.
- If an SSH connection fails due to keys or auth, UnderLord must report the problem
  and wait for Mark's instructions; it must not "guess" alternate credentials or
  attempt brute-force behavior.

---

## 4. Daily Review Requirement ("make daily")

As part of the first "make daily" run each day, UnderLord must:

1. **Read this document**: `docs/UNDERLORD_MAC_ACCESS.md`.
2. Confirm in its output something equivalent to:
   - "I have reviewed the UNDERLORD_MAC_ACCESS policy for this day."
   - "I will not access Mark's Mac without explicit permission in this session."
3. If any Mac-related work is requested later in the same day, UnderLord must:
   - Present a plan,
   - Ask: "Do you authorize Mac access for this specific task? (Yes/No)"
   - Proceed with Mac access **only** if Mark responds with an explicit "Yes".

If there is no explicit "Yes", UnderLord must not access the Mac.

---

## 5. Logging Requirements for Mac Sessions

Whenever Mark explicitly authorizes Mac access for a given task, UnderLord must:

- Clearly state in its response:
  - That Mac access has been authorized,
  - What it intends to do on the Mac.
- After completing the work, provide a short session log including:
  - The directories it touched (within `/Users/underlord/...`),
  - The types of commands it ran (builds, tests, file edits),
  - Whether any files were modified or created.

This log can be part of the normal chunk completion report.

---

## 6. Revocation and Key Rotation

Mark can revoke UnderLord's Mac access at any time by saying:

> "Revoke UnderLord Mac access now."

When this happens, UnderLord must:

1. Treat any existing SSH keys as invalid for future use.
2. Not attempt any Mac SSH connection until:
   - Mark explicitly authorizes new key generation and installation, and
   - A new key has been configured and confirmed.

---

## 7. Scope of This Policy

This policy **only** governs UnderLord's access to Mark's Mac.

It does not replace or weaken any other security policies
for dev-01, cloud infrastructure, or other systems.

If there is any conflict between this policy and another,
UnderLord must choose the **more restrictive** behavior and ask Mark for guidance.

---

_End of UNDERLORD MAC ACCESS POLICY_

---
