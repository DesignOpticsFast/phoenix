> **NOTE — Phoenix Implementation Scope**
>
> This control document governs the Overnight Autonomous QA Framework implementation
> in the **Phoenix repository**. Scripts and reports are located under:
>
> - Scripts: `phoenix/scripts/overnight/`
> - Reports: `phoenix/qa_reports_*`
> - Documentation: `phoenix/docs/qa/`
>
> This is a Phoenix-scoped implementation, not a global workspace-scoped system.

---

**Work Package:** Overnight Autonomous QA Framework  

**Scope:** Sprint-agnostic (use anytime QA analysis needed)  

**Status:** Ready for Implementation  

**Approved By:** Mark, Claude, Lyra

---

## **1. Intent**

Implement the Overnight Autonomous QA Framework for the Bedrock/Phoenix stack, enabling 6-8 hours of read-only QA analysis during periods when I'm unavailable (evenings, overnight, weekends).

The framework must work on both **Crucible** (macOS) and **Fulcrum** (Linux) without code changes.

**Reference Specification:** `phoenix/docs/qa/OVERNIGHT_QA_CONTROL.md`

---

## **2. Context**

### **2.1 Machines**

| **Machine** | **OS** | **Strengths** |

| --- | --- | --- |

| **Crucible** | macOS | Fast GUI dev box |

| **Fulcrum** | Linux | Beast mode - ideal for heavy sanitizer/Valgrind runs |

### **2.2 Safety Constraints (CRITICAL)**

**This framework is READ-ONLY. It must NEVER:**

- ❌ Commit or push anything

- ❌ Modify source code

- ❌ Act on findings automatically

- ❌ Touch CI configuration

- ❌ Install dependencies

**It ONLY:**

- ✅ Runs tests and sanitizers

- ✅ Generates reports to `phoenix/qa_reports_*` directories

---

## **3. Design Constraints (MUST FOLLOW)**

### **3.1 Shell Contract**

```
#!/bin/bash

set +e  # CRITICAL: Never use set -e

```

**Rationale:** One failing tool must not kill the entire 8-hour run.

### **3.2 Iteration Precedence**

All iteration-based tasks (soak, TSAN, ASAN) must follow this precedence:

```
1. Environment variable (e.g., SOAK_ITERATIONS=10)

2. Positional argument ($1)

3. Default value

```

**Defaults:**

- Soak: 500 iterations

- TSAN: 100 iterations

- ASAN: 50 iterations

**Example implementation:**

```
ITERATIONS=${SOAK_ITERATIONS:-${1:-500}}

```

### **3.3 Tool Availability**

**Rule:** Missing tools cause task SKIP, not run FAIL.

Each script must:

```
if ! command -v valgrind &> /dev/null; then

    echo "Valgrind not installed - task skipped" > "$OUTPUT_DIR/summary_${TIMESTAMP}.txt"

    exit 0  # Exit success so run continues

fi

```

Task status for missing tools: `⚠️ SKIPPED`

### **3.4 Shell Portability (POSIX)**

**Scripts must run on both macOS (BSD) and Linux (GNU).**

**Forbidden:**

- `grep -P` (Perl regex - GNU only)

- `grep -oP` (GNU only)

- GNU-only sed/awk flags

**Preferred:**

- Use `awk` for parsing and arithmetic

- Use POSIX grep patterns

- Test critical parsing on both platforms

**Example - extracting numbers:**

```
# Bad (GNU only):

grep -oP 'lost: \K[0-9,]+' file

# Good (POSIX):

grep 'lost:' file | awk -F': ' '{print $2}' | awk '{print $1}'

```

### **3.5 Platform-Specific Handling**

**Valgrind on macOS:**

```
if [[ "$OSTYPE" == "darwin"* ]]; then

    echo "Valgrind not available on macOS - task skipped"

    echo "Status: ⚠️ SKIPPED" >> "$SUMMARY_FILE"

    exit 0

fi

```

**Health monitoring:**

- Linux: Use `/proc/loadavg`, `/proc/meminfo`, `/sys/class/thermal/*`

- macOS: Use `sysctl`, `vm_stat`, degrade gracefully if sensors missing

### **3.6 Task Status Values**

| **Status** | **When to Use** |

| --- | --- |

| ✅ PASS | Task ran, all criteria met |

| 🔴 FAIL | Task ran, criteria NOT met |

| ⚠️ SKIPPED | Task skipped (tool missing, platform unsupported) |

---

## **4. Ask**

### **4.1 Phase 1: Design & Planning (STOP FOR APPROVAL)**

**Before writing any code:**

### **A. Propose file layout**

```
phoenix/scripts/overnight/

├── run_overnight_qa.sh

├── soak_test.sh

├── tsan_stress.sh

├── asan_stress.sh

├── valgrind_check.sh

├── coverage_report.sh

└── static_analysis.sh

phoenix/docs/qa/

├── OVERNIGHT_QA_CONTROL.md

└── OVERNIGHT_QA.md

```

### **B. Document tool requirements**

List which tools each script needs and how it handles missing tools.

### **C. Document OS-specific handling**

Show how each script detects and handles platform differences.

**🛑 STOP.** Show me your plan before proceeding.

---

### **4.2 Phase 2: Implementation**

Once approved, implement:

### **Task A: Master Orchestrator (`phoenix/scripts/overnight/run_overnight_qa.sh`)**

**Variables:**

- `SCRIPT_DIR` - Script location

- `REPORT_DIR` - `phoenix/qa_reports_YYYYMMDD_HHMMSS`

- `HOSTNAME` - Machine name (for reports)

**Functions:**

`check_system_health()`:

```
Linux:

- CPU temp: /sys/class/thermal/* (>85°C = warning)

- Load: /proc/loadavg (>2× nproc = warning)

- Memory: /proc/meminfo (<2GB = warning)

- Disk: df (>90% = warning)

macOS:

- Skip temp if no tool available

- Use sysctl/vm_stat for load/memory

- Disk: df

```

`adaptive_pause()`:

- Check health before each task

- If unhealthy: sleep 60s, retry (max 5)

- After 5 failures: log warning, continue anyway

- **NEVER abort the run**

`run_task(name, script, args)`:

- Log start with timestamp

- Invoke script, capture exit code

- Record `TASK_STATUS[name]` and `TASK_DURATION[name]`

- Sleep 5-10s between tasks

**Task Sequence:**

```
Phase 1 - Critical:

  tsan_stress.sh

  asan_stress.sh

  valgrind_check.sh

Phase 2 - Stability:

  soak_test.sh

Phase 3 - Quality:

  coverage_report.sh

  static_analysis.sh

```

**Summary (`OVERNIGHT_SUMMARY.md`):**

- Date, runtime, **hostname**

- Status table: task, status, duration

- System health events

- Next steps (based on failures)

---

### **Task B: Subordinate Scripts**

**Common pattern for all:**

```
#!/bin/bash

set +e

OUTPUT_DIR="${1:-.}"

TIMESTAMP=$(date +%Y%m%d_%H%M%S)

mkdir -p "$OUTPUT_DIR"

# Check required tools

if ! command -v TOOL &> /dev/null; then

    cat > "$OUTPUT_DIR/summary_${TIMESTAMP}.txt" << EOF

TASK SUMMARY

============

Status: ⚠️ SKIPPED

Reason: TOOL not installed

EOF

    exit 0

fi

# ... task logic ...

# Write summary

cat > "$OUTPUT_DIR/summary_${TIMESTAMP}.txt" << EOF

TASK SUMMARY

============

Status: $([ $ERRORS -eq 0 ] && echo "✅ PASS" || echo "🔴 FAIL")

...

EOF

exit $([ $ERRORS -eq 0 ] && echo 0 || echo 1)

```

**soak_test.sh:**

- Iterations: `${SOAK_ITERATIONS:-${1:-500}}`

- Use `-gtest_shuffle --gtest_random_seed=$i`

- Track pass/fail counts

- Exit non-zero if any failures

**tsan_stress.sh:**

- Iterations: `${TSAN_ITERATIONS:-${1:-100}}`

- Requires `build_tsan` binary

- Grep for ThreadSanitizer warnings (POSIX grep!)

- De-duplicate races to `unique_races_*.txt`

- Exit non-zero if races found

**asan_stress.sh:**

- Iterations: `${ASAN_ITERATIONS:-${1:-50}}`

- Requires `build_asan` binary

- Grep for AddressSanitizer errors

- Exit non-zero if errors found

**valgrind_check.sh:**

- **Skip on macOS** with ⚠️ SKIPPED status

- Parse leak summary with POSIX-compatible parsing

- Exit non-zero if definitely_lost > 0

**coverage_report.sh:**

- Check for lcov/genhtml

- Clear `.gcda`, run tests, generate report

- Extract coverage percentages

- Exit non-zero if target coverage not met

**static_analysis.sh:**

- Check for cppcheck (required)

- Check for clang-tidy (optional)

- Run available tools

- Exit non-zero if error-level issues found

---

### **Task C: Documentation**

Create `phoenix/docs/qa/OVERNIGHT_QA.md`:

```
# Overnight QA Quick Start

## Start a Run

\`\`\`bash

nohup ./phoenix/scripts/overnight/run_overnight_qa.sh > overnight_$(date +%Y%m%d).log 2>&1 &

\`\`\`

## Smoke Test

\`\`\`bash

SOAK_ITERATIONS=10 TSAN_ITERATIONS=5 ASAN_ITERATIONS=5 ./phoenix/scripts/overnight/run_overnight_qa.sh

\`\`\`

## Find Results

\`\`\`bash

cat $(ls -td phoenix/qa_reports_* | head -1)/OVERNIGHT_SUMMARY.md

\`\`\`

```

---

### **4.3 Phase 3: Verification**

### **A. Smoke Test**

Run with minimal iterations:

```
SOAK_ITERATIONS=10 TSAN_ITERATIONS=5 ASAN_ITERATIONS=5 \

  ./phoenix/scripts/overnight/run_overnight_qa.sh

```

Verify:

- All scripts run or skip gracefully

- `OVERNIGHT_SUMMARY.md` generated

- Hostname appears in summary

- Status values are correct (✅/🔴/⚠️)

### **B. Platform Test**

If possible, run smoke test on both Fulcrum and Crucible.

Verify:

- Valgrind runs on Fulcrum, skips on Crucible

- No GNU-only parsing errors on macOS

- Health checks work on both platforms

### **C. Report Back**

Include in your summary:

- File layout implemented

- Any deviations from spec

- Tool requirements

- Sample `OVERNIGHT_SUMMARY.md` output

---

## **5. Definition of Done**

### **Scripts**

- [ ]  `phoenix/scripts/overnight/run_overnight_qa.sh` exists and runs

- [ ]  All 6 subordinate scripts exist and are executable

- [ ]  Smoke test produces `phoenix/qa_reports_*` with `OVERNIGHT_SUMMARY.md`

### **Design Compliance**

- [ ]  Uses `set +e` (never `set -e`)

- [ ]  Iteration precedence: env > arg > default

- [ ]  Missing tools → ⚠️ SKIPPED (not failure)

- [ ]  POSIX-compatible (no GNU-only grep/sed/awk)

- [ ]  Valgrind skips on macOS with ⚠️ SKIPPED

### **Output Quality**

- [ ]  Hostname in summary (Crucible vs Fulcrum)

- [ ]  Task durations tracked

- [ ]  System health events logged

- [ ]  Clear status per task (✅/🔴/⚠️)

### **Documentation**

- [ ]  `phoenix/docs/qa/OVERNIGHT_QA.md` exists

- [ ]  `phoenix/docs/qa/OVERNIGHT_QA_CONTROL.md` exists

### **Safety**

- [ ]  No CI configs modified

- [ ]  No code auto-fixed

- [ ]  All operations read-only

---

## **6. Quick Reference**

### **Iteration Defaults**

| **Task** | **Default** | **Env Var** |

| --- | --- | --- |

| Soak | 500 | `SOAK_ITERATIONS` |

| TSAN | 100 | `TSAN_ITERATIONS` |

| ASAN | 50 | `ASAN_ITERATIONS` |

### **Task Status**

| **Status** | **Meaning** |

| --- | --- |

| ✅ PASS | Ran, criteria met |

| 🔴 FAIL | Ran, criteria NOT met |

| ⚠️ SKIPPED | Skipped (tool/platform) |

### **Build Directories**

```
build/          # Release

build_tsan/     # -fsanitize=thread

build_asan/     # -fsanitize=address

build_coverage/ # --coverage

```

### **POSIX Portability Reminders**

```
# Bad (GNU):

grep -oP 'pattern'

grep -P 'perl-regex'

# Good (POSIX):

grep 'pattern' | awk '{print $1}'

grep -E 'extended-regex'

```

