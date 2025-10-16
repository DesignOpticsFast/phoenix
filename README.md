# Phoenix — GUI/IDE for Bedrock

**Phoenix** is the Qt-based GUI that orchestrates **Bedrock** primitives:
visualization, tolerancing, reporting, and (eventually) AI-assisted workflows.

> Phoenix provides the interactive UX layer; Bedrock is the compute foundation.

---

## 🚀 Current Scope (MVP Phase 1)

Sprint: **MVP Phase 1 — New Design (TSE)**

- Add **“New Design”** toolbar button.
- Call Bedrock API to create a **Two-Surface Element (TSE)** with hard-wired parameters.
- Bedrock generates a **STEP file** for the TSE.
- Phoenix opens a **3D STEP viewer** to display the element.
- No editing yet — view-only.

---

## 📂 Repo Structure
phoenix/
├── src/
│    ├── ui/                # Qt windows, STEP viewer
│    └── adapters/          # Bedrock client adapter
├── resources/              # icons, Qt .ui files
├── docs/                   # ADRs, design notes
├── CMakeLists.txt          # build config
└── .github/
├── workflows/         # CI pipelines
└── pull_request_template.md
---

## 🛠️ Development Workflow

### Branch Naming
Use `type/scope/short-desc`:

- `feature/phoenix/new-design-button`
- `feature/phoenix/step-viewer`
- `fix/phoenix/step-viewer-crash`
- `docs/phoenix/readme-update`

Rules:
- lowercase, hyphen-separated
- scope = `phoenix`, `bedrock`, `ci`, `docs`, etc.
- short and specific (≤ 4 words)

### Commits
Use conventional style:

- `feat(phoenix): add STEP viewer widget`
- `feat(phoenix): New Design toolbar button`
- `chore(ci): add smoke test`
- `fix(phoenix): guard null shape in STEP viewer`

### Pull Requests
- **One branch = one Issue**
- Link PR to Issue: `Closes #ISSUE_NUMBER`
- Assign Milestone: `MVP Phase 1 — New Design (TSE)`
- Fill out PR template checklist
- Squash & merge; delete branch after merge

---

## ✅ Definition of Done (for Sprint 1)

- Phoenix builds and runs
- “New Design” button creates a STEP file via Bedrock
- STEP file loads in Phoenix viewer
- CI smoke test green (offscreen run)

---

## 🔮 Future Sprints

- SOM inspector (read-only values)
- Editing parameters in UI
- Import/export support (Rosetta, ZMX)
- Tolerancing and optimization workflows
- AI-assisted design tools
# Trigger validation with infra-ci label
# Test commit for dev-01 runner
