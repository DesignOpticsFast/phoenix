# CI dev-01 Runner Configuration

**Purpose**: Configure dev-01 as the dedicated Qt build runner for Phoenix CI/CD pipeline.

---

## **Required Runner Labels**

The dev-01 runner must be configured with these labels:

- `self-hosted` (automatic)
- `linux` (OS specification)
- `x64` (architecture specification)
- `dev-01` (custom)
- `qt6` (custom)

### **Applying Labels**

**For new runner setup:**

```bash
cd ~/actions-runner
./config.sh --url https://github.com/DesignOpticsFast/phoenix \
            --token <RUNNER_TOKEN> \
            --labels "linux,x64,dev-01,qt6" \
            --name "dev-01"
sudo ./svc.sh install
sudo ./svc.sh start
```

**For existing runner:**

```bash
cd ~/actions-runner
sudo ./svc.sh stop
./config.sh --replace --labels "linux,x64,dev-01,qt6"
sudo ./svc.sh start
```

---

## **Required Environment Variables**

### **Method 1: Profile Script (Login Shells)**

Create `/etc/profile.d/qt.sh` on dev-01:

```bash
# /etc/profile.d/qt.sh
export QT_VERSION=6.9.3
export QT_PATH=/opt/Qt/${QT_VERSION}/gcc_64
export CMAKE_PREFIX_PATH=$QT_PATH
export PATH=$QT_PATH/bin:$PATH
```

### **Method 2: Systemd Service Environment (Recommended)**

The Actions runner is a systemd service, so we need to ensure environment variables are loaded by the service:

```bash
# Create systemd drop-in for the runner service
sudo systemctl edit actions.runner-*.service
```

**Paste this configuration:**

```ini
[Service]
Environment=QT_VERSION=6.9.3
Environment=QT_PATH=/opt/Qt/6.9.3/gcc_64
Environment=CMAKE_PREFIX_PATH=/opt/Qt/6.9.3/gcc_64
Environment=PATH=/opt/Qt/6.9.3/gcc_64/bin:%h/actions-runner:%h/actions-runner/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin
```

**Apply the changes:**

```bash
# Reload systemd configuration
sudo systemctl daemon-reload

# Restart the runner service
sudo systemctl restart actions.runner-*.service
```

**Apply both methods for maximum compatibility:**

```bash
# Method 1: Profile script
sudo tee /etc/profile.d/qt.sh << 'EOF'
export QT_VERSION=6.9.3
export QT_PATH=/opt/Qt/${QT_VERSION}/gcc_64
export CMAKE_PREFIX_PATH=$QT_PATH
export PATH=$QT_PATH/bin:$PATH
EOF
sudo chmod +x /etc/profile.d/qt.sh

# Method 2: Systemd service environment
sudo systemctl edit actions.runner-*.service
# (paste the [Service] configuration above)

# Apply both
sudo systemctl daemon-reload
sudo systemctl restart actions.runner-*.service
```

---

## **Verification Steps**

### **1. Test Environment Locally**

```bash
# Source environment
source /etc/profile.d/qt.sh

# Verify variables
echo "QT_VERSION=$QT_VERSION"
echo "CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH"

# Test Qt installation
test -f $CMAKE_PREFIX_PATH/lib/cmake/Qt6/Qt6Config.cmake && echo "Qt OK" || echo "Qt MISSING"
```

### **2. One-Time Verification on dev-01**

**Copy/paste this complete verification script:**

```bash
#!/bin/bash
echo "=== dev-01 Runner Verification ==="

# 1. Check runner service status
echo "1. Runner service status:"
cd ~/actions-runner
sudo ./svc.sh status || systemctl status actions.runner* --no-pager

# 2. Check environment variables as seen by the service
echo -e "\n2. Environment variables:"
env | egrep 'QT_|CMAKE_PREFIX_PATH|PATH'

# 3. Verify Qt installation
echo -e "\n3. Qt installation check:"
test -f $CMAKE_PREFIX_PATH/lib/cmake/Qt6/Qt6Config.cmake && echo "✅ Qt OK" || echo "❌ Qt MISSING"

# 4. System resources
echo -e "\n4. System resources:"
echo "CPU cores: $(nproc)"
echo "Disk space:"
df -h | sed -n '1p;/\/opt\|\/home/p'

echo -e "\n=== Verification Complete ==="
```

### **2. Verify from CI Workflow**

The CI workflow includes an environment verification step that will:

- Display runner labels
- Show environment variables
- Probe Qt installation
- Fail with clear error if Qt is missing

---

## **CI Workflow Behavior**

### **Build Job Conditions**

- **Internal PRs with code changes**: Build runs on dev-01
- **Docs-only PRs**: Build skipped, docs checks run
- **CI-only PRs**: Build skipped, ci_checks run
- **Fork PRs**: Build skipped (safety), docs/ci_checks run

### **Safety Features**

- **Fork Protection**: `head.repo.full_name == github.repository`
- **Timeout**: 30 minutes maximum build time
- **Concurrency**: Serialized builds on dev-01 (`dev01-build` group)
- **Environment Probe**: Clear error messages if Qt is missing

---

## **Monitoring**

### **Daily Heartbeat**

- **Schedule**: Daily at 09:00 UTC
- **Purpose**: Verify dev-01 is online and Qt is available
- **Manual Trigger**: Available via `workflow_dispatch`

### **Health Check**

The heartbeat workflow verifies:

- Runner is accessible
- Environment variables are set
- Qt installation is present

---

## **Qt Version Management**

### **Current Configuration**

- **Version**: 6.9.3
- **Path**: `/opt/Qt/6.9.3/gcc_64`
- **Configurable**: Via `QT_VERSION` environment variable

### **Upgrading Qt**

1. Install new Qt version on dev-01
2. Update `QT_VERSION` in `/etc/profile.d/qt.sh`
3. Restart runner service
4. Test with heartbeat workflow

### **Multiple Versions (Future)**

For multiple Qt versions:

```bash
# Maintain multiple installs
/opt/Qt/6.9.3/gcc_64
/opt/Qt/6.10.0/gcc_64

# Switch via environment
export QT_VERSION=6.10.0
```

---

## **Troubleshooting**

### **Common Issues**

**1. Runner not found**

- Verify labels: `self-hosted`, `dev-01`, `qt6`
- Check runner service status: `sudo systemctl status actions.runner.*`

**2. Qt not found**

- Verify environment variables are set
- Check Qt installation path
- Test with: `test -f $CMAKE_PREFIX_PATH/lib/cmake/Qt6/Qt6Config.cmake`

**3. Build timeouts**

- Check dev-01 resource usage
- Verify no hanging processes
- Review build logs for specific errors

### **Rollback Plan (Fast)**

If dev-01 misbehaves, here's the quick rollback procedure:

**Step 1: Stop dev-01 service**

```bash
# On dev-01
cd ~/actions-runner
sudo ./svc.sh stop
```

**Step 2: Temporary fallback to GitHub runners**
Update the build job in `ci.yml`:

```yaml
build:
  name: Build (Qt on GitHub runners - temporary)
  needs: changes
  runs-on: ubuntu-latest
  if: >
    (github.event_name == 'push') ||
    (github.event_name == 'pull_request' &&
     github.event.pull_request.head.repo.full_name == github.repository &&
     needs.changes.outputs.code_changed == 'true')
  steps:
    - uses: actions/checkout@v4
    - name: Install Qt
      run: |
        python3 -m pip install --upgrade pip aqtinstall
        aqt install-qt linux desktop 6.9.3 gcc_64 -m qtgraphs
        echo "CMAKE_PREFIX_PATH=$PWD/6.9.3/gcc_64" >> $GITHUB_ENV
    - name: Configure
      run: cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH"
    - name: Build
      run: cmake --build build -j 4
    - name: Unit tests
      if: hashFiles('build/CTestTestfile.cmake') != ''
      run: ctest --test-dir build --output-on-failure
```

**Step 3: Revert once dev-01 is fixed**

```bash
# Revert the temporary changes
git revert <commit-sha>
# OR restore the original dev-01 configuration
```

**Benefits of this approach:**

- ✅ Repository remains mergeable
- ✅ Builds continue on GitHub runners
- ✅ No data loss or configuration changes
- ✅ Easy to revert when dev-01 is fixed

---

## **Success Criteria**

- ✅ Internal code PRs build on dev-01
- ✅ Docs-only PRs skip build, run docs checks
- ✅ CI-only PRs skip build, run ci_checks
- ✅ Fork PRs never touch dev-01
- ✅ Daily heartbeat shows runner health
- ✅ Clear error messages when Qt is missing

## **Final Validation Checklist**

After implementing the dev-01 configuration, run these validation tests:

### **✅ Test Scenarios**

1. **Internal code PR** → Build runs on dev-01, passes
2. **Docs PR** → Docs checks only, gate passes  
3. **Workflow PR (Dependabot)** → ci_checks only, gate passes
4. **Fork PR** → No dev-01 usage, gate passes
5. **Scheduled audit** → ubuntu-latest only, passes
6. **dev-01 heartbeat** (manual or daily) shows online

### **✅ Expected CI Behavior**

**In a PR build log, the "Verify dev-01 environment" step should print:**

- Runner labels containing: `linux`, `x64`, `dev-01`, `qt6`
- `CMAKE_PREFIX_PATH` → `/opt/Qt/6.9.3/gcc_64`
- `"Qt probe OK…"` message

### **✅ Troubleshooting**

**If any build still misses dev-01:**

- Check the "Verify labels" line in the build log
- Check the evaluated `if:` condition from the skipped job
- Verify runner labels match exactly: `[self-hosted, linux, x64, dev-01, qt6]`

---

**Maintained By**: Engineering Team  
**Last Updated**: 2025-01-23  
**Questions**: See `.underlord/docs/GOTCHAS.md` for common issues
