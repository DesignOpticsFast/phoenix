# Preflight Build Hook Failure - Root Cause Analysis & Fix

**Date:** 2025-11-06  
**Reporter:** Auto (AI Assistant)  
**For:** Lyra  
**Issue:** Pre-push hook build failure preventing git push

## Executive Summary

After committing menu icon refresh changes, the git push failed due to a pre-push hook that runs a build verification. The build failure was caused by two bugs in the preflight script (`scripts/lib/preflight_common.sh`):

1. **Output corruption bug**: The `resolve_qt_prefix()` function printed debug messages to stdout, which were captured when assigning the function's return value, corrupting the Qt installation path.
2. **Environment variable corruption**: The `CMAKE_PREFIX_PATH` environment variable contained newline characters, which CMake rejects, causing configuration failures.

Both issues have been fixed. The push now succeeds, but these bugs should not have existed in the first place.

## Timeline of Events

1. **Commit successful**: Menu icon refresh changes committed to `sprint4-ui-hardening` branch (commit `a63281f1`)
2. **Push attempt failed**: Git push triggered pre-push hook, which runs `scripts/dev01-preflight.sh`
3. **Build failure**: Preflight build failed with CMake error: "Could not find a package configuration file provided by 'Qt6'"
4. **Investigation**: Discovered that:
   - Qt 6.10.0 exists at `/opt/Qt/6.10.0/gcc_64` (correct location)
   - Environment had `CMAKE_PREFIX_PATH` pointing to non-existent Qt 6.9.3
   - The `resolve_qt_prefix()` function correctly found Qt 6.10.0 as fallback
   - However, the function's debug output was being captured, corrupting the path
5. **Fix applied**: 
   - Redirected debug output to stderr in `resolve_qt_prefix()`
   - Added cleanup to strip newlines from `CMAKE_PREFIX_PATH`
6. **Push succeeded**: After fix, preflight build passes and push completes

## Root Cause Analysis

### Bug #1: Debug Output Corruption

**Location:** `scripts/lib/preflight_common.sh::resolve_qt_prefix()`

**Problem:**
```bash
resolve_qt_prefix() {
  local cpp="${CMAKE_PREFIX_PATH:-}"
  echo "Searching for Qt6Config.cmake..."  # ← Goes to stdout
  # ... search logic ...
  echo "$p"; return 0  # ← Also goes to stdout
}
```

When called as:
```bash
qt_prefix="$(resolve_qt_prefix)"  # Captures ALL stdout
```

The captured value included both the debug message AND the actual path:
```
"Searching for Qt6Config.cmake in CMAKE_PREFIX_PATH: ...
/opt/Qt/6.10.0/gcc_64"
```

This corrupted path was then passed to CMake, causing it to fail finding Qt6.

**Fix:**
Redirected all debug/informational output to stderr (`>&2`), leaving only the actual Qt prefix path on stdout:

```bash
resolve_qt_prefix() {
  local cpp="${CMAKE_PREFIX_PATH:-}"
  echo "Searching for Qt6Config.cmake..." >&2  # ← Now goes to stderr
  # ... search logic ...
  echo "$p"; return 0  # ← Only this goes to stdout
}
```

### Bug #2: Newline Corruption in Environment Variable

**Location:** `scripts/lib/preflight_common.sh::preflight_configure_build()`

**Problem:**
The `CMAKE_PREFIX_PATH` environment variable contained newline characters (likely from shell initialization or environment setup). CMake explicitly rejects newlines in `CMAKE_PREFIX_PATH` and emits a warning:
```
CMake Warning: Value of CMAKE_PREFIX_PATH contained a newline; truncating
```

This truncation caused CMake to lose the correct Qt path.

**Fix:**
Added cleanup step to strip newlines and normalize the path before use:

```bash
# Clean CMAKE_PREFIX_PATH: remove newlines and normalize
local cleaned_cpp
if [[ -n "${CMAKE_PREFIX_PATH:-}" ]]; then
  cleaned_cpp="$(echo "$CMAKE_PREFIX_PATH" | tr -d '\n\r' | tr -s ':' | sed 's/^://;s/:$//')"
fi
: "${cleaned_cpp:=/opt/Qt/6.10.0/gcc_64}"
export CMAKE_PREFIX_PATH="$cleaned_cpp"
```

Also added a final cleanup step after building the CMake prefix list to ensure no newlines slip through:

```bash
cmake_prefix_list="$(build_cmake_prefix_list "$qt_prefix" "$CMAKE_PREFIX_PATH")"
cmake_prefix_list="$(echo "$cmake_prefix_list" | tr -d '\n\r')"  # Final safety check
```

## Why This Shouldn't Have Happened

1. **Best Practice Violation**: Functions that return values via stdout should NEVER print debug/informational messages to stdout. This is a fundamental shell scripting best practice that should have been caught during code review.

2. **Lack of Input Sanitization**: The preflight script should sanitize environment variables before using them, especially when passing them to external tools like CMake that have strict requirements.

3. **Insufficient Testing**: These bugs would have been caught if:
   - The preflight script had been tested with corrupted environment variables
   - The script had been tested in the actual environment where it runs (dev-01)
   - There had been integration tests for the git hook workflow

4. **Missing Error Handling**: The script didn't validate that the resolved Qt path was actually valid before using it, which could have provided earlier error detection.

## Recommendations

1. **Code Review**: Add a checklist item for shell scripts: "Functions returning values via stdout must not print debug output to stdout"

2. **Input Sanitization**: Add a standard input sanitization function for environment variables used in build scripts:
   ```bash
   sanitize_path_var() {
     echo "$1" | tr -d '\n\r' | tr -s ':' | sed 's/^://;s/:$//'
   }
   ```

3. **Testing**: 
   - Add unit tests for `resolve_qt_prefix()` with various `CMAKE_PREFIX_PATH` values
   - Test with corrupted environment variables (newlines, special characters)
   - Test the full git hook workflow in CI/CD

4. **Validation**: Add validation after resolving Qt prefix to ensure the path is valid:
   ```bash
   qt_prefix="$(resolve_qt_prefix)" || exit 1
   [[ -f "$qt_prefix/lib/cmake/Qt6/Qt6Config.cmake" ]] || {
     echo "ERROR: Resolved Qt prefix is invalid: $qt_prefix" >&2
     exit 1
   }
   ```

5. **Documentation**: Document the expected format of `CMAKE_PREFIX_PATH` and add warnings if it doesn't match expectations.

## Files Modified

- `scripts/lib/preflight_common.sh`:
  - `resolve_qt_prefix()`: Redirected debug output to stderr
  - `preflight_configure_build()`: Added `CMAKE_PREFIX_PATH` sanitization

## Verification

After the fix:
- ✅ Preflight build completes successfully
- ✅ CMake correctly finds Qt 6.10.0
- ✅ Git push succeeds
- ✅ No more "newline" warnings from CMake

## Conclusion

These were preventable bugs that should have been caught during development or code review. The fixes are now in place, but the underlying issues (lack of input sanitization and stdout pollution) should be addressed systematically across all build scripts to prevent similar issues in the future.

---

**Status:** ✅ Fixed and verified  
**Impact:** Low (blocked one push, but fix was straightforward)  
**Priority for Prevention:** Medium (should add to code review checklist and testing)

