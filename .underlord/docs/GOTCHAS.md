# Known Issues & Gotchas

**Last Updated:** 2025-10-14

---

## 🚨 Critical Issues

### Directory Naming Convention
**Issue:** Working directories follow `{repo}-check` pattern

**Example:**
```bash
# ❌ Wrong
cd /home/ec2-user/bedrock

# ✅ Correct
cd /home/ec2-user/bedrock-check
```

**Verification:** Always run `pwd` before git operations

**Encountered:** 2025-10-11 (Phase 6)

---

### CodeQL Default vs Advanced Mode
**Issue:** Cannot have both default and advanced CodeQL enabled

**Resolution:**
1. Go to GitHub Settings → Security → CodeQL analysis
2. Click "Switch to advanced" to disable default
3. Keep only `.github/workflows/codeql.yml`

**Prevention:** Never enable default CodeQL via GitHub UI

**Encountered:** 2025-10-11 (Phase 6)

---

### Classic vs Ruleset Branch Protection
**Issue:** Classic rules can coexist with rulesets and cause conflicts

**Resolution:**
1. Navigate to Settings → Branches
2. Delete any classic branch protection rules
3. Use rulesets only (Settings → Rules → Rulesets)

**Prevention:** Never create classic protection rules

**Encountered:** 2025-10-11 (Phase 6)

---

## ⚠️ Configuration Issues

### Git Credentials Per-Repo
**Issue:** Must configure `credential.useHttpPath`

**Resolution:**
```bash
git config --global credential.useHttpPath true
gh auth setup-git
```

**Encountered:** 2025-10-11 (Phase 6)

---

### Check Context Discovery
**Issue:** GitHub UI doesn't show check contexts until they run

**Workaround:**
```bash
gh api /repos/{org}/{repo}/commits/{sha}/check-runs | \
  jq -r '.check_runs[].name'
```

**Encountered:** 2025-10-11 (Phase 6)

---

## 🔧 Build Issues

### Phoenix Requires Bedrock Sibling
**Issue:** Phoenix expects Bedrock in `../bedrock`

**Resolution:**
```bash
workspace/
├── bedrock/
└── phoenix/
```

**CI Handling:** CI checks out both repos automatically

**Encountered:** 2025-10-14 (Phase 7)

---

### Build Directory Pollution
**Issue:** Build artifacts can leak into git

**Resolution:**
```bash
rm -rf build/

# Verify .gitignore includes:
build/
*.o
*.a
```

**Encountered:** Ongoing

---

## 📦 CI/CD Issues

### CodeQL Configuration Differences
**Issue:** CodeQL workflows differ between Bedrock and Phoenix

**Status:** Active issue (2025-10-14)

**Investigation:** Compare `.github/workflows/codeql.yml`

**Workaround:** Align configurations across repos

---

## 🔐 Security Issues

### GitHub Token Expiration
**Issue:** PATs expire without automatic renewal

**Resolution:**
```bash
gh auth refresh -h github.com -s repo
gh auth setup-git
```

**Prevention:** Set calendar reminders

---

## 🔍 Debugging Tips

### When Build Fails Unexpectedly
1. Clean rebuild: `rm -rf build && cmake -S . -B build`
2. Check compiler version
3. Verify dependencies: `.underlord/preflight.sh`

### When CI Passes but Local Fails
1. Check environment differences
2. Verify dependency versions
3. Compare CMake configurations

### When PR Can't Merge
1. Check all required checks passed
2. Verify no conflicts
3. Check ruleset enforcement
4. Look for classic branch protection

---

**Contributing:** If you discover a new gotcha, add it here with:
- Clear description
- Symptoms to watch for
- Resolution steps
- Prevention strategy
- Date encountered
