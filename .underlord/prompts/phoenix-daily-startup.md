### UnderLord Daily Startup / Preflight Ritual

**Tasks:**

1) Documentation Review + Policy Diff
   - Scan: docs/**/*.md, .underlord/**/*.md
   - Compute SHA256 over concatenated contents.
   - Compare to cache at .underlord/state/docs.sha256.
   - If changed: output a concise “policy changes detected” summary (list changed files + first changed headings).
   - If unchanged: print “No documentation changes since last run.”

2) Environment Verification
   - Source .underlord/env.d/20-qt6.sh
   - Print:
     [env] cmake=$(which cmake) ; $(cmake --version | head -1)
     [env] Qt6_DIR=$Qt6_DIR
     [env] CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH
     [env] git branch=$(git rev-parse --abbrev-ref HEAD) commit=$(git rev-parse --short HEAD)
   - Assert Qt6_DIR == /opt/Qt/6.10.0/gcc_64/lib/cmake/Qt6
   - Assert CMAKE_PREFIX_PATH starts with /opt/Qt/6.10.0/gcc_64
   - Warn (non-fatal) if working tree is dirty: git diff --quiet || echo “[env] WARNING: uncommitted changes present”

3) Smoke + Preflight
   - Run scripts/dev/qt6_smoke.sh /tmp/qt-smoke (non-blocking; prints OK/FAIL)
   - Run .underlord/preflight.sh (existing auto-retry with find-debug is kept)
   - Tee preflight configure output to /tmp/preflight.log

4) Stop-the-line Handling
   - If any red check (smoke hard error or preflight failure):
     - mkdir -p .underlord/logs/$(date +%Y%m%d-%H%M%S) and copy:
         /tmp/preflight.log (if exists)
         /tmp/qt-smoke/configure.log (if exists)
         build/CMakeCache.txt (if exists)
     - Append one-line entry to .underlord/logs/stop_the_line.log:
       "$(date -Iseconds) | $(git rev-parse --abbrev-ref HEAD)@$(git rev-parse --short HEAD) | PRELIGHT FAIL | logs=<captured-dir>"
     - Print: [STOP-THE-LINE] <same message> and exit 1

5) Success Summary
   - If all green, print single block:
     ✅ Phoenix Preflight Complete
     Qt6: 6.10.0 @ /opt/Qt/6.10.0/gcc_64
     CMake: <version>
     Build/Test: PASS
     Docs: <No changes | N files changed>
     Timestamp: $(date -Iseconds)

**Notes:**
- Respect quiet mode when env DAILY_QUIET=1: skip doc summary unless checksum changed.
- Idempotent: updating docs.sha256 only after successful doc scan.

