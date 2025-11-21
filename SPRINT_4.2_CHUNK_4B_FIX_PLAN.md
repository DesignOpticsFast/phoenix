# Sprint 4.2 Chunk 4B-Fix: High-Priority Fixes Plan

**Date:** 2025-11-21  
**Status:** PLANNING ONLY — No implementation yet  
**Intent:** Plan surgical fixes for three high-priority issues identified by Claude's review

---

## ⚠️ UNDERLORD WORKFLOW DIRECTIVE (CRITICAL)

**From this point forward, all implementation work MUST operate on the canonical repo working copy, not on any files extracted from the review ZIP.**

**Your source of truth is:**
- `/home/ec2-user/workspace/phoenix/`
- `/home/ec2-user/workspace/bedrock/`

**You may read the ZIP contents (`sprint_4.2_chunk_4b_review.zip`) only as a reference for review, but all changes, edits, patches, commits, and tests MUST be applied to the actual repo files in the working directories above.**

**If any file from the ZIP and the repo disagrees, the repo version wins.**

**Acknowledge this in your plan before implementing anything.**

**Why this is necessary:**
- The ZIP is a snapshot, not the evolving codebase
- UL must never patch "dead" files
- UL should only make commits inside the repo directories

---

## Executive Summary

**Three Issues to Fix:**
1. **Parameter parsing duplication** in Bedrock (`PalantirServer.cpp`)
2. **Missing structured TransportError codes** in Phoenix transport layer
3. **String-matching error classification** in `AnalysisWorker` and `XYAnalysisWindow`

**Scope:** Strictly within identified files — no architectural changes, no UI refactoring (just TODO comment).

---

## Issue 1: Parameter Parsing Duplication (Bedrock)

### Exact Locations

**File:** `bedrock/src/palantir/PalantirServer.cpp`

**Duplicated Code Found In:**

1. **`validateXYSineParameters()`** (lines ~400-490)
   - Parses `frequency`, `amplitude`, `phase`, `samples`, `n_samples`
   - Tracks `hasFrequency`, `hasAmplitude`, `hasPhase`, `hasSamples` flags
   - Returns validation error messages

2. **`computeXYSine()`** (lines ~500-550)
   - Parses same parameters (identical logic)
   - No validation flags (assumes already validated)
   - Defensive minimum samples check

3. **`computeXYSineWithProgress()`** (lines ~575-625)
   - Parses same parameters (identical logic)
   - No validation flags
   - Defensive minimum samples check

**Duplication Pattern:**
```cpp
// Repeated in all three methods:
double frequency = 1.0;
double amplitude = 1.0;
double phase = 0.0;
int samples = 1000;
bool explicitSamplesSet = false;

for (const auto& [key, value] : spec.params()) {
    QString paramKey = QString::fromStdString(key);
    QString paramValue = QString::fromStdString(value);
    
    if (paramKey == "frequency") {
        bool ok;
        double val = paramValue.toDouble(&ok);
        if (ok) { frequency = val; }
    }
    // ... (same for amplitude, phase, samples, n_samples)
}
```

**Additional Issue:** Line ~480 has incorrect phase formatting:
```cpp
// Current (WRONG):
return QString("Parameter 'phase' out of range: %1 (valid: -%.2f to %.2f)").arg(phase).arg(-twoPi).arg(twoPi);

// Should be:
return QString("Parameter 'phase' out of range: %1 (valid: %2 to %3)")
    .arg(phase).arg(-twoPi, 0, 'f', 2).arg(twoPi, 0, 'f', 2);
```

### Proposed Fix

**Create Shared Helper Struct and Function:**

**In `PalantirServer.hpp`:**
```cpp
// Add after computeXYSine declaration:
struct ParsedXYSineParams {
    double frequency = 1.0;
    double amplitude = 1.0;
    double phase = 0.0;
    int samples = 1000;
    
    // Flags for validation (only used in validateXYSineParameters)
    bool hasFrequency = false;
    bool hasAmplitude = false;
    bool hasPhase = false;
    bool hasSamples = false;
};

// Helper function to parse parameters (no validation)
ParsedXYSineParams parseXYSineParameters(const palantir::ComputeSpec& spec);
```

**In `PalantirServer.cpp`:**
1. **Implement `parseXYSineParameters()`:**
   - Extract the common parsing logic
   - Set `has*` flags based on presence in `spec.params()`
   - Handle `samples` vs `n_samples` precedence
   - Return `ParsedXYSineParams` struct

2. **Refactor `validateXYSineParameters()`:**
   - Call `parseXYSineParameters()` to get parsed params
   - Use `has*` flags to validate only explicitly-set parameters
   - Fix phase formatting bug (use `.arg(value, 0, 'f', 2)`)

3. **Refactor `computeXYSine()`:**
   - Call `parseXYSineParameters()` to get parsed params
   - Use struct members directly
   - Remove defensive minimum check (validation already handled)

4. **Refactor `computeXYSineWithProgress()`:**
   - Call `parseXYSineParameters()` to get parsed params
   - Use struct members directly
   - Remove defensive minimum check

**Benefits:**
- ✅ Single source of truth for parameter parsing
- ✅ Consistent behavior across all methods
- ✅ Easier to maintain (changes in one place)
- ✅ Fixes phase formatting bug

---

## Issue 2: Missing Structured TransportError Codes (Phoenix)

### Exact Locations

**File:** `phoenix/src/transport/TransportClient.hpp`

**Current Enum (lines 6-13):**
```cpp
enum class TransportError {
    NoError = 0,
    ConnectionFailed,
    ConnectionTimeout,
    ProtocolError,
    ServerError,          // ← Too generic!
    NetworkError
};
```

**File:** `phoenix/src/transport/LocalSocketChannel.cpp`

**Current Mapping (lines 424-438):**
```cpp
// Check status and map to specific error types
QString status = QString::fromStdString(startReply.status());
if (status != "OK") {
    QString errorMsg = QString::fromStdString(startReply.error_message());
    
    // Map protocol status to TransportError
    if (status == "INVALID_ARGUMENT") {
        setError(TransportError::ServerError, errorMsg);  // ← All map to ServerError!
    } else if (status == "UNIMPLEMENTED") {
        setError(TransportError::ServerError, ...);
    } else if (status == "RESOURCE_EXHAUSTED") {
        setError(TransportError::ServerError, ...);
    } else {
        setError(TransportError::ServerError, ...);
    }
    return false;
}
```

**Problem:** All server errors map to generic `ServerError`, losing semantic information.

### Proposed Fix

**Extend `TransportError` Enum:**

**In `TransportClient.hpp`:**
```cpp
enum class TransportError {
    NoError = 0,
    ConnectionFailed,      // Cannot connect to server socket
    ConnectionTimeout,     // Connection attempt timed out
    ProtocolError,         // Protocol/message parsing error
    NetworkError,         // Socket/network I/O error
    
    // Server-side errors (new)
    InvalidArgument,      // INVALID_ARGUMENT - bad parameters
    ResourceExhausted,   // RESOURCE_EXHAUSTED - server at capacity
    Unimplemented,       // UNIMPLEMENTED - feature not supported
    PermissionDenied,    // PERMISSION_DENIED - license issue
    Cancelled,           // CANCELLED - job was cancelled
    ServerError          // Generic server error (fallback)
};
```

**Update `transportErrorString()` Helper:**
```cpp
inline QString transportErrorString(TransportError error) {
    switch (error) {
        // ... existing cases ...
        case TransportError::InvalidArgument:
            return QStringLiteral("Invalid parameters provided.");
        case TransportError::ResourceExhausted:
            return QStringLiteral("Server at capacity.");
        case TransportError::Unimplemented:
            return QStringLiteral("Feature not supported.");
        case TransportError::PermissionDenied:
            return QStringLiteral("License required.");
        case TransportError::Cancelled:
            return QStringLiteral("Operation cancelled.");
        case TransportError::ServerError:
            return QStringLiteral("Bedrock server returned an error.");
        // ...
    }
}
```

**Update `LocalSocketChannel::computeXYSine()` Mapping:**

**In `LocalSocketChannel.cpp` (lines 424-438):**
```cpp
// Check status and map to specific error types
QString status = QString::fromStdString(startReply.status());
if (status != "OK") {
    QString errorMsg = QString::fromStdString(startReply.error_message());
    
    // Map protocol status to specific TransportError codes
    TransportError errorCode;
    if (status == "INVALID_ARGUMENT") {
        errorCode = TransportError::InvalidArgument;
    } else if (status == "UNIMPLEMENTED") {
        errorCode = TransportError::Unimplemented;
    } else if (status == "RESOURCE_EXHAUSTED") {
        errorCode = TransportError::ResourceExhausted;
    } else if (status == "PERMISSION_DENIED") {
        errorCode = TransportError::PermissionDenied;
    } else {
        errorCode = TransportError::ServerError;  // Fallback
    }
    
    setError(errorCode, errorMsg);
    return false;
}
```

**Also Map `ResultMeta.status`:**
- `ResultMeta.status == "CANCELLED"` → `TransportError::Cancelled`
- `ResultMeta.status == "FAILED"` → `TransportError::ServerError`

**Benefits:**
- ✅ Semantic error codes (no string matching needed)
- ✅ Type-safe error handling
- ✅ Easier to extend in future

---

## Issue 3: String-Matching Error Classification (Phoenix)

### Exact Locations

**File:** `phoenix/src/analysis/AnalysisWorker.cpp`

**Current String Matching (lines 252-260):**
```cpp
} else if (errorCode == TransportError::ServerError) {
    // Check if it's INVALID_ARGUMENT (error string will contain parameter details)
    if (errorString.contains("out of range") || errorString.contains("Parameter")) {
        // INVALID_ARGUMENT - will be handled as inline error
        errorMessage = errorString;  // Pass through parameter validation error
    } else {
        // INTERNAL or other server error
        errorMessage = tr("Computation failed: %1").arg(errorString);
    }
}
```

**Problem:** Infers error type from English text (brittle, not maintainable).

**File:** `phoenix/src/ui/analysis/XYAnalysisWindow.cpp`

**Current String Matching (lines 233-243):**
```cpp
// Determine error type from error message content
bool isInvalidArgument = error.contains("out of range") || 
                         error.contains("Parameter") ||
                         error.contains("too small") ||
                         error.contains("not a valid");
bool isUnavailable = error.contains("not available") || 
                     error.contains("not running") ||
                     error.contains("Connection");
bool isPermissionDenied = error.contains("license") || 
                          error.contains("License") ||
                          error.contains("PERMISSION_DENIED");
bool isInternal = error.contains("failed") && !isInvalidArgument && !isUnavailable;
```

**Problem:** Entire UI error classification relies on string matching (very brittle).

### Proposed Fix

**A. Update `AnalysisWorker` to Use Structured Errors:**

**In `AnalysisWorker.cpp` (lines 242-265):**
```cpp
// Get error details from transport client
TransportError errorCode = client->lastError();
QString errorString = client->lastErrorString();

// Map error codes to user-friendly messages using switch (no string matching)
QString errorMessage;
switch (errorCode) {
    case TransportError::ConnectionFailed:
    case TransportError::ConnectionTimeout:
        // UNAVAILABLE - Bedrock not running
        errorMessage = tr("Bedrock server is not available.\n\n"
                         "Please ensure Bedrock is running and try again.");
        break;
        
    case TransportError::InvalidArgument:
        // INVALID_ARGUMENT - pass through parameter validation error
        errorMessage = errorString;  // Already user-friendly from Bedrock
        break;
        
    case TransportError::Unimplemented:
        errorMessage = tr("Feature not supported: %1").arg(errorString);
        break;
        
    case TransportError::ResourceExhausted:
        errorMessage = tr("Server at capacity: %1").arg(errorString);
        break;
        
    case TransportError::PermissionDenied:
        errorMessage = tr("License required: %1").arg(errorString);
        break;
        
    case TransportError::Cancelled:
        errorMessage = tr("Computation cancelled.");
        break;
        
    case TransportError::ServerError:
    case TransportError::ProtocolError:
    case TransportError::NetworkError:
    default:
        // INTERNAL or other errors
        errorMessage = tr("Computation failed: %1").arg(
            errorString.isEmpty() ? tr("Unknown error") : errorString);
        break;
}

emit finished(false, QVariant(), errorMessage);
```

**B. Document UI String Matching as Tech Debt:**

**In `XYAnalysisWindow.cpp` (before line 233):**
```cpp
// TODO(Sprint 4.3): Replace string matching with structured error codes.
// AnalysisWorker should pass TransportError enum along with error message,
// allowing UI to use switch statement instead of brittle string.contains() checks.
// Current implementation works but is fragile and hard to maintain.

// Determine error type from error message content
bool isInvalidArgument = error.contains("out of range") || 
                         // ... (keep existing code for now)
```

**Benefits:**
- ✅ Backend uses structured errors (clean, maintainable)
- ✅ UI documented as tech debt (explicit, not hidden)
- ✅ Future refactor path is clear (Sprint 4.3)

---

## Scope Confirmation

### Files to Modify

**Bedrock:**
1. ✅ `src/palantir/PalantirServer.hpp` — Add `ParsedXYSineParams` struct and `parseXYSineParameters()` declaration
2. ✅ `src/palantir/PalantirServer.cpp` — Implement helper, refactor three methods, fix phase formatting

**Phoenix:**
1. ✅ `src/transport/TransportClient.hpp` — Extend `TransportError` enum, update `transportErrorString()`
2. ✅ `src/transport/LocalSocketChannel.hpp` — (No changes needed, enum is in base class)
3. ✅ `src/transport/LocalSocketChannel.cpp` — Update error mapping to use new enum codes
4. ✅ `src/analysis/AnalysisWorker.cpp` — Replace string matching with switch statement
5. ✅ `src/ui/analysis/XYAnalysisWindow.cpp` — Add TODO comment (no code changes)

### Files NOT Modified

- ❌ No UI refactoring (just TODO comment)
- ❌ No architectural changes
- ❌ No test files (testing comes in Chunk 4C)
- ❌ No CMake files
- ❌ No protocol files

---

## Dependencies and Ambiguities

### Dependencies

1. **`ParsedXYSineParams` struct:**
   - Must be accessible to `validateXYSineParameters()`, `computeXYSine()`, and `computeXYSineWithProgress()`
   - **Decision:** Place in `PalantirServer.hpp` as private struct (or public if needed for testing)

2. **`TransportError` enum extension:**
   - Must be backward-compatible (existing code uses `ServerError`)
   - **Decision:** Add new codes, keep `ServerError` as fallback

3. **Error message propagation:**
   - `LocalSocketChannel` sets error code + message
   - `AnalysisWorker` reads error code + message
   - `XYAnalysisWindow` receives only message (for now)
   - **Decision:** Keep current signal signature (`finished(bool, QVariant, QString)`), add TODO for future enhancement

### Ambiguities

1. **Phase formatting precision:**
   - Current bug uses `%.2f` in `QString::arg()` (incorrect)
   - **Decision:** Use `.arg(value, 0, 'f', 2)` for 2 decimal places

2. **`ResultMeta.status` mapping:**
   - Currently not mapped to `TransportError` codes
   - **Decision:** Map `"CANCELLED"` → `TransportError::Cancelled`, `"FAILED"` → `TransportError::ServerError`

3. **`has*` flags in `ParsedXYSineParams`:**
   - Only needed for validation (to check if parameter was explicitly set)
   - **Decision:** Include flags in struct, but only set them in `parseXYSineParameters()` when called from `validateXYSineParameters()`

4. **Minimum samples defensive check:**
   - Currently in `computeXYSine()` and `computeXYSineWithProgress()`
   - **Decision:** Remove defensive checks (validation already handled in `handleStartJob()`)

---

## Implementation Order

**Step 0: Acknowledge Workflow Directive**
- ✅ Confirm you are working in `/home/ec2-user/workspace/phoenix/` and `/home/ec2-user/workspace/bedrock/`
- ✅ Verify you are NOT editing files from the ZIP archive
- ✅ All file paths in this plan refer to the live repo directories

1. **Bedrock: Extract Parameter Parsing Helper**
   - Add struct and function declaration to `.hpp`
   - Implement `parseXYSineParameters()` in `.cpp`
   - Refactor `validateXYSineParameters()` (fix phase formatting)
   - Refactor `computeXYSine()`
   - Refactor `computeXYSineWithProgress()`

2. **Phoenix: Extend TransportError Enum**
   - Add new error codes to `TransportClient.hpp`
   - Update `transportErrorString()` helper
   - Update `LocalSocketChannel::computeXYSine()` mapping
   - Map `ResultMeta.status` to error codes

3. **Phoenix: Replace String Matching**
   - Update `AnalysisWorker::executeCompute()` to use switch statement
   - Add TODO comment to `XYAnalysisWindow::onWorkerFinished()`

---

## Testing Considerations

**Note:** Testing will be done in Chunk 4C (integration testing). This chunk focuses on code quality fixes only.

**Future Tests Should Verify:**
- Parameter parsing consistency across all three methods
- Error code mapping correctness (all `StartReply.status` values map correctly)
- Error message propagation (structured errors flow through to UI)

---

## Summary

**Three Surgical Fixes:**
1. ✅ Extract `parseXYSineParameters()` helper (eliminates duplication, fixes phase bug)
2. ✅ Extend `TransportError` enum (adds semantic error codes)
3. ✅ Replace string matching with switch (backend clean, UI documented as tech debt)

**Scope:** Strictly within identified files, no architectural changes, minimal UI change (just TODO).

**Ready for Implementation:** ✅ Yes — all ambiguities resolved, dependencies identified.

