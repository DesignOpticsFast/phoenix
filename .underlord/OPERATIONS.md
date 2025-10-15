# UnderLord Operations Guide

**Quick-start guide for daily development workflow**

---

## 🌅 Morning Routine

```bash
cd /home/ec2-user/{repo}-check
.underlord/startup.sh
```

This runs preflight checks and shows current sprint status.

---

## 🔧 Pre-Flight Checks

**Before any work:**
```bash
.underlord/preflight.sh
```

Validates environment, toolchain, git status, and GitHub auth.

---

## 📋 Sprint Lifecycle

### Start New Sprint
```bash
.underlord/sprint-init.sh <number> <slug>

# Example:
.underlord/sprint-init.sh 002 lens-calculator
```

### Check Progress
```bash
.underlord/sprint-status.sh
```

### Close Sprint
```bash
.underlord/sprint-close.sh
```

---

## 🚦 Workflow

1. **Pre-flight** → `.underlord/preflight.sh`
2. **Branch** → `sprint-{number}-{slug}`
3. **Build** → Out-of-source in `build/`
4. **Test** → After every change
5. **Commit** → Conventional format
6. **PR** → `gh pr create`
7. **CI** → `gh pr checks --watch`
8. **Merge** → Squash after approval

---

## 📚 Key Documents

| Document | Purpose |
|----------|---------|
| `OPERATIONS.md` | This guide |
| `PROJECT_OVERVIEW.md` | Architecture |
| `DEPENDENCIES.md` | Build requirements |
| `GOTCHAS.md` | Known issues |
| `SPRINT_HISTORY.md` | Past work |

---

## 🆘 Common Issues

See `GOTCHAS.md` for detailed troubleshooting.

---

**Last Updated:** 2025-10-14
