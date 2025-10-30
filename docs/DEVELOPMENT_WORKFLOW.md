# Development Workflow - Phoenix/Bedrock

**Version:** 2.0.0  
**Last Updated:** 2025-01-21  
**Environment:** dev-01 (Primary) + Local Machines (Secondary)

---

## 🎯 **Core Development Policy**

### **ALL CODE CHANGES MUST BE DONE ON DEV-01 FIRST**

**Rationale:**

- ✅ **Consistent build environment** - Linux-based development
- ✅ **Automated testing** - CI/CD pipelines run on dev-01
- ✅ **Resource debugging** - Can test GUI with Xvfb
- ✅ **Version control** - All changes tracked in Git
- ✅ **Collaboration** - Team can access and review changes
- ✅ **Quality assurance** - Automated tests and validation

---

## 🚀 **Development Workflow**

### **Phase 1: Development on dev-01**

#### **1.1 Environment Setup**

```bash
# Connect to dev-01 via Tailscale
ssh -i ~/.ssh/github_phoenix -o StrictHostKeyChecking=no mark@100.97.54.75

# Navigate to project
cd /home/ec2-user/workspace/phoenix
```

#### **1.2 Code Changes**

- ✅ **Make all code changes on dev-01**
- ✅ **Test builds on dev-01**
- ✅ **Test GUI with Xvfb** (for Phoenix UI testing)
- ✅ **Run automated tests**
- ✅ **Debug resource loading issues**
- ✅ **Capture screenshots for verification**

#### **1.3 Testing & Validation**

```bash
# Build and test on dev-01
cd /home/ec2-user/workspace/phoenix
mkdir -p build && cd build
cmake .. -G Ninja
ninja -k0

# Test GUI with Xvfb (for Phoenix)
xvfb-run -a ./phoenix_app
```

#### **1.4 Commit & Push**

```bash
# Commit changes to dev-01
git add .
git commit -m "feat(ui): add new feature"
git push origin feature-branch

# Create PR for review
```

### **Phase 2: Local Machine Testing**

#### **2.1 Sync Changes**

```bash
# On local machine (Mac/Windows)
git pull origin feature-branch
```

#### **2.2 Local Build & Test**

```bash
# Build on local machine
mkdir -p build && cd build
cmake .. -G Ninja
ninja -k0

# Test local-specific behavior
./phoenix_app
```

#### **2.3 Platform-Specific Testing**

- ✅ **macOS**: Dock icons, menu bar behavior, native look
- ✅ **Windows**: Taskbar icons, window management
- ✅ **Linux**: Desktop integration, theme support

---

## 🔧 **Tailscale Integration**

### **Connection Setup**

```bash
# Install Tailscale on dev-01 (already configured)
# Install Tailscale on local machine
# Join same Tailscale network

# Connect via Tailscale IP
ssh -i ~/.ssh/github_phoenix mark@100.97.54.75
```

### **Benefits of Tailscale**

- ✅ **Direct connection** - No NAT issues
- ✅ **Secure** - Encrypted tunnel
- ✅ **Reliable** - No port forwarding needed
- ✅ **Fast** - Low latency connection
- ✅ **Persistent** - Always available

---

## 📋 **Development Standards**

### **Code Quality Requirements**

- ✅ **All code changes on dev-01 first**
- ✅ **Build and test on dev-01**
- ✅ **Commit with descriptive messages**
- ✅ **Create PR for review**
- ✅ **Test on local machine after sync**

### **Testing Requirements**

- ✅ **Build success on dev-01**
- ✅ **GUI testing with Xvfb (Phoenix)**
- ✅ **Automated tests pass**
- ✅ **Resource loading verified**
- ✅ **Local platform testing**

### **Resource Management**

- ✅ **Test resource embedding on dev-01**
- ✅ **Verify QRC files work**
- ✅ **Test Font Awesome loading**
- ✅ **Validate icon paths**

---

## 🚫 **Anti-Patterns to Avoid**

### **❌ Direct Local Development**

```bash
# DON'T DO THIS
# Making changes directly on local machine
# Without testing on dev-01 first
```

### **❌ Skip Testing**

```bash
# DON'T DO THIS
# Committing without testing on dev-01
# Pushing untested changes
```

### **❌ Resource Path Issues**

```bash
# DON'T DO THIS
# Hardcoding resource paths
# Not testing resource embedding
```

---

## 🔄 **Workflow Examples**

### **Example 1: Adding New UI Component**

#### **Step 1: Development on dev-01**

```bash
# Connect to dev-01
ssh -i ~/.ssh/github_phoenix mark@100.97.54.75
cd /home/ec2-user/workspace/phoenix

# Create new component
mkdir -p src/ui/components
# Edit component files
# Test build
mkdir -p build && cd build
cmake .. -G Ninja
ninja -k0

# Test GUI
xvfb-run -a ./phoenix_app
```

#### **Step 2: Commit & Push**

```bash
git add .
git commit -m "feat(ui): add new component"
git push origin feature-branch
```

#### **Step 3: Local Testing**

```bash
# On local machine
git pull origin feature-branch
# Build and test locally
# Verify platform-specific behavior
```

### **Example 2: Fixing Resource Loading**

#### **Step 1: Debug on dev-01**

```bash
# Connect to dev-01
ssh -i ~/.ssh/github_phoenix mark@100.97.54.75
cd /home/ec2-user/workspace/phoenix

# Debug resource loading
# Test with Xvfb
# Fix resource paths
# Verify embedding works
```

#### **Step 2: Test & Commit**

```bash
# Test build and GUI
# Commit fixes
git add .
git commit -m "fix(resources): correct icon paths"
git push origin feature-branch
```

#### **Step 3: Local Verification**

```bash
# On local machine
git pull origin feature-branch
# Build and test
# Verify icons load correctly
```

---

## 📊 **Quality Metrics**

### **Success Criteria**

- ✅ **Build success on dev-01**
- ✅ **GUI testing with Xvfb**
- ✅ **Resource loading verified**
- ✅ **Local platform testing**
- ✅ **Code review approved**
- ✅ **CI/CD pipeline passes**
- ✅ **CI workflow protection active**

### **CI Workflow Protection (2025-01-23)**

- ✅ **Conflict marker detection** - Prevents merge conflicts in workflows
- ✅ **YAML linting** - Validates workflow syntax
- ✅ **CODEOWNERS protection** - Requires maintainer review for workflow changes
- ✅ **Weekly self-audit** - Automated health checks every Monday
- ✅ **Pre-commit hooks** - Prevents conflict markers from being committed
- ✅ **Single Gate job** - Simplified branch protection logic

### **Failure Scenarios**

- ❌ **Build fails on dev-01** → Fix on dev-01
- ❌ **Resource loading fails** → Debug on dev-01
- ❌ **Local build fails** → Sync latest changes
- ❌ **Platform-specific issues** → Test on local machine
- ❌ **CI workflow conflicts** → Resolve merge conflicts in `.github/workflows/`

---

## 🛠 **Tools & Commands**

### **Dev-01 Development**

```bash
# Connect to dev-01
ssh -i ~/.ssh/github_phoenix mark@100.97.54.75

# Build project
cd /home/ec2-user/workspace/phoenix
mkdir -p build && cd build
cmake .. -G Ninja
ninja -k0

# Test GUI with Xvfb
xvfb-run -a ./phoenix_app

# Capture screenshots
xvfb-run -a ./phoenix_app &
sleep 5
import -window root screenshot.png
```

### **Local Machine Testing**

```bash
# Sync changes
git pull origin feature-branch

# Build locally
mkdir -p build && cd build
cmake .. -G Ninja
ninja -k0

# Test locally
./phoenix_app
```

---

## 🛡️ **CI Workflow Protection (2025-01-23)**

### **Self-Healing CI System**

The CI workflow now includes comprehensive protection against common issues:

#### **Conflict Prevention**

- **Pre-commit hook** - Prevents conflict markers from being committed
- **Conflict marker guard** - Fails CI loudly if conflicts detected in workflows
- **YAML linting** - Validates workflow syntax before execution

#### **Access Control**

- **CODEOWNERS** - Requires maintainer review for workflow changes
- **Single Gate job** - Simplified branch protection logic
- **Manual triggers** - `workflow_dispatch` for debugging

#### **Automated Monitoring**

- **Weekly self-audit** - Runs every Monday at 09:00 UTC
- **Status badges** - Visible CI health indicators in README
- **Smart job execution** - Docs vs code change detection

#### **Protection Layers**

| Layer | Purpose | Status |
|-------|---------|--------|
| Pre-commit hook | Stops conflict markers before GitHub | ✅ Active |
| Conflict guard | Detects conflicts and halts CI | ✅ Active |
| YAML linting | Validates workflow syntax | ✅ Active |
| CODEOWNERS | Requires maintainer review | ✅ Active |
| Weekly audit | Automated health checks | ✅ Active |

### **CI Workflow Features**

- **Smart change detection** - Uses `dorny/paths-filter` for docs vs code
- **Lightweight docs checks** - markdownlint, link checker, spell check
- **Conditional job execution** - Build only for code changes, docs only for documentation
- **Single required status** - Gate job prevents deadlocks
- **Manual debugging** - `workflow_dispatch` trigger available

---

## 📚 **Documentation Updates**

### **Updated Documents**

- ✅ **DEVELOPMENT_WORKFLOW.md** - This document
- ✅ **CODING_STANDARDS.md** - Updated with dev-01 policy
- ✅ **dev-setup.md** - Updated with Tailscale approach

### **Key Changes**

- ✅ **All development on dev-01 first**
- ✅ **Tailscale for secure connection**
- ✅ **Xvfb for GUI testing**
- ✅ **Resource debugging on dev-01**
- ✅ **Local testing after sync**

---

## 🎯 **Success Metrics**

### **Development Efficiency**

- ✅ **Faster iteration** - No SSH overhead for every change
- ✅ **Better debugging** - Can test GUI with Xvfb on dev-01
- ✅ **Consistent builds** - Same environment for all developers
- ✅ **Quality assurance** - Automated testing on dev-01

### **Code Quality**

- ✅ **All changes tested** - Build and GUI testing on dev-01
- ✅ **Resource management** - Proper embedding and loading
- ✅ **Platform compatibility** - Tested on multiple platforms
- ✅ **Version control** - All changes tracked in Git

---

**Maintained By:** Engineering Team  
**Questions:** See `.underlord/docs/GOTCHAS.md` for common issues  
**Last Updated:** 2025-01-21
