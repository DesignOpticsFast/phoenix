# Sprint Completion Checklist

Use this checklist before marking any infrastructure-impacted sprint as "complete."

## For Infrastructure Sprints (Sprint 3A, etc.)

- [ ] `.underlord/preflight.sh` **PASS** in phoenix (exit 0)
- [ ] `.underlord/preflight.sh` **PASS** in bedrock (exit 0)
- [ ] Logs stored in `.underlord/logs/` for both runs
- [ ] `METRICS.md` updated with timestamped entries
- [ ] Warnings triaged: fixed or documented with owner/date
- [ ] Environment variables added to `~/.bashrc` if needed
- [ ] Third-party dependencies documented (versions, install paths)

## For Feature/Bug Sprints (Sprint 1, 2, etc.)

- [ ] All acceptance criteria met
- [ ] Tests written and passing
- [ ] Code reviewed (if applicable)
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
- [ ] Sprint metrics recorded in METRICS.md

## General (All Sprints)

- [ ] Git status clean (or uncommitted changes documented)
- [ ] Sprint close script executed successfully
- [ ] Handoff document created (if needed)
- [ ] Next sprint identified and documented

## Notes

**Cannot certify infrastructure without preflight logs.** This is a hard requirement.

If preflight fails, the sprint is not complete. Address failures before closing.
