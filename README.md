# Phoenix — GUI/IDE for Optical Design

> **Version**: 0.0.3  
> **Status**: CI workflows stabilized and tested ✅  
> **Icons**: Font Awesome Pro icon system integrated 🎨  
> **Development**: dev-01-first workflow established 🚀  
> **CI stabilized on**: 2025-01-27  
> **Icon system fixed**: 2025-01-21

[![CI](https://github.com/DesignOpticsFast/phoenix/actions/workflows/ci.yml/badge.svg)](https://github.com/DesignOpticsFast/phoenix/actions/workflows/ci.yml)
[![CodeQL](https://github.com/DesignOpticsFast/phoenix/actions/workflows/codeql.yml/badge.svg)](https://github.com/DesignOpticsFast/phoenix/actions/workflows/codeql.yml)
[![PR Guard](https://github.com/DesignOpticsFast/phoenix/actions/workflows/pr-guard.yml/badge.svg)](https://github.com/DesignOpticsFast/phoenix/actions/workflows/pr-guard.yml)
[![Policy](https://github.com/DesignOpticsFast/phoenix/actions/workflows/policy.yml/badge.svg)](https://github.com/DesignOpticsFast/phoenix/actions/workflows/policy.yml)
[![Auto-merge Dependabot](https://github.com/DesignOpticsFast/phoenix/actions/workflows/auto-merge-dependabot.yml/badge.svg)](https://github.com/DesignOpticsFast/phoenix/actions/workflows/auto-merge-dependabot.yml)

📋 **[Workflow Dependencies](docs/workflow-dependencies.md)** | 🎨 **[Icon System](docs/icons.md)** | 🚀 **[Development Workflow](docs/DEVELOPMENT_WORKFLOW.md)** | 🛡️ **[CI Workflow System](docs/CI_WORKFLOW_SYSTEM.md)** | 📚 **[API Documentation](docs/)**

**Phoenix** is the Qt-based GUI for optical design workflows:
visualization, tolerancing, reporting, and (eventually) AI-assisted design.

> Phoenix provides the interactive UX layer; communication with compute services occurs via **Palantir** (IPC layer), which connects to **Bedrock** (compute foundation).

---

## 🚀 Current Scope (Sprint 4/5)

**Version 0.0.3** — UI Hardening & Palantir Foundation

- **Icon System**: Font Awesome Pro integration with theme-aware tinting
- **Palantir IPC**: Non-blocking connection FSM with exponential backoff
- **Protocol Layer**: Framed binary protocol (PLTR magic, BigEndian, version/type/length header)
- **Message Dispatcher**: Type→handler registration scaffold for future features
- **UI Polish**: Version display in splash, window title, and About dialog

---

## 📂 Repo Structure
phoenix/
├── src/
│    ├── ui/                # Qt windows, dialogs, widgets
│    ├── palantir/          # Palantir IPC client (non-blocking FSM, protocol framing)
│    ├── app/               # Application core (settings, I/O utilities)
│    └── common/            # Shared utilities (canonical_json, etc.)
├── contracts/              # Palantir contracts submodule (DesignOpticsFast/palantir)
├── third_party/            # Third-party dependencies (qcustomplot, libsodium)
├── resources/              # icons, Qt .ui files
├── docs/                   # ADRs, design notes
├── .contract-version       # Pinned contracts commit SHA
├── CMakeLists.txt          # build config
└── .github/
├── workflows/         # CI pipelines
└── pull_request_template.md

---

## 📋 Contracts Submodule

Phoenix uses the Palantir contracts repository as a git submodule:

- **Path:** `contracts/`
- **Repository:** `DesignOpticsFast/palantir`
- **Current Version:** v1.0.0 (commit `ad0e9882cd3d9cbbf80fc3b4ac23cd1df7547f53`)
- **Purpose:** Shared transport/contract specifications for Phoenix ↔ Bedrock communication

The contracts submodule is pinned via `.contract-version` file. CI enforces that the submodule commit matches the `.contract-version` file to ensure contract SHA parity between Phoenix and Bedrock.

**To initialize the submodule:**
```bash
git submodule update --init --recursive
```

**To update the contracts submodule:**
```bash
cd contracts
git fetch origin
git checkout <desired-commit-or-tag>
cd ..
# Update .contract-version to match the new commit SHA
git add contracts .contract-version
git commit -m "chore: update contracts submodule to <version>"
```

**Note:** Phoenix must keep the contracts submodule in sync with Bedrock. Both repos use the same commit SHA (`ad0e988...`) to ensure compatibility.

---

## 🛠️ Development Workflow

### **🎯 Core Policy: ALL CODE CHANGES ON DEV-01 FIRST**

**Rationale:**
- ✅ **Consistent build environment** - Linux-based development
- ✅ **Automated testing** - CI/CD pipelines run on dev-01
- ✅ **Resource debugging** - Can test GUI with Xvfb
- ✅ **Version control** - All changes tracked in Git
- ✅ **Quality assurance** - Automated tests and validation

**Workflow:**
1. **Develop on dev-01** - Make all code changes via Tailscale
2. **Test on dev-01** - Build and test with Xvfb for GUI
3. **Commit from dev-01** - Push changes to repository
4. **Test locally** - Pull changes and test on local machine

### **Connection Setup**
```bash
# Connect to dev-01 via Tailscale
ssh -i ~/.ssh/github_phoenix mark@100.97.54.75
cd /home/ec2-user/workspace/phoenix

# Initialize submodules (if not already done)
git submodule update --init --recursive

# Build with Qt 6.10.0 (required)
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64 \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build

# Test GUI with Xvfb
xvfb-run -a ./build/phoenix_app
```

**Important:** Phoenix must be configured with Qt 6.10.0 on dev-01. Always use `-DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64` when configuring CMake.

### **Branch Naming**
Use `type/scope/short-desc`:

- `feature/phoenix/new-design-button`
- `feature/phoenix/step-viewer`
- `fix/phoenix/step-viewer-crash`
- `docs/phoenix/readme-update`

Rules:
- lowercase, hyphen-separated
- scope = `phoenix`, `bedrock`, `ci`, `docs`, etc.
- short and specific (≤ 4 words)

### **Commits**
Use conventional style:

- `feat(phoenix): add STEP viewer widget`
- `feat(phoenix): New Design toolbar button`
- `chore(ci): add smoke test`
- `fix(phoenix): guard null shape in STEP viewer`

### **Pull Requests**
- **One branch = one Issue**
- Link PR to Issue: `Closes #ISSUE_NUMBER`
- Assign Milestone: `MVP Phase 1 — New Design (TSE)`
- Fill out PR template checklist
- Squash & merge; delete branch after merge

---

## 🐧 Linux Platform Notes

Phoenix uses X11/XWayland by default on Linux (`QT_QPA_PLATFORM=xcb`) to provide 
reliable toolbar and dock dragging. Native Wayland docking in Qt is currently 
limited and may produce warnings like:

> "This plugin supports grabbing the mouse only for popup windows"

Advanced users can force native Wayland by setting `PHOENIX_FORCE_WAYLAND=1` 
before launching Phoenix, but docking behavior may be constrained on some compositors.

This is a temporary, pragmatic choice until Qt's Wayland docking support improves.

**Example:**
```bash
# Use default X11/XWayland (recommended)
./phoenix_app

# Force native Wayland
PHOENIX_FORCE_WAYLAND=1 ./phoenix_app

# Or set QT_QPA_PLATFORM directly
QT_QPA_PLATFORM=wayland ./phoenix_app
```

---

## ✅ Definition of Done (for Sprint 1)

- Phoenix builds and runs
- "New Design" button creates a STEP file via Bedrock
- STEP file loads in Phoenix viewer
- CI smoke test green (offscreen run)

---

## 🎯 **Sprint Work Completed (2025-01-21)**

### **Icon System Fixes**
- ✅ **File menu icons working** - System icons instead of "?" placeholders
- ✅ **Resource loading fixed** - Proper embedding and path resolution
- ✅ **Font Awesome integration** - Fonts loading correctly
- ✅ **Phoenix icon path corrected** - Application icon in Dock

### **Development Workflow Established**
- ✅ **dev-01-first policy** - All code changes on dev-01 first
- ✅ **Tailscale integration** - Secure connection for development
- ✅ **Xvfb GUI testing** - Can test Phoenix UI on dev-01
- ✅ **Resource debugging** - Proper testing of resource embedding

### **Documentation Updates**
- ✅ **DEVELOPMENT_WORKFLOW.md** - Comprehensive dev-01 workflow
- ✅ **CODING_STANDARDS.md** - Updated with dev-01 policy
- ✅ **dev-setup.md** - Added Tailscale integration
- ✅ **README.md** - Updated with new workflow

### **Technical Achievements**
- ✅ **Resource embedding working** - Individual files instead of QRC
- ✅ **Icon manifest updated** - Font Awesome Unicode characters
- ✅ **Build process optimized** - CMake configuration improved
- ✅ **Quality assurance** - Automated testing on dev-01

### **CI Workflow System (2025-01-23)**
- ✅ **Self-healing CI system** - Comprehensive conflict prevention
- ✅ **Smart change detection** - Docs vs code change classification
- ✅ **Protection layers** - Pre-commit hooks, conflict guards, YAML linting
- ✅ **Access control** - CODEOWNERS protection for workflow changes
- ✅ **Automated monitoring** - Weekly health checks and status badges
- ✅ **Production-ready** - Bulletproof CI workflow with guardrails

---

## 🔮 Future Sprints

- SOM inspector (read-only values)
- Editing parameters in UI
- Import/export support (Rosetta, ZMX)
- Tolerancing and optimization workflows
- AI-assisted design tools
# Trigger validation with infra-ci label
# Test commit for dev-01 runner
# Test Phoenix CI
# Final Sprint 3 validation
