# CI dev-01 Runner Configuration

**Purpose**: Configure dev-01 as the dedicated Qt build runner for Phoenix CI/CD pipeline.

---

## **Required Runner Labels**

The dev-01 runner must be configured with these labels:
- `self-hosted` (automatic)
- `dev-01` (custom)
- `qt6` (custom)

### **Applying Labels**

**For new runner setup:**
```bash
cd ~/actions-runner
./config.sh --url https://github.com/DesignOpticsFast/phoenix \
            --token <RUNNER_TOKEN> \
            --labels "dev-01,qt6" \
            --name "dev-01"
sudo ./svc.sh install
sudo ./svc.sh start
```

**For existing runner:**
```bash
cd ~/actions-runner
sudo ./svc.sh stop
./config.sh --replace --labels "dev-01,qt6"
sudo ./svc.sh start
```

---

## **Required Environment Variables**

Create `/etc/profile.d/qt.sh` on dev-01:

```bash
# /etc/profile.d/qt.sh
export QT_VERSION=6.9.3
export QT_PATH=/opt/Qt/${QT_VERSION}/gcc_64
export CMAKE_PREFIX_PATH=$QT_PATH
export PATH=$QT_PATH/bin:$PATH
```

**Apply the environment:**
```bash
# Make executable
sudo chmod +x /etc/profile.d/qt.sh

# Restart runner service
sudo systemctl restart actions.runner.*
# OR
cd ~/actions-runner && sudo ./svc.sh stop && sudo ./svc.sh start
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

### **Rollback Plan**
If dev-01 becomes problematic:
1. Comment out build job in `ci.yml`
2. Revert to last known good commit
3. Keep docs/ci_checks/gate unchanged
4. Repository remains mergeable

---

## **Success Criteria**

- ✅ Internal code PRs build on dev-01
- ✅ Docs-only PRs skip build, run docs checks
- ✅ CI-only PRs skip build, run ci_checks
- ✅ Fork PRs never touch dev-01
- ✅ Daily heartbeat shows runner health
- ✅ Clear error messages when Qt is missing

---

**Maintained By**: Engineering Team  
**Last Updated**: 2025-01-23  
**Questions**: See `.underlord/docs/GOTCHAS.md` for common issues
