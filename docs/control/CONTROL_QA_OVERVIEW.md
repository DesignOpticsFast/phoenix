# CONTROL QA Overview – Phoenix

**Repository:** Phoenix  
**Purpose:** Build & QA governance reference

---

## Build & QA Governance

Build and QA behavior for the Phoenix repository is governed by the shared CONTROL document:

**`docs/control/CONTROL_QA_MULTI_PLATFORM_v1.0.md` at the workspace root**

This canonical document defines the multi-platform build and QA strategy for the entire DesignOpticsFast stack (Phoenix, Bedrock, Palantir), including:

- Build infrastructure and toolchains ("One Ring" concept)
- Daily gates and overnight QA phases
- Cross-repo consistency and rollback rules
- Forward-looking QA objectives (heavy OpenMP benchmark, Phoenix scripting/ghost user automation)

---

## Repository-Specific Documentation

Phoenix-specific build and QA implementation details are documented locally in:

- `CMakePresets.json` – Build presets and configurations
- `scripts/` – Build and test scripts
- `docs/qa/` – QA implementation details and reports
- `docs/adr/` – Architecture Decision Records for Phoenix-specific choices

---

## Navigation

From the workspace root (`/Users/underlord/workspace`), the canonical CONTROL document is located at:

```
docs/control/CONTROL_QA_MULTI_PLATFORM_v1.0.md
```

Repository-specific details supplement the canonical CONTROL document but do not override it.

