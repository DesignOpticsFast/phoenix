# WP1.D Chunk 1 – Documentation Skeleton Plan

## Prompt Review & Analysis

### Original Prompt Strengths
- ✅ Clear structure: Phoenix vs Bedrock docs
- ✅ Specific file paths and content requirements
- ✅ Emphasis on "skeletal but accurate" (match Sprint 4.2 reality)
- ✅ Good separation of concerns (architecture, build, ADR)

### Areas for Improvement

1. **Verify current state first** - Check what actually exists before documenting
2. **Check existing docs** - Don't duplicate or conflict with existing documentation
3. **Verify architecture** - Understand actual code structure before documenting
4. **Template structure** - Provide clear templates for each doc type
5. **Accuracy checks** - Verify claims match actual codebase

---

## Improved Plan

### Pre-flight: Verify Current State

**Phoenix:**
1. Check existing docs structure:
   ```bash
   cd /home/ec2-user/workspace/phoenix
   find docs -type d
   ls -la docs/
   ```

2. Verify key components exist:
   - `src/ui/main/MainWindow.*` - Main window
   - `src/ui/analysis/AnalysisWindow.*` - Analysis window
   - `src/ui/analysis/IAnalysisView.hpp` - View interface
   - `src/plot/XYPlotViewGraphs.*` - Qt Graphs view
   - `src/transport/` - Transport stubs
   - `src/qml/XYPlotView.qml` - QML for Qt Graphs

3. Check CMake for Qt version:
   ```bash
   grep "Qt6.*6.10" CMakeLists.txt
   ```

**Bedrock:**
1. Check existing docs structure:
   ```bash
   cd /home/ec2-user/workspace/bedrock
   find docs -type d
   ls -la docs/
   ```

2. Verify key components:
   - `src/bedrock_engine.cpp` - Core engine
   - `tests/bedrock_sanity.cpp` - Sanity test
   - `tests/bedrock_echo_sanity.cpp` - Echo RPC test
   - `cmake/Options.cmake` - Build options (if exists from WP1.B)
   - `contracts/` - Palantir contracts submodule

3. Check CMake for C++ standard:
   ```bash
   grep "CXX_STANDARD" CMakeLists.txt
   ```
   **Note:** Currently shows C++20, not C++23 - document actual state, not aspirational

---

### Step 1: Phoenix Architecture Overview

**File:** `docs/architecture/overview.md`

**Structure:**
```markdown
# Phoenix Architecture Overview

## Purpose
Phoenix is a Qt 6.10-based UI application that provides a graphical interface for the Bedrock analysis engine.

## High-Level Architecture

### Components

#### MainWindow
- Primary application window
- Ribbon toolbar for actions
- Dock widgets for panels
- Menu system

#### AnalysisWindow
- Container for analysis visualizations
- Manages IAnalysisView instances
- Window-per-analysis model

#### IAnalysisView Interface
- Abstract interface for analysis views
- Implementations: XYPlotViewGraphs (Qt Graphs)
- Future: scatter plots, 3D views, tables

#### XYPlotViewGraphs
- Qt Graphs 2D implementation
- Uses QQuickWidget + QML (XYPlotView.qml)
- Renders XY line plots
- Supports pan/zoom interactions

#### Transport Layer (Current State)
- TransportClient: stub interface
- GrpcUdsChannel: stub (WP2)
- LocalSocketChannel: stub (WP2)
- Real transport implementation planned for WP2

## Data Flow (Intended)

```
Phoenix UI → FeatureManager → TransportClient → Bedrock Engine
                                                      ↓
Phoenix AnalysisWindow ← TransportClient ← Analysis Results
```

## Current Sprint 4.2 State

- ✅ Qt Graphs integration complete
- ✅ XYPlotViewGraphs functional with dummy data
- ✅ AnalysisWindow can display Qt Graphs views
- ⏳ Transport layer stubbed (WP2)
- ⏳ FeatureManager planned (WP2/WP3)
```

**Key Points:**
- Match actual code structure
- Note what's complete vs stubbed
- Keep it factual, not aspirational

---

### Step 2: Phoenix Analysis Loop

**File:** `docs/architecture/analysis_loop.md`

**Structure:**
```markdown
# Analysis Loop Architecture

## Intended Flow

1. User triggers analysis (e.g., XY Sine) via FeatureManager UI
2. FeatureManager constructs request with parameters
3. TransportClient sends request to Bedrock via gRPC/LocalSocket
4. Bedrock processes analysis and streams results
5. Phoenix receives results and updates AnalysisWindow
6. XYPlotViewGraphs renders data via Qt Graphs

## Current Sprint 4.2 State

### Implemented
- AnalysisWindow exists and can host views
- XYPlotViewGraphs can display XY data (dummy sine wave)
- Qt Graphs QML integration working
- Basic pan/zoom interactions enabled

### Planned (WP2/WP3)
- FeatureManager / FeatureRegistry for feature discovery
- TransportClient full implementation (gRPC UDS + LocalSocket)
- Real data flow from Bedrock to Phoenix
- Progress reporting and cancellation
- Error handling matrix

### Example: XY Sine Feature (Planned)

1. User selects "XY Sine" from Feature Manager
2. Parameters: frequency, amplitude, point count
3. Phoenix sends request to Bedrock via TransportClient
4. Bedrock generates sine wave data
5. Results streamed back to Phoenix
6. XYPlotViewGraphs updates with real data
7. User can pan/zoom the plot

## Transport Design

- Primary: gRPC over Unix Domain Socket (UDS)
- Fallback: LocalSocket (plain socket)
- Contracts: Shared Palantir proto definitions
- Environment: `PHOENIX_TRANSPORT=auto` (defaults to gRPC, falls back to LocalSocket)
```

**Key Points:**
- Document intended flow
- Clearly mark what's current vs planned
- Reference WP2/WP3 for future work

---

### Step 3: Phoenix Build Documentation

**File:** `docs/build/phoenix_build.md`

**Structure:**
```markdown
# Building Phoenix

## Requirements

- Qt 6.10.0 or later
- CMake 3.20 or later
- C++17-capable compiler (GCC 11+, Clang 14+, MSVC 2019+)
- Linux (dev-01) or macOS

## Quick Start

### Linux (dev-01)

```bash
git clone <repo-url>
cd phoenix
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64 \
  -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### macOS

```bash
git clone <repo-url>
cd phoenix
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.10.0/macos \
  -DBUILD_TESTING=OFF
cmake --build build --parallel
```

## Build Targets

- `phoenix_app` - Main application executable
- `phoenix_analysis` - Analysis window library
- `analysis_sanity_tests` - Analysis window tests
- `graphs_perf_sanity` - Qt Graphs performance test

## Notes

- On dev-01: Tests can be built and run
- On macOS: `BUILD_TESTING=OFF` currently used due to libsodium/test harness work-in-progress
- Qt Graphs requires Qt 6.10+ (GraphsWidgets module)
```

**Key Points:**
- Keep it practical and accurate
- Note platform-specific differences
- Reference actual build targets

---

### Step 4: Phoenix Plotting ADR

**File:** `docs/adr/ADR-S4.2-Plotting-01.md`

**Structure:**
```markdown
# ADR-S4.2-Plotting-01: Qt Graphs as Unified Plotting Backend

**Status:** Accepted  
**Date:** 2025-11-18  
**Sprint:** 4.2

## Context

Phoenix needs plotting capabilities for:
- 2D XY plots (line, scatter)
- Future 3D visualizations
- Heat maps and grid data

## Decision Drivers

- Need unified 2D/3D plotting solution
- Want official Qt support (commercial license)
- Avoid licensing ambiguity
- Modern, maintainable API

## Considered Options

### Option 1: QCustomPlot (Rejected)
- **Pros:** Mature, C++ native, no QML required
- **Cons:** Third-party, licensing ambiguity, not officially supported, 2D only
- **Status:** Prototyped in Sprint 4.2, removed in WP1.A Chunk 2

### Option 2: Qt Charts (Rejected)
- **Pros:** Official Qt module
- **Cons:** Legacy API, limited 3D support, maintenance concerns

### Option 3: Qt Graphs (Chosen)
- **Pros:** Modern, unified 2D/3D, officially supported, covered by Qt license
- **Cons:** Requires QML/QQuickWidget integration, offscreen testing is trickier
- **Status:** Implemented in Sprint 4.2 WP1.A

## Decision

Phoenix uses **Qt Graphs (Qt 6.10+)** as the unified plotting backend:
- 2D plots via QQuickWidget + QML (XYPlotViewGraphs)
- Future 3D plots via same stack
- Minimum Qt version: 6.10.0

## Implementation

- `XYPlotViewGraphs` implements `IAnalysisView`
- Uses `QQuickWidget` to embed QML scene
- QML file: `src/qml/XYPlotView.qml` (GraphsView + LineSeries)
- Performance: 10k points update in ~1 ms (exceeds 30 FPS target)

## Consequences

### Positive
- Unified 2D/3D stack
- Official Qt support
- No extra licensing concerns
- Modern API

### Negative
- QML integration complexity
- Offscreen test harness limitations (known issue)
- Requires Qt 6.10+ (newer than some systems)

## Follow-ups

- Refine QML for better pan/zoom/axes (WP1.A Chunk 3R done)
- Add scatter plot view (future)
- Add 3D visualization support (future)
- Improve offscreen test harness (future)
```

**Key Points:**
- Follow ADR format
- Document decision rationale
- Note QCustomPlot removal
- Reference actual performance numbers

---

### Step 5: Bedrock Architecture Overview

**File:** `docs/architecture/overview.md`

**Structure:**
```markdown
# Bedrock Architecture Overview

## Purpose

Bedrock is a C++20 analysis engine that provides computational capabilities for Phoenix and other clients.

## High-Level Architecture

### Core Components

#### bedrock_engine Library
- Core analysis engine (static library)
- C++20 standard (verify actual CMakeLists.txt)
- Minimal Phase 0 implementation

#### bedrock_sanity Executable
- Basic sanity test
- Validates engine initialization
- Part of CTest suite

#### Transport Support
- gRPC over Unix Domain Socket (UDS)
- LocalSocket fallback
- Palantir contracts for RPC definitions
- Conditional: `BEDROCK_WITH_TRANSPORT_DEPS` option

### Relationship to Phoenix

- Bedrock provides analysis capabilities
- Phoenix consumes Bedrock via transport layer
- Shared contracts ensure compatibility

### Relationship to Palantir Contracts

- `contracts/` submodule contains shared proto definitions
- Contract parity enforced in CI
- `.contract-version` tracks contract SHA

## Current Sprint 4.2 State

- ✅ C++23 reset complete
- ✅ Minimal engine library
- ✅ Sanity test passing
- ✅ Echo RPC implemented and tested
- ✅ Transport dependencies optional (BEDROCK_WITH_TRANSPORT_DEPS)
- ⏳ Full feature implementations planned (WP4+)
```

**Key Points:**
- Document actual Phase 0 state
- Note modular CMake structure
- Reference transport option

---

### Step 6: Bedrock Transport Documentation

**File:** `docs/architecture/transport.md`

**Structure:**
```markdown
# Bedrock Transport Architecture

## Design

Bedrock provides analysis capabilities via RPC transport:
- Primary: gRPC over Unix Domain Socket (UDS)
- Fallback: LocalSocket (plain socket)
- Contracts: Palantir proto definitions

## Transport Modes

### gRPC UDS (Primary)
- High performance
- Type-safe via protobuf
- Unix Domain Socket for local communication

### LocalSocket (Fallback)
- Simpler, no gRPC dependency
- Plain socket communication
- Used when gRPC unavailable

## Current Implementation

### Echo RPC
- Basic echo service implemented
- `bedrock_echo_sanity` test validates functionality
- Proto generation working (`proto/bedrock_echo.proto`)

### Dependencies

Transport dependencies are optional:
- `BEDROCK_WITH_TRANSPORT_DEPS=ON` enables:
  - Protobuf
  - gRPC
  - libsodium (optional)

### Build Configuration

```cmake
option(BEDROCK_WITH_TRANSPORT_DEPS "Enable transport dependencies" OFF)
```

When enabled:
- Finds Protobuf (CONFIG or MODULE mode)
- Finds gRPC (CONFIG or MODULE mode)
- Finds libsodium (optional)
- Generates proto code
- Builds transport sanity targets

## Future Work

- Full feature RPCs (XY Sine, etc.)
- Streaming results
- Progress reporting
- Cancellation support
```

**Key Points:**
- Document actual transport design
- Note optional dependencies
- Reference current Echo RPC implementation

---

### Step 7: Bedrock Build Documentation

**File:** `docs/build/bedrock_build.md`

**Structure:**
```markdown
# Building Bedrock

## Requirements

- C++23-capable compiler (GCC 11+, Clang 14+, MSVC 2022+)
- CMake 3.20 or later
- (Optional) Protobuf, gRPC, libsodium for transport

## Quick Start

### Basic Build (No Transport)

```bash
git clone <repo-url>
cd bedrock
git submodule update --init --recursive  # For contracts/
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### Build with Transport Dependencies

```bash
cmake -S . -B build \
  -DBUILD_TESTING=ON \
  -DBEDROCK_WITH_TRANSPORT_DEPS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Build Options

- `BUILD_TESTING` - Enable tests (default: ON)
- `BEDROCK_WITH_TRANSPORT_DEPS` - Enable transport deps (default: OFF)

## Build Targets

- `bedrock_engine` - Core engine library (static)
- `bedrock_sanity` - Basic sanity test
- `bedrock_transport_deps_sanity` - Transport deps test (if enabled)
- `bedrock_echo_sanity` - Echo RPC test (if transport enabled)

## CMake Structure

Bedrock uses modular CMake:
- `cmake/Options.cmake` - Build options
- `cmake/Dependencies.cmake` - Dependency discovery
- `cmake/Targets.cmake` - Target definitions

## Notes

- Contract submodule must be initialized for proto generation
- Transport dependencies are optional (can build without)
- Tests run on dev-01 with BUILD_TESTING=ON
```

**Key Points:**
- Document modular CMake structure
- Note optional transport dependencies
- Reference actual build targets

---

## Implementation Order

### Phoenix
1. Check out sprint/4.2 branch
2. Create `docs/architecture/` directory
3. Create `docs/build/` directory
4. Create `docs/adr/` directory
5. Write `docs/architecture/overview.md`
6. Write `docs/architecture/analysis_loop.md`
7. Write `docs/build/phoenix_build.md`
8. Write `docs/adr/ADR-S4.2-Plotting-01.md`
9. Commit and push

### Bedrock
1. Check out sprint/4.2 branch
2. Create `docs/architecture/` directory
3. Create `docs/build/` directory
4. Write `docs/architecture/overview.md`
5. Write `docs/architecture/transport.md`
6. Write `docs/build/bedrock_build.md`
7. Commit and push

---

## Verification Checklist

**Before Writing:**
- ✅ Verify branch is sprint/4.2
- ✅ Check existing docs structure
- ✅ Verify component existence
- ✅ Understand current state vs planned

**After Writing:**
- ✅ All files created in correct locations
- ✅ Content matches actual codebase
- ✅ No aspirational claims (only current state)
- ✅ ADR follows standard format
- ✅ Build docs are accurate
- ✅ Commits pushed to sprint/4.2

---

## Success Criteria

- ✅ Phoenix docs created (4 files)
- ✅ Bedrock docs created (3 files)
- ✅ All docs are skeletal but accurate
- ✅ ADR documents Qt Graphs decision clearly
- ✅ Build docs are practical and usable
- ✅ Architecture docs match current Sprint 4.2 state
- ✅ Commits on sprint/4.2 branches
- ✅ Pushed to origin/sprint/4.2

---

## Notes

- **Keep it short** - Skeletal docs, not comprehensive
- **Match reality** - Document what exists, not what's planned
- **Be truthful** - Mark stubs/TODOs clearly
- **Reference sprints** - Note when features are planned (WP2, WP3, etc.)
- **No images** - Use markdown lists/diagrams only

