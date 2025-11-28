# CI dev-01 Runner Configuration

> **⚠️ DEPRECATED: This document is obsolete. dev-01 has been permanently removed from CI infrastructure.**
>
> **All CI now runs on GitHub Actions. See `.github/workflows/ci.yml` for current CI configuration.**
>
> This document is kept for historical reference only.

---

## Historical Context

This document previously described the configuration of dev-01 as a self-hosted GitHub Actions runner for Phoenix CI/CD. As of the dev-01 removal migration, all CI tasks have been moved to GitHub Actions hosted runners.

## Current CI Configuration

- **Platform**: GitHub Actions (ubuntu-latest, macos-latest)
- **Qt Version**: 6.10.1 (installed via `jurplel/install-qt-action@v3`)
- **Build System**: CMake with Ninja
- **Testing**: Unit tests and integration tests run automatically

See `.github/workflows/ci.yml` for the current CI workflow configuration.
