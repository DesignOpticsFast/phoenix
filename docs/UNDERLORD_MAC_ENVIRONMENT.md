# UnderLord Mac Environment Map

_Last updated: Sprint 4.3 — Unified Daily Ritual_

This document provides context about the Mac development environment structure. **This is context only, not permission for Mac access.**

---

## 1. Valid User Account

- **Account name:** `underlord`
- UnderLord must **only** use this account for Mac access
- No access to other user accounts (especially `/Users/mark/`)

---

## 2. Workspace Structure

**Valid workspace root:**
```
/Users/underlord/workspace/
```

**Allowed subdirectories:**
- `/Users/underlord/workspace/phoenix/` - Phoenix repository clone
- `/Users/underlord/workspace/build-demo/` - Demo build directory
- `/Users/underlord/workspace/build-transport/` - Transport-enabled build directory
- `/Users/underlord/workspace/build-*/` - Other build directories
- `/Users/underlord/workspace/logs/` - Log files (if exists)

**UnderLord may:**
- Clone repositories into `/Users/underlord/workspace/`
- Create build directories under `/Users/underlord/workspace/`
- Build Phoenix projects
- Run tests
- Read Qt installation at `/Users/mark/Qt/6.10.0/macos` (read-only)

---

## 3. Forbidden Operations

UnderLord **MUST NOT**:

- Modify anything under `/Users/mark/`
- Access non-workspace directories
- Run scripts affecting the system
- Install OS-level software without explicit approval
- Access personal files, documents, photos, or non-work artifacts
- Modify system settings unrelated to development tooling
- Access macOS Keychain, passwords, tokens, or credentials
- Create, modify, or delete macOS user accounts

---

## 4. Qt Installation (Read-Only)

**Qt location:**
```
/Users/mark/Qt/6.10.0/macos
```

**Rules:**
- This path is **read-only** for UnderLord
- UnderLord may read Qt configuration and libraries
- UnderLord may **not** modify Qt installation
- CMake may reference this path for builds

---

## 5. SSH Access

- UnderLord connects via SSH as `underlord@marksmaudio1077` (or Tailscale IP)
- Uses SSH key configured on dev-01
- Must follow `UNDERLORD_MAC_ACCESS.md` policy for authorization

---

## 6. Important Notes

- This document provides **context only**
- It does **not** grant permission for Mac access
- Mac access requires explicit authorization per `UNDERLORD_MAC_ENVIRONMENT.md`
- All Mac operations must stay within `/Users/underlord/workspace/`

---

_End of UnderLord Mac Environment Map_

---

