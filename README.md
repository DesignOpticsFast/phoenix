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
│    └── app/               # Application core (settings, I/O utilities)
├── resources/              # icons, Qt .ui files
├── docs/                   # ADRs, design notes
├── CMakeLists.txt          # build config
└── .github/
├── workflows/         # CI pipelines
└── pull_request_template.md
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

# Test GUI with Xvfb
xvfb-run -a ./phoenix_app
```

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
