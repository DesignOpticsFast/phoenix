#!/usr/bin/env bash
# Overnight Autonomous QA Framework - Main Orchestrator
# Reads-only: runs tests, sanitizers, coverage, static analysis
# Writes: logs and reports under qa_reports_*/

set +e  # CRITICAL: Never use set -e
set -u
set -o pipefail

# Platform detection
PLATFORM=$(uname -s | tr '[:upper:]' '[:lower:]')

# Report directory
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
REPORT_DIR="qa_reports_${TIMESTAMP}"
mkdir -p "$REPORT_DIR"

# Master log
MASTER_LOG="$REPORT_DIR/master.log"

# Task status tracking (simple variables, no associative arrays)
TSAN_STATUS=""
TSAN_DURATION=0
ASAN_STATUS=""
ASAN_DURATION=0
VALGRIND_STATUS=""
VALGRIND_DURATION=0
SOAK_STATUS=""
SOAK_DURATION=0
COVERAGE_STATUS=""
COVERAGE_DURATION=0
STATIC_STATUS=""
STATIC_DURATION=0

# Overall timing
OVERALL_START_TIME=$(date +%s)

# Initialize master log
{
  echo "=== Overnight QA Framework - Run Started ==="
  echo "Platform: $PLATFORM"
  echo "Hostname: $(hostname)"
  echo "Start Time: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "Report Directory: $REPORT_DIR"
  echo ""
} > "$MASTER_LOG"

# Health check function (POSIX-compatible)
check_system_health() {
  local healthy=0
  
  if [ "$PLATFORM" = "linux" ]; then
    # Linux: /proc and /sys
    # CPU temp (may not be available on all systems)
    if [ -d /sys/class/thermal ]; then
      local temp_file
      temp_file=$(find /sys/class/thermal -name "temp" -type f 2>/dev/null | head -1)
      if [ -n "$temp_file" ] && [ -r "$temp_file" ]; then
        local temp_mc=$(cat "$temp_file" 2>/dev/null || echo "0")
        local temp_c=$((temp_mc / 1000))
        if [ "$temp_c" -gt 80 ]; then
          echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [HEALTH] WARNING: CPU temp ${temp_c}°C exceeds threshold 80°C" >> "$MASTER_LOG"
          healthy=1
        fi
      fi
    fi
    
    # Load average
    if [ -r /proc/loadavg ]; then
      local load_1min
      load_1min=$(awk '{print $1}' /proc/loadavg)
      local cpu_count
      cpu_count=$(nproc 2>/dev/null || echo "1")
      local threshold
      threshold=$(awk -v c="$cpu_count" 'BEGIN{printf "%.1f", c * 2.0}')
      if [ "$(echo "$load_1min $threshold" | awk '{if ($1 > $2) print 1; else print 0}')" = "1" ]; then
        echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [HEALTH] WARNING: Load $load_1min exceeds threshold $threshold" >> "$MASTER_LOG"
        healthy=1
      fi
    fi
    
    # Memory (available vs total)
    if [ -r /proc/meminfo ]; then
      local mem_total mem_available
      mem_total=$(awk '/^MemTotal:/ {print $2}' /proc/meminfo)
      mem_available=$(awk '/^MemAvailable:/ {print $2}' /proc/meminfo)
      if [ -z "$mem_available" ]; then
        # Fallback: compute from MemFree + Buffers + Cached
        local mem_free buffers cached
        mem_free=$(awk '/^MemFree:/ {print $2}' /proc/meminfo)
        buffers=$(awk '/^Buffers:/ {print $2}' /proc/meminfo)
        cached=$(awk '/^Cached:/ {print $2}' /proc/meminfo)
        mem_available=$((mem_free + buffers + cached))
      fi
      if [ -n "$mem_total" ] && [ -n "$mem_available" ] && [ "$mem_total" -gt 0 ]; then
        local mem_percent
        mem_percent=$(awk -v avail="$mem_available" -v total="$mem_total" 'BEGIN{printf "%.1f", (avail/total)*100}')
        if [ "$(echo "$mem_percent 10.0" | awk '{if ($1 < $2) print 1; else print 0}')" = "1" ]; then
          echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [HEALTH] WARNING: Memory ${mem_percent}% free, below threshold 10%" >> "$MASTER_LOG"
          healthy=1
        fi
      fi
    fi
    
    # Disk space
    local disk_percent
    disk_percent=$(df -P / 2>/dev/null | awk 'NR==2 {print $5}' | sed 's/%//')
    if [ -n "$disk_percent" ] && [ "$disk_percent" -gt 95 ]; then
      echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [HEALTH] WARNING: Disk ${disk_percent}% used, above threshold 95%" >> "$MASTER_LOG"
      healthy=1
    fi
    
  elif [ "$PLATFORM" = "darwin" ]; then
    # macOS: sysctl, vm_stat, df
    # CPU temp (may not be available)
    local temp_level
    temp_level=$(sysctl -n machdep.xcpm.cpu_thermal_level 2>/dev/null || echo "")
    if [ -n "$temp_level" ] && [ "$temp_level" -gt 80 ]; then
      echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [HEALTH] WARNING: CPU thermal level $temp_level exceeds threshold 80" >> "$MASTER_LOG"
      healthy=1
    fi
    
    # Load average
    local load_1min cpu_count threshold
    load_1min=$(sysctl -n vm.loadavg 2>/dev/null | awk '{print $2}' || echo "0")
    cpu_count=$(sysctl -n hw.ncpu 2>/dev/null || echo "1")
    threshold=$(awk -v c="$cpu_count" 'BEGIN{printf "%.1f", c * 2.0}')
    if [ "$(echo "$load_1min $threshold" | awk '{if ($1 > $2) print 1; else print 0}')" = "1" ]; then
      echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [HEALTH] WARNING: Load $load_1min exceeds threshold $threshold" >> "$MASTER_LOG"
      healthy=1
    fi
    
    # Memory (from vm_stat)
    local pages_free pages_active pages_inactive pages_wired pages_speculative
    pages_free=$(vm_stat 2>/dev/null | awk '/Pages free/ {gsub(/\./, ""); print $3}' || echo "0")
    pages_active=$(vm_stat 2>/dev/null | awk '/Pages active/ {gsub(/\./, ""); print $3}' || echo "0")
    pages_inactive=$(vm_stat 2>/dev/null | awk '/Pages inactive/ {gsub(/\./, ""); print $3}' || echo "0")
    pages_wired=$(vm_stat 2>/dev/null | awk '/Pages wired down/ {gsub(/\./, ""); print $4}' || echo "0")
    pages_speculative=$(vm_stat 2>/dev/null | awk '/Pages speculative/ {gsub(/\./, ""); print $3}' || echo "0")
    
    local total_pages=$((pages_free + pages_active + pages_inactive + pages_wired + pages_speculative))
    if [ "$total_pages" -gt 0 ]; then
      local mem_percent
      mem_percent=$(awk -v free="$pages_free" -v total="$total_pages" 'BEGIN{printf "%.1f", (free/total)*100}')
      if [ "$(echo "$mem_percent 10.0" | awk '{if ($1 < $2) print 1; else print 0}')" = "1" ]; then
        echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [HEALTH] WARNING: Memory ${mem_percent}% free, below threshold 10%" >> "$MASTER_LOG"
        healthy=1
      fi
    fi
    
    # Disk space
    local disk_percent
    disk_percent=$(df -P / 2>/dev/null | awk 'NR==2 {print $5}' | sed 's/%//')
    if [ -n "$disk_percent" ] && [ "$disk_percent" -gt 95 ]; then
      echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [HEALTH] WARNING: Disk ${disk_percent}% used, above threshold 95%" >> "$MASTER_LOG"
      healthy=1
    fi
  fi
  
  if [ $healthy -eq 0 ]; then
    echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [HEALTH] System health check: OK" >> "$MASTER_LOG"
  fi
  
  return $healthy
}

# Map exit code to status string
map_exit_to_status() {
  case "$1" in
    0) echo "✅ PASS" ;;
    1) echo "🔴 FAIL" ;;
    2) echo "⚠️ SKIPPED" ;;
    *) echo "🔴 FAIL" ;;
  esac
}

# Adaptive pause with bounded retries
adaptive_pause() {
  local retries=0
  local max_retries=3
  local pause_seconds=60
  
  while [ $retries -lt $max_retries ]; do
    echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [HEALTH] System unhealthy, pausing ${pause_seconds}s (retry $((retries + 1))/$max_retries)..." >> "$MASTER_LOG"
    sleep $pause_seconds
    
    if check_system_health; then
      echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [HEALTH] System recovered, continuing" >> "$MASTER_LOG"
      return 0
    fi
    
    retries=$((retries + 1))
  done
  
  echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [HEALTH] WARNING: System still unhealthy after $max_retries retries, continuing anyway" >> "$MASTER_LOG"
  return 0
}

# Task execution wrapper
run_task() {
  local task_name="$1"
  local script_name="$2"
  local script_path="scripts/overnight/${script_name}.sh"
  
  # Health check before task
  if ! check_system_health; then
    adaptive_pause
  fi
  
  # Record start time
  local start_time
  start_time=$(date +%s)
  echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [TASK] Starting: $task_name" >> "$MASTER_LOG"
  
  # Create task report directory
  local task_dir="$REPORT_DIR/${script_name}"
  mkdir -p "$task_dir"
  
  # Run task script (capture exit code)
  # Note: script may fail, but we continue regardless
  # CRITICAL: Capture exit code immediately after invocation with no intervening commands
  "$script_path" "$task_dir" >> "$MASTER_LOG" 2>&1
  local exit_code=$?
  
  # Record end time and duration
  local end_time
  end_time=$(date +%s)
  local duration=$((end_time - start_time))
  
  # Determine status from exit code and set status variable
  local status_var="${task_name}_STATUS"
  local duration_var="${task_name}_DURATION"
  
  # Map exit code to status using centralized function
  local status
  status=$(map_exit_to_status "$exit_code")
  eval "${status_var}='$status'"
  
  # Log warning for unexpected exit codes (other than 0, 1, 2)
  if [ "$exit_code" -ne 0 ] && [ "$exit_code" -ne 1 ] && [ "$exit_code" -ne 2 ]; then
    echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [TASK] WARNING: $task_name returned unexpected exit code $exit_code" >> "$MASTER_LOG"
  fi
  
  eval "${duration_var}=$duration"
  
  echo "[$(date -u +%Y-%m-%dT%H:%M:%SZ)] [TASK] Finished: $task_name - $(eval echo \$${status_var}) (${duration}s)" >> "$MASTER_LOG"
}

# Generate OVERNIGHT_SUMMARY.md
generate_summary() {
  local summary_file="$REPORT_DIR/OVERNIGHT_SUMMARY.md"
  local end_time
  end_time=$(date +%s)
  local total_duration=$((end_time - OVERALL_START_TIME))
  local start_time_iso
  start_time_iso=$(date -u -r "$OVERALL_START_TIME" +%Y-%m-%dT%H:%M:%SZ 2>/dev/null || date -u -d "@$OVERALL_START_TIME" +%Y-%m-%dT%H:%M:%SZ)
  local end_time_iso
  end_time_iso=$(date -u +%Y-%m-%dT%H:%M:%SZ)
  local hostname
  hostname=$(hostname)
  
  # Count statuses
  local pass_count=0
  local fail_count=0
  local skip_count=0
  
  for status in "$TSAN_STATUS" "$ASAN_STATUS" "$VALGRIND_STATUS" "$SOAK_STATUS" "$COVERAGE_STATUS" "$STATIC_STATUS"; do
    case "$status" in
      *PASS*) pass_count=$((pass_count + 1)) ;;
      *FAIL*) fail_count=$((fail_count + 1)) ;;
      *SKIPPED*) skip_count=$((skip_count + 1)) ;;
    esac
  done
  
  # Count health warnings
  local health_warnings
  health_warnings=$(grep -c "\[HEALTH\] WARNING" "$MASTER_LOG" 2>/dev/null)
  if [ -z "$health_warnings" ]; then
    health_warnings=0
  fi
  local health_checks
  health_checks=$(grep -c "\[HEALTH\]" "$MASTER_LOG" 2>/dev/null)
  if [ -z "$health_checks" ]; then
    health_checks=0
  fi
  
  # Format duration
  local hours=$((total_duration / 3600))
  local minutes=$(((total_duration % 3600) / 60))
  local seconds=$((total_duration % 60))
  local duration_str="${hours}h ${minutes}m ${seconds}s"
  
  cat > "$summary_file" <<EOF
# Overnight QA Summary - ${TIMESTAMP}

## Platform
- OS: ${PLATFORM}
- Hostname: ${hostname}
- Start: ${start_time_iso}
- End: ${end_time_iso}
- Total Duration: ${duration_str} (${total_duration} seconds)

## Task Results

| Task | Status | Duration | Notes |
|------|--------|----------|-------|
| TSAN Stress | ${TSAN_STATUS} | ${TSAN_DURATION}s | See [tsan_report.md](tsan/tsan_report.md) |
| ASAN Stress | ${ASAN_STATUS} | ${ASAN_DURATION}s | See [asan_report.md](asan/asan_report.md) |
| Valgrind Check | ${VALGRIND_STATUS} | ${VALGRIND_DURATION}s | See [valgrind_report.md](valgrind/valgrind_report.md) |
| Soak Test | ${SOAK_STATUS} | ${SOAK_DURATION}s | See [soak_report.md](soak/soak_report.md) |
| Coverage Report | ${COVERAGE_STATUS} | ${COVERAGE_DURATION}s | See [coverage_report.md](coverage/coverage_report.md) |
| Static Analysis | ${STATIC_STATUS} | ${STATIC_DURATION}s | See [static_analysis_report.md](static_analysis/static_analysis_report.md) |

## Health Monitoring
- Health checks performed: ${health_checks}
- Warnings: ${health_warnings}

## Summary
- Passed: ${pass_count}
- Failed: ${fail_count}
- Skipped: ${skip_count}
EOF

  # Add "all tasks skipped" warning if applicable
  if [ $skip_count -eq 6 ] && [ $pass_count -eq 0 ] && [ $fail_count -eq 0 ]; then
    echo "" >> "$summary_file"
    echo "⚠️ **ALL TASKS SKIPPED** — No tests were actually executed." >> "$summary_file"
  fi
  
  cat >> "$summary_file" <<EOF

## Action Items
EOF

  # Add action items for failed tasks
  if [ "$TSAN_STATUS" = "🔴 FAIL" ]; then
    echo "- [ ] Investigate TSAN failures: see [tsan_report.md](tsan/tsan_report.md)" >> "$summary_file"
  fi
  if [ "$ASAN_STATUS" = "🔴 FAIL" ]; then
    echo "- [ ] Investigate ASAN failures: see [asan_report.md](asan/asan_report.md)" >> "$summary_file"
  fi
  if [ "$VALGRIND_STATUS" = "🔴 FAIL" ]; then
    echo "- [ ] Investigate Valgrind failures: see [valgrind_report.md](valgrind/valgrind_report.md)" >> "$summary_file"
  fi
  if [ "$SOAK_STATUS" = "🔴 FAIL" ]; then
    echo "- [ ] Investigate Soak test failures: see [soak_report.md](soak/soak_report.md)" >> "$summary_file"
  fi
  if [ "$COVERAGE_STATUS" = "🔴 FAIL" ]; then
    echo "- [ ] Investigate Coverage issues: see [coverage_report.md](coverage/coverage_report.md)" >> "$summary_file"
  fi
  if [ "$STATIC_STATUS" = "🔴 FAIL" ]; then
    echo "- [ ] Investigate Static analysis errors: see [static_analysis_report.md](static_analysis/static_analysis_report.md)" >> "$summary_file"
  fi
  
  if [ $fail_count -eq 0 ]; then
    echo "- No action items (all tasks passed or were skipped)" >> "$summary_file"
  fi
  
  echo "" >> "$summary_file"
  echo "---" >> "$summary_file"
  echo "" >> "$summary_file"
  echo "Full master log: [master.log](master.log)" >> "$summary_file"
}

# Main execution
main() {
  echo "Starting Overnight QA Framework..."
  echo "Report directory: $REPORT_DIR"
  
  # Phase 1: Critical Safety Checks
  echo "Phase 1: Critical Safety Checks"
  run_task "TSAN" "tsan_stress"
  run_task "ASAN" "asan_stress"
  run_task "VALGRIND" "valgrind_check"
  
  # Phase 2: Stability
  echo "Phase 2: Stability"
  run_task "SOAK" "soak_test"
  
  # Phase 3: Quality Metrics
  echo "Phase 3: Quality Metrics"
  run_task "COVERAGE" "coverage_report"
  run_task "STATIC" "static_analysis"
  
  # Generate summary
  generate_summary
  
  echo ""
  echo "Overnight QA Framework completed."
  echo "Summary: $REPORT_DIR/OVERNIGHT_SUMMARY.md"
  
  # Final exit code: 1 if any task failed, 0 otherwise
  local final_fail_count=0
  for status in "$TSAN_STATUS" "$ASAN_STATUS" "$VALGRIND_STATUS" "$SOAK_STATUS" "$COVERAGE_STATUS" "$STATIC_STATUS"; do
    case "$status" in
      *FAIL*) final_fail_count=$((final_fail_count + 1)) ;;
    esac
  done
  
  if [ $final_fail_count -gt 0 ]; then
    exit 1
  else
    exit 0
  fi
}

# Run main
main "$@"

