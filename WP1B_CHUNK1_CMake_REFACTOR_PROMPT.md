# 🧠 UnderLord Prompt – WP1.B Chunk 1 – Bedrock CMake Refactor

## Intent

Refactor Bedrock's top-level CMake into modular files (`cmake/Options.cmake`, `cmake/Dependencies.cmake`, `cmake/Targets.cmake`) without changing any build behavior. This is a pure hygiene/refactor: no functional changes, no option removals.

---

## Context

- **Repo:** DesignOpticsFast/bedrock
- **Path:** `/home/ec2-user/workspace/bedrock`
- **Current State:**
  - Monolithic `CMakeLists.txt` (~377 lines)
  - Contains: project settings, options, dependencies, targets
  - Existing `cmake/` directory with `ToolchainCcache.cmake` (don't overwrite)
  - Behavior is correct; this is about structure/readability

**Goal:** Reorganize into modular files while keeping behavior identical.

---

## Ask

### Step 1: Inspect Current CMake Structure

1. **Navigate to Bedrock repo:**
   ```bash
   cd /home/ec2-user/workspace/bedrock
   ```

2. **Review current structure:**
   - Read `CMakeLists.txt` completely
   - Check existing `cmake/` directory contents (already has `ToolchainCcache.cmake`)
   - Note: Don't modify existing `cmake/` files

3. **Categorize content mentally:**

   **Category A: Project/Global (STAYS in root CMakeLists.txt)**
   - `cmake_minimum_required(...)`
   - `project(...)`
   - Global policies: `set(CMAKE_CXX_STANDARD 23)`, `set(CMAKE_CXX_STANDARD_REQUIRED ON)`
   - `enable_testing()` (if global)
   - Language enablement
   - Global compiler flags/warnings (if any)

   **Category B: Options/Feature Flags (→ cmake/Options.cmake)**
   - `option(...)` statements (e.g., `option(BEDROCK_WITH_TRANSPORT_DEPS ...)`)
   - `set(BEDROCK_* ... CACHE ...)` cache variables
   - `set(BEDROCK_* ...)` variables controlling build features
   - Feature toggles and conditional defaults

   **Category C: Dependencies (→ cmake/Dependencies.cmake)**
   - `find_package(...)` calls (protobuf, gRPC, libsodium, etc.)
   - `FetchContent_Declare(...)` / `FetchContent_MakeAvailable(...)`
   - `include()` statements for external CMake modules
   - `include_directories(...)` / `link_directories(...)` for third-party libs
   - Dependency-related `set(...)` variables (e.g., `set(Protobuf_ROOT ...)`)
   - Version checks/requirements

   **Category D: Targets (→ cmake/Targets.cmake)**
   - `add_library(...)` / `add_executable(...)`
   - `target_sources(...)`, `target_link_libraries(...)`, `target_include_directories(...)`
   - `target_compile_definitions(...)`, `target_compile_options(...)`
   - `set_target_properties(...)`
   - `add_test(...)` (if tightly coupled to targets)
   - `target_compile_features(...)`

**Do not change anything yet** - just understand the structure.

---

### Step 2: Create New Modular CMake Files

Create three new files in `cmake/` directory (don't modify existing `ToolchainCcache.cmake`):

#### 2.1 cmake/Options.cmake

**Move here:**
- ALL `option(...)` statements
- ALL `set(BEDROCK_* ... CACHE ...)` cache variables
- ALL feature flag `set(...)` statements

**Preserve:**
- Exact default values
- All comments explaining options
- Help text in `option()` statements

**Add:**
- File header comment: `# Build options and feature flags for Bedrock`
- Group related options with section comments

**Example structure:**
```cmake
# cmake/Options.cmake
# Build options and feature flags for Bedrock

# Transport/Network options
option(BEDROCK_WITH_TRANSPORT_DEPS "Enable transport dependencies (gRPC, protobuf)" ON)

# Other options...
```

#### 2.2 cmake/Dependencies.cmake

**Move here:**
- ALL `find_package(...)` calls
- ALL `FetchContent_*` blocks
- Dependency-related `include()` statements
- Dependency-related `include_directories()` / `link_directories()`
- Dependency-related `set(...)` variables

**Preserve:**
- Required/optional markers (`REQUIRED`, `QUIET`)
- Version requirements
- All comments explaining dependencies

**Add:**
- File header comment: `# External dependencies for Bedrock`
- Group dependencies logically (e.g., transport deps together)
- Note conditional dependencies (e.g., `if(BEDROCK_WITH_TRANSPORT_DEPS)`)

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

#### 2.3 cmake/Targets.cmake

**Move here:**
- ALL `add_library(...)` / `add_executable(...)` calls
- ALL target configuration (`target_*` commands)
- `add_test(...)` if tightly coupled to targets

**Preserve:**
- All source file lists
- All link dependencies
- All include directories
- All compile definitions/options
- All target properties
- Target ordering (if it matters)

**Add:**
- File header comment: `# Target definitions for Bedrock`
- Group targets logically (libraries, executables, tests)

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

### Step 3: Update Top-Level CMakeLists.txt

**Refactor CMakeLists.txt:**

1. **Keep at top (in order):**
   ```cmake
   cmake_minimum_required(...)
   project(...)
   # Global policies
   set(CMAKE_CXX_STANDARD 23)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
   enable_testing()  # if global
   ```

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

4. **Add header comment:**
   ```cmake
   # Bedrock Engine - CMake Build Configuration
   # See cmake/Options.cmake, cmake/Dependencies.cmake, cmake/Targets.cmake
   ```

5. **Handle CMAKE_MODULE_PATH:**
   - Usually not needed for `include()` with relative paths
   - Only add if `include()` fails to find files
   - Test without it first

**Result:** Top-level CMakeLists.txt should be slim (~20-30 lines) with just project setup and includes.

---

### Step 4: Verify No Behavior Change

**On dev-01, from Bedrock repo root:**

1. **Clean and configure:**
   ```bash
   cd /home/ec2-user/workspace/bedrock
   rm -rf build
   cmake -S . -B build -DBUILD_TESTING=ON
   ```

2. **Verify configuration:**
   - Check for any new warnings
   - Verify all options recognized: `cmake -S . -B build -L | grep BEDROCK`
   - Check dependency discovery messages
   - Verify no "NOT FOUND" warnings (unless expected)

3. **Build all targets:**
   ```bash
   cmake --build build --parallel
   ```

4. **Verify build:**
   - All targets build successfully
   - No missing libraries
   - No undefined references
   - List targets: `cmake --build build --target help`

5. **Run tests:**
   ```bash
   ctest --test-dir build --output-on-failure
   ```

6. **Verify tests:**
   - All tests pass (or same failures as before)
   - Compare results with previous runs

**Success Criteria:**
- ✅ CMake configuration succeeds with same options
- ✅ All targets build successfully
- ✅ Tests pass (or same results as before)
- ✅ No options removed or changed
- ✅ No behavior change

**If issues occur:**
- Fix minimal needed (e.g., missed include, wrong order)
- Don't change feature behavior
- Re-test until identical to before

---

### Step 5: Deliverable & Reporting

**Provide concise summary:**

1. **Files Created:**
   - `cmake/Options.cmake` - X options, Y feature flags
   - `cmake/Dependencies.cmake` - Z dependencies
   - `cmake/Targets.cmake` - N targets

2. **Files Modified:**
   - `CMakeLists.txt` - Reduced from ~377 lines to ~X lines

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
   - CMakeLists.txt line count: Before (~377) → After (~X)
   - Number of options extracted
   - Number of dependencies extracted
   - Number of targets extracted

6. **One Improvement Suggestion:**
   - Example: "WP1.B Chunk 2: Identify and remove unused/legacy options"
   - Example: "Add documentation comments for key options"
   - Example: "Standardize option naming conventions"

---

## Definition of Done

- ✅ Bedrock's CMake refactored into:
  - `cmake/Options.cmake`
  - `cmake/Dependencies.cmake`
  - `cmake/Targets.cmake`
- ✅ Top-level `CMakeLists.txt` is slim and readable, including those modules
- ✅ Clean configure + build + test run succeeds on dev-01 with `BUILD_TESTING=ON`
- ✅ No behavior or feature flags changed; only structure improved
- ✅ All comments preserved
- ✅ Clear summary provided

---

## Important Constraints

1. **Pure Refactor:** No functional changes, no option removals
2. **Preserve Everything:** Options, dependencies, targets, comments
3. **Don't Overwrite:** Existing `cmake/ToolchainCcache.cmake` should remain untouched
4. **Test Thoroughly:** Verify identical behavior before reporting completion

---

## Troubleshooting

**If include() fails:**
- Check file paths are correct
- May need `list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")`
- Test without CMAKE_MODULE_PATH first

**If options not recognized:**
- Check Options.cmake is included before Dependencies.cmake
- Verify option syntax unchanged

**If dependencies not found:**
- Check Dependencies.cmake includes Options.cmake variables if needed
- Verify find_package() calls unchanged

**If targets fail to build:**
- Check Targets.cmake includes Dependencies.cmake
- Verify target_link_libraries() unchanged
- Check include order

---

## Summary

Refactor monolithic CMakeLists.txt into three modular files (Options, Dependencies, Targets) while preserving all behavior. This improves readability and maintainability without changing any build functionality.

