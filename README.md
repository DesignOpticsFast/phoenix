# Phoenix — GUI/IDE for Optical Design

> **Version**: 0.0.4  
> **Status**: CI workflows running on GitHub Actions ✅  
> **Icons**: Font Awesome Pro icon system integrated 🎨  
> **CI**: GitHub Actions 🚀  
> **Icon system fixed**: 2025-01-21  
> **Toolchain**: See [docs/VERSIONS.md](docs/VERSIONS.md) for current versions

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

## 🚀 Current Scope (Sprint 4.5)

**Version 0.0.4** — Envelope Protocol & IPC Hardening

- **Icon System**: Font Awesome Pro integration with theme-aware tinting
- **Palantir IPC**: Non-blocking connection FSM with exponential backoff
- **Protocol Layer**: Envelope-based Palantir protocol (MessageEnvelope with length prefix)
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

### **🎯 CI/CD: GitHub Actions**

**CI runs on GitHub Actions with:**
- ✅ **Linux builds** - Ubuntu latest (see [docs/VERSIONS.md](docs/VERSIONS.md) for toolchain versions)
- ✅ **Automated testing** - Unit tests run on every PR
- ✅ **Integration tests** - Bedrock integration tests in CI
- ✅ **Quality assurance** - Automated validation and checks

**Local Development:**
1. **Develop locally** - Make code changes on your machine
2. **Test locally** - Build and test (see [docs/VERSIONS.md](docs/VERSIONS.md) for toolchain versions)
3. **Commit and push** - Push changes to repository
4. **CI validates** - GitHub Actions runs tests automatically

> **Note:** For current toolchain versions (Qt, C++ standard, CMake, Protobuf, etc.), see [docs/VERSIONS.md](docs/VERSIONS.md).

### **Local Build Setup**

**macOS:**
```bash
# Install Qt (via Homebrew or Qt installer, see docs/VERSIONS.md for current version)
brew install qt@6  # or download from qt.io

# Initialize submodules
git submodule update --init --recursive

# Build (see docs/VERSIONS.md for Qt path)
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6) \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

**Linux:**
```bash
# Install Qt (via package manager or Qt installer, see docs/VERSIONS.md for current version)
# Then set CMAKE_PREFIX_PATH to Qt installation

# Initialize submodules
git submodule update --init --recursive

# Build (see docs/VERSIONS.md for Qt path)
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/<version>/gcc_64 \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

**Important:** For current Qt version and installation paths, see [docs/VERSIONS.md](docs/VERSIONS.md). CI uses `jurplel/install-qt-action@v3` to install Qt automatically.

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
- ✅ **GitHub Actions CI** - Automated builds and tests
- ✅ **Qt 6.10.1** - Latest stable Qt version
- ✅ **Local development** - Build and test on your machine
- ✅ **Resource debugging** - Proper testing of resource embedding

### **Documentation Updates**
- ✅ **DEVELOPMENT_WORKFLOW.md** - Comprehensive development workflow
- ✅ **CODING_STANDARDS.md** - Coding standards and guidelines
- ✅ **CI_WORKFLOW_SYSTEM.md** - CI workflow documentation
- ✅ **README.md** - Updated with GitHub Actions workflow

### **Technical Achievements**
- ✅ **Resource embedding working** - Individual files instead of QRC
- ✅ **Icon manifest updated** - Font Awesome Unicode characters
- ✅ **Build process optimized** - CMake configuration improved
- ✅ **Quality assurance** - Automated testing on GitHub Actions

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
# CI runs on GitHub Actions
# Test Phoenix CI
# Final Sprint 3 validation
