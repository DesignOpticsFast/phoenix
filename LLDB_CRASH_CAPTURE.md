# LLDB Crash Capture Instructions for macOS

## Purpose
Capture a full backtrace when Phoenix crashes on macOS startup to identify the exact crash site.

## Steps

### 1. Navigate to build directory
```bash
cd /Users/mark/Projects/phoenix/build/Qt_6_9_3_for_macOS_e74f68-Debug
```

### 2. Launch LLDB
```bash
lldb -- ./Phoenix.app/Contents/MacOS/Phoenix
```

### 3. Configure LLDB (at the `(lldb)` prompt)
```
settings set target.process.stop-on-exec false
breakpoint set --name main
run
```

### 4. When it crashes, capture backtrace
```
bt all                      # Full backtrace
thread backtrace all        # Alternative view
frame info
frame variable
register read
image lookup --address $pc
```

### 5. Save everything to file
```
log enable lldb all
script import sys
script sys.stdout = open('/tmp/phoenix_crash_full.txt','w')
bt all
thread list
frame variable
up
frame variable
up
frame variable
script sys.stdout.close()
quit
```

### 6. Also capture app output (from normal shell)
```bash
./Phoenix.app/Contents/MacOS/Phoenix --lang=en &> /tmp/phoenix_macos_boot.log
```

## Deliverables
Please share these two files:
- `/tmp/phoenix_crash_full.txt` - Full LLDB crash dump
- `/tmp/phoenix_macos_boot.log` - App output log

## Notes
- The crash happens right after i18n logs, before any UI appears
- We need to see the exact function and line number where the crash occurs
- The backtrace will show us if it's in MainWindow constructor, deferred lambda, or elsewhere

