# CI Workflow System - Phoenix

**Version:** 1.0.0  
**Last Updated:** 2025-01-23  
**Status:** Production Ready ✅

---

## 🎯 **Overview**

The Phoenix CI workflow system is a **self-healing, production-ready** continuous integration system designed to prevent common issues and ensure reliable builds. It includes comprehensive protection against merge conflicts, workflow syntax errors, and unauthorized changes.

---

## 🛡️ **Protection Layers**

### **Layer 1: Pre-commit Protection**

- **Pre-commit hook** - Prevents conflict markers from being committed locally
- **Location**: `.git/hooks/pre-commit`
- **Function**: Scans staged changes for conflict markers (`<<<<<<<`, `>>>>>>>`)
- **Action**: Blocks commit if conflict markers detected

### **Layer 2: Workflow Validation**

- **Conflict marker guard** - Fails CI loudly if conflicts detected in workflows
- **YAML linting** - Validates workflow syntax with `yamllint`
- **Location**: `.github/workflows/ci.yml` (changes job)
- **Function**: Early detection of workflow issues

### **Layer 3: Access Control**

- **CODEOWNERS** - Requires maintainer review for workflow changes
- **Location**: `.github/CODEOWNERS`
- **Function**: Ensures all workflow edits are peer-reviewed
- **Scope**: `.github/workflows/*` files

### **Layer 4: Automated Monitoring**

- **Weekly self-audit** - Runs every Monday at 09:00 UTC
- **Status badges** - Visible CI health indicators in README
- **Manual triggers** - `workflow_dispatch` for debugging

---

## 🔧 **CI Workflow Features**

### **Smart Change Detection**

- **Tool**: `dorny/paths-filter@v3`
- **Function**: Automatically detects docs-only vs code changes
- **Benefits**: Runs appropriate jobs based on change type

### **Conditional Job Execution**

- **Docs-only PRs**: Run lightweight checks (markdownlint, link checker, spell check)
- **Code changes**: Run full build and test suite
- **Push to main**: Always runs build (not just docs)

### **Lightweight Docs Checks**

- **markdownlint** - Markdown formatting and style
- **lychee** - Link checking with retry logic
- **codespell** - Spell checking with domain-specific ignore list

### **Single Gate Job**

- **Purpose**: Prevents deadlocks with branch protection
- **Function**: Single required status check
- **Logic**: Docs-only requires docs success, code changes require build success

---

## 📋 **Workflow Triggers**

```yaml
on:
  pull_request:
    types: [opened, synchronize, reopened, ready_for_review]
    branches: [ main ]
  push:
    branches: [ main ]
  workflow_dispatch:           # manual trigger
  schedule:
    - cron: "0 9 * * 1"       # weekly self-audit
```

### **Trigger Explanations**

- **pull_request**: Runs on PR events targeting main branch
- **push**: Runs on pushes to main branch
- **workflow_dispatch**: Manual trigger for debugging
- **schedule**: Weekly Monday 09:00 UTC health check

---

## 🚀 **Job Structure**

### **1. Changes Job**

- **Purpose**: Detect change type (docs vs code)
- **Runner**: `ubuntu-latest`
- **Features**: Conflict detection, YAML linting, change classification
- **Outputs**: `docs_only`, `code_changed`

### **2. Docs Job**

- **Purpose**: Lightweight documentation checks
- **Runner**: `ubuntu-latest`
- **Condition**: `docs_only == true`
- **Checks**: markdownlint, link checker, spell check

### **3. Build Job**

- **Purpose**: Full application build and test
- **Runner**: `self-hosted` (Amazon Linux 2023)
- **Condition**: `code_changed == true` OR `push` event
- **Steps**: Configure, Build, Unit tests

### **4. Gate Job**

- **Purpose**: Single required status check
- **Runner**: `ubuntu-latest`
- **Condition**: `always()`
- **Logic**: Docs-only requires docs success, code changes require build success

---

## 🔍 **Troubleshooting**

### **Common Issues**

#### **CI Workflow Not Triggering**

1. **Check for merge conflicts** in `.github/workflows/ci.yml`
2. **Verify branch filters** - ensure `branches: [ main ]` is present
3. **Check PR status** - ensure PR is not draft
4. **Verify workflow file** exists on main branch

#### **Conflict Marker Detection**

```bash
# Check for conflict markers
grep -R -nE '^<<<<<<<|^>>>>>>>' .github/workflows

# Resolve conflicts
git checkout --ours .github/workflows/ci.yml
git add .github/workflows/ci.yml
git commit -m "resolve: Fix merge conflicts in CI workflow"
```

#### **YAML Linting Failures**

```bash
# Check YAML syntax
yamllint .github/workflows/ci.yml

# Fix common issues
# - Indentation (use 2 spaces)
# - Quoted strings
# - Valid GitHub Actions syntax
```

### **Manual Debugging**

```bash
# Trigger workflow manually
gh workflow run ci.yml

# Check workflow status
gh run list --workflow ci.yml

# View workflow logs
gh run view <run-id> --log
```

---

## 📊 **Monitoring & Health**

### **Status Badges**

```markdown
[![CI](https://github.com/DesignOpticsFast/phoenix/actions/workflows/ci.yml/badge.svg)](https://github.com/DesignOpticsFast/phoenix/actions/workflows/ci.yml)
```

### **Weekly Health Check**

- **Schedule**: Every Monday at 09:00 UTC
- **Purpose**: Automated validation of workflow files
- **Checks**: Conflict markers, YAML syntax, workflow structure
- **Notification**: Fails loudly if issues detected

### **Manual Health Check**

```bash
# Run local validation
yamllint .github/workflows/ci.yml
grep -R -nE '^<<<<<<<|^>>>>>>>' .github/workflows

# Test workflow syntax
gh workflow run ci.yml --ref main
```

---

## 🛠️ **Configuration Files**

### **CI Workflow**

- **File**: `.github/workflows/ci.yml`
- **Purpose**: Main CI workflow definition
- **Features**: Smart change detection, conditional jobs, conflict protection

### **CODEOWNERS**

- **File**: `.github/CODEOWNERS`
- **Purpose**: Access control for workflow changes
- **Scope**: `.github/workflows/*` files

### **Pre-commit Hook**

- **File**: `.git/hooks/pre-commit`
- **Purpose**: Local conflict marker prevention
- **Function**: Blocks commits with conflict markers

### **Configuration Files**

- **File**: `.markdownlint.json`
- **Purpose**: Markdown linting rules
- **Scope**: Documentation files

- **File**: `.codespellignore`
- **Purpose**: Spell checker ignore list
- **Scope**: Domain-specific terms

- **File**: `.lycheeignore`
- **Purpose**: Link checker ignore list
- **Scope**: Flaky external URLs

---

## 🎯 **Best Practices**

### **Workflow Development**

1. **Test locally first** - Use `yamllint` to validate syntax
2. **Check for conflicts** - Ensure no merge conflict markers
3. **Use descriptive names** - Clear job and step names
4. **Add comments** - Explain complex logic
5. **Test manually** - Use `workflow_dispatch` for debugging

### **Conflict Prevention**

1. **Rebase before merging** - Keep history clean
2. **Resolve conflicts immediately** - Don't leave conflict markers
3. **Test after resolution** - Ensure workflow still works
4. **Use CODEOWNERS** - Require review for workflow changes

### **Monitoring**

1. **Check status badges** - Monitor CI health in README
2. **Review weekly audits** - Check Monday health check results
3. **Watch for failures** - Investigate CI failures promptly
4. **Update documentation** - Keep this guide current

---

## 📚 **Related Documentation**

- **[Development Workflow](DEVELOPMENT_WORKFLOW.md)** - Overall development process
- **[Coding Standards](CODING_STANDARDS.md)** - Code quality requirements
- **[Dev Setup](dev-setup.md)** - Environment configuration

---

## 🔄 **Version History**

### **1.0.0 (2025-01-23)**

- **INITIAL RELEASE**: Production-ready CI workflow system
- **NEW**: Self-healing conflict prevention
- **NEW**: Smart change detection (docs vs code)
- **NEW**: Comprehensive protection layers
- **NEW**: Weekly automated health checks
- **NEW**: Manual debugging capabilities

---

**Maintained By:** Engineering Team  
**Questions:** See `.underlord/docs/GOTCHAS.md` for common issues  
**Last Updated:** 2025-01-23
