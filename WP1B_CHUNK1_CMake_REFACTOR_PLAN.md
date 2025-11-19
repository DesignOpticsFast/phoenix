# WP1.B Chunk 1 – Bedrock CMake Refactor Plan

## Prompt Review & Improvements

### Original Prompt Strengths
- ✅ Clear intent: Refactor without behavior change
- ✅ Well-structured steps
- ✅ Good verification criteria
- ✅ Explicit "no functional changes" constraint

### Areas for Improvement

1. **More specific categorization guidance** - What exactly goes where?
2. **Edge case handling** - What if cmake/ already exists?
3. **Order of operations clarity** - Some steps could be more explicit
4. **Comment preservation** - How to handle documentation
5. **Verification depth** - More specific checks

---

## Improved Plan

### Step 1: Pre-flight Inspection (Enhanced)

**Actions:**
1. **Verify Bedrock repo location:**
   ```bash
   cd /home/ec2-user/workspace/bedrock
   # Or adjust path if different
   ```

2. **Check existing structure:**
   - Does `cmake/` directory exist? If yes, list contents
   - How many lines in `CMakeLists.txt`? (gives sense of complexity)
   - Are there any existing CMake modules/helpers?

3. **Categorize CMakeLists.txt content:**
   
   **Category A: Project/Global Settings (STAYS in root CMakeLists.txt)**
   - `cmake_minimum_required(...)`
   - `project(...)`
   - Global policies: `set(CMAKE_CXX_STANDARD ...)`, `set(CMAKE_CXX_STANDARD_REQUIRED ON)`
   - `enable_testing()` (if truly global)
   - Language enablement: `enable_language(CXX)`
   - Global warnings/compiler flags (if any)
   
   **Category B: Options/Feature Flags (→ cmake/Options.cmake)**
   - `option(...)` statements
   - `set(BEDROCK_* ... CACHE ...)` cache variables
   - `set(BEDROCK_* ...)` variables that control build features
   - Feature toggles like `BEDROCK_WITH_TRANSPORT_DEPS`
   - Any `if(BEDROCK_*)` conditionals that set defaults
   
   **Category C: Dependencies (→ cmake/Dependencies.cmake)**
   - `find_package(...)` calls
   - `FetchContent_Declare(...)` / `FetchContent_MakeAvailable(...)`
   - `include()` statements for external CMake modules
   - `include_directories(...)` for third-party libs
   - `link_directories(...)` for third-party libs
   - Dependency-related `set(...)` variables (e.g., `set(Protobuf_ROOT ...)`)
   - Any dependency version checks or requirements
   
   **Category D: Targets (→ cmake/Targets.cmake)**
   - `add_library(...)`
   - `add_executable(...)`
   - `target_sources(...)`
   - `target_link_libraries(...)`
   - `target_include_directories(...)`
   - `target_compile_definitions(...)`
   - `target_compile_options(...)`
   - `set_target_properties(...)`
   - `add_test(...)` (if tightly coupled to targets)
   - `target_compile_features(...)`

4. **Document findings:**
   - Count lines in each category (rough estimate)
   - Note any complex interdependencies
   - Identify any tricky sections that need special care

---

### Step 2: Create cmake/ Directory Structure

**Actions:**
1. **Create cmake/ directory if it doesn't exist:**
   ```bash
   mkdir -p cmake
   ```

2. **If cmake/ already exists:**
   - List existing files
   - Don't overwrite existing helpers
   - Consider if existing files should be integrated or kept separate

---

### Step 3: Extract Options (cmake/Options.cmake)

**Guidelines:**
- Move ALL `option(...)` statements
- Move ALL `set(BEDROCK_* ... CACHE ...)` cache variables
- Move ALL feature flag `set(...)` statements
- **Preserve:**
  - Exact default values
  - All comments explaining options
  - Help text in `option()` statements
- **Add:**
  - File header comment explaining this file's purpose
  - Group related options together with section comments

**Example structure:**
```cmake
# cmake/Options.cmake
# Build options and feature flags for Bedrock

# Transport/Network options
option(BEDROCK_WITH_TRANSPORT_DEPS "Enable transport dependencies (gRPC, protobuf)" ON)

# Other options...
```

---

### Step 4: Extract Dependencies (cmake/Dependencies.cmake)

**Guidelines:**
- Move ALL `find_package(...)` calls
- Move ALL `FetchContent_*` blocks
- Move dependency-related `include()` statements
- Move dependency-related `include_directories()` / `link_directories()`
- **Preserve:**
  - Required/optional markers (`REQUIRED`, `QUIET`)
  - Version requirements
  - All comments explaining dependencies
- **Add:**
  - File header comment
  - Group dependencies logically (e.g., transport deps together)
  - Note any conditional dependencies (e.g., `if(BEDROCK_WITH_TRANSPORT_DEPS)`)

**Example structure:**
```cmake
# cmake/Dependencies.cmake
# External dependencies for Bedrock

# Transport dependencies (conditional)
if(BEDROCK_WITH_TRANSPORT_DEPS)
    find_package(Protobuf REQUIRED)
    find_package(gRPC REQUIRED)
    # ...
endif()
```

**Important:** Dependencies.cmake should NOT define targets, only discover dependencies.

---

### Step 5: Extract Targets (cmake/Targets.cmake)

**Guidelines:**
- Move ALL `add_library(...)` / `add_executable(...)` calls
- Move ALL target configuration (`target_*` commands)
- Move `add_test(...)` if tightly coupled to targets
- **Preserve:**
  - All source file lists
  - All link dependencies
  - All include directories
  - All compile definitions/options
  - All target properties
- **Add:**
  - File header comment
  - Group targets logically (libraries, executables, tests)
  - Preserve target ordering if it matters

**Example structure:**
```cmake
# cmake/Targets.cmake
# Target definitions for Bedrock

# Core library
add_library(bedrock_engine ...)
target_link_libraries(bedrock_engine ...)
# ...

# Executables
add_executable(bedrock_sanity ...)
target_link_libraries(bedrock_sanity bedrock_engine ...)
# ...
```

---

### Step 6: Update Top-Level CMakeLists.txt

**Guidelines:**
1. **Keep at top (in order):**
   - `cmake_minimum_required(...)`
   - `project(...)`
   - Global policies (C++ standard, etc.)
   - `enable_testing()` if global
   - Language settings

2. **Add module includes:**
   ```cmake
   # Include modular CMake files
   include(cmake/Options.cmake)
   include(cmake/Dependencies.cmake)
   include(cmake/Targets.cmake)
   ```

3. **Remove moved code:**
   - Delete all code moved to Options.cmake
   - Delete all code moved to Dependencies.cmake
   - Delete all code moved to Targets.cmake
   - **Double-check:** Nothing left behind that should have moved

4. **Handle CMAKE_MODULE_PATH:**
   - Usually not needed for `include()` with relative paths
   - Only add if `include()` fails to find files
   - Test without it first

5. **Add header comment:**
   ```cmake
   # Bedrock Engine - CMake Build Configuration
   # See cmake/Options.cmake, cmake/Dependencies.cmake, cmake/Targets.cmake
   ```

---

### Step 7: Verification (Enhanced)

**Build Verification:**
```bash
cd /home/ec2-user/workspace/bedrock

# Clean build
rm -rf build

# Configure with same options as before
cmake -S . -B build -DBUILD_TESTING=ON

# Verify configuration output
# - Check for any new warnings
# - Verify all options are recognized
# - Check dependency discovery messages

# Build all targets
cmake --build build --parallel

# Verify build output
# - All targets build successfully
# - No missing libraries
# - No undefined references

# Run tests
ctest --test-dir build --output-on-failure

# Verify test results
# - All tests pass (or same failures as before)
```

**Specific Checks:**
1. **Options preserved:**
   - List all options: `cmake -S . -B build -L | grep BEDROCK`
   - Verify defaults unchanged
   - Verify option help text preserved

2. **Dependencies found:**
   - Check configure output for dependency messages
   - Verify no "NOT FOUND" warnings (unless expected)

3. **Targets built:**
   - List all targets: `cmake --build build --target help`
   - Verify all expected targets exist
   - Verify target properties unchanged

4. **No regressions:**
   - Compare build output size/location
   - Compare test results (should be identical)

---

### Step 8: Deliverable & Reporting

**Summary Should Include:**

1. **Files Created:**
   - `cmake/Options.cmake` - X options, Y feature flags
   - `cmake/Dependencies.cmake` - Z dependencies
   - `cmake/Targets.cmake` - N targets

2. **Files Modified:**
   - `CMakeLists.txt` - Reduced from X lines to Y lines

3. **What Moved Where:**
   - **Options.cmake:** List key options (BEDROCK_WITH_TRANSPORT_DEPS, etc.)
   - **Dependencies.cmake:** List key dependencies (protobuf, gRPC, libsodium, etc.)
   - **Targets.cmake:** List key targets (bedrock_engine, bedrock_sanity, etc.)

4. **Behavior Verification:**
   - Configure: ✅ Success (same options recognized)
   - Build: ✅ Success (all targets built)
   - Tests: ✅ Pass (or same results as before)
   - **Explicit confirmation:** No options removed or changed

5. **Metrics:**
   - CMakeLists.txt line count: Before → After
   - Number of options extracted
   - Number of dependencies extracted
   - Number of targets extracted

6. **Next Steps Suggestion:**
   - Example: "WP1.B Chunk 2: Identify and remove unused/legacy options"
   - Example: "Add documentation comments for key options"
   - Example: "Standardize option naming conventions"

---

## Risk Mitigation

**Risk 1: Include order matters**
- **Mitigation:** Test include order carefully, fix if needed
- **Check:** Dependencies.cmake may need Options.cmake variables

**Risk 2: Missing code**
- **Mitigation:** Use `diff` to compare before/after
- **Check:** Ensure all code moved, nothing duplicated

**Risk 3: Breaking existing builds**
- **Mitigation:** Clean build test, verify all targets still build
- **Check:** Compare build artifacts (sizes, locations)

**Risk 4: Comments/documentation lost**
- **Mitigation:** Preserve all comments, add file headers
- **Check:** Review each file for comment preservation

---

## Success Criteria

- ✅ `cmake/Options.cmake` contains all build options
- ✅ `cmake/Dependencies.cmake` contains all dependency discovery
- ✅ `cmake/Targets.cmake` contains all target definitions
- ✅ Top-level `CMakeLists.txt` is slim and readable
- ✅ Clean configure + build + test succeeds
- ✅ No behavior change (same options, same targets, same tests)
- ✅ All comments preserved
- ✅ Clear summary provided

---

## Implementation Order

1. Inspect current structure (Step 1)
2. Create cmake/ directory (Step 2)
3. Extract Options.cmake (Step 3)
4. Extract Dependencies.cmake (Step 4)
5. Extract Targets.cmake (Step 5)
6. Update CMakeLists.txt (Step 6)
7. Verify no behavior change (Step 7)
8. Generate summary (Step 8)

---

## Notes

- This is a **pure refactor** - no feature changes
- **Preserve everything** - options, dependencies, targets, comments
- **Test thoroughly** - verify identical behavior
- **Document clearly** - make it easy to understand the new structure

