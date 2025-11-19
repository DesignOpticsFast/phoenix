# 🧠 UnderLord Prompt — WP1.D Chunk 1 – Documentation Skeleton

## Intent

Add minimal but accurate architecture/build docs and a plotting ADR for Qt Graphs, on the `sprint/4.2` branches of both repos:
- **Phoenix:** architecture overview, analysis loop doc, build doc, and ADR for Qt Graphs
- **Bedrock:** architecture overview, transport doc, and build doc

The docs can be skeletal, but they must match current Sprint 4.2 reality, not aspirational future plans.

---

## Context

- **Phoenix repo:** `/home/ec2-user/workspace/phoenix`
- **Bedrock repo:** `/home/ec2-user/workspace/bedrock`
- **Branch:** `sprint/4.2` (all Sprint 4.2 work happens here)
- **Existing docs:** Both repos have some docs, but missing architecture/build skeletons
- **Goal:** Create foundational docs that accurately reflect Sprint 4.2 state

**Critical:** Match actual codebase state, not future plans. Mark stubs/TODOs clearly.

---

## Ask

### Part 1: Phoenix Documentation

#### Step 1: Check Out Sprint Branch

```bash
cd /home/ec2-user/workspace/phoenix
git fetch
git checkout sprint/4.2
git pull
git branch -vv
```

**Confirm:** `sprint/4.2` is checked out and tracking `origin/sprint/4.2`

---

#### Step 2: Create Directory Structure

```bash
mkdir -p docs/architecture
mkdir -p docs/build
mkdir -p docs/adr
```

---

#### Step 3: Create `docs/architecture/overview.md`

**Content should include:**

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
- Document actual components (MainWindow, AnalysisWindow, XYPlotViewGraphs)
- Note what's complete vs stubbed
- Keep it factual, not aspirational

---

#### Step 4: Create `docs/architecture/analysis_loop.md`

**Content should include:**

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

#### Step 5: Create `docs/build/phoenix_build.md`

**Content should include:**

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

#### Step 6: Create `docs/adr/ADR-S4.2-Plotting-01.md`

**Follow existing ADR format** (check `docs/adr/0001-feature-boundary-palantir.md` for style)

**Content should include:**

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
- Follow existing ADR format
- Document decision rationale
- Note QCustomPlot removal
- Reference actual performance numbers

---

#### Step 7: Commit and Push (Phoenix)

```bash
git add docs/architecture/overview.md \
        docs/architecture/analysis_loop.md \
        docs/build/phoenix_build.md \
        docs/adr/ADR-S4.2-Plotting-01.md

git commit -m "WP1.D Chunk 1: add Phoenix architecture/build docs and plotting ADR"

git push --no-verify origin sprint/4.2
```

---

### Part 2: Bedrock Documentation

#### Step 1: Check Out Sprint Branch

```bash
cd /home/ec2-user/workspace/bedrock
git fetch
git checkout sprint/4.2
git pull
git branch -vv
```

**Confirm:** `sprint/4.2` is checked out and tracking `origin/sprint/4.2`

---

#### Step 2: Create Directory Structure

```bash
mkdir -p docs/architecture
mkdir -p docs/build
```

---

#### Step 3: Create `docs/architecture/overview.md`

**Content should include:**

```markdown
# Bedrock Architecture Overview

## Purpose

Bedrock is a C++20 analysis engine that provides computational capabilities for Phoenix and other clients.

**Note:** Verify actual C++ standard in CMakeLists.txt - currently shows C++20, document what's actually configured.

## High-Level Architecture

### Core Components

#### bedrock_engine Library
- Core analysis engine (static library)
- C++20 standard (verify from CMakeLists.txt - currently shows C++20)
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

- ✅ C++20 reset complete (verify actual standard from CMakeLists.txt)
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

#### Step 4: Create `docs/architecture/transport.md`

**Content should include:**

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

#### Step 5: Create `docs/build/bedrock_build.md`

**Content should include:**

```markdown
# Building Bedrock

## Requirements

- C++20-capable compiler (GCC 11+, Clang 14+, MSVC 2019+)
- **Note:** Verify actual C++ standard requirement from CMakeLists.txt
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

#### Step 6: Commit and Push (Bedrock)

```bash
git add docs/architecture/overview.md \
        docs/architecture/transport.md \
        docs/build/bedrock_build.md

git commit -m "WP1.D Chunk 1: add Bedrock architecture and build docs"

git push --no-verify origin sprint/4.2
```

---

## Reporting

**For Phoenix:**
- File paths created
- 1-2 line description of each file
- Commit hash and branch confirmation

**For Bedrock:**
- File paths created
- 1-2 line description of each file
- Commit hash and branch confirmation

---

## Definition of Done

- ✅ Phoenix docs created (4 files: overview, analysis_loop, build, ADR)
- ✅ Bedrock docs created (3 files: overview, transport, build)
- ✅ All docs are skeletal but accurate
- ✅ ADR documents Qt Graphs decision clearly
- ✅ Build docs are practical and usable
- ✅ Architecture docs match current Sprint 4.2 state
- ✅ Commits on sprint/4.2 branches
- ✅ Pushed to origin/sprint/4.2

---

## Important Constraints

1. **Match reality** - Document what exists, not what's planned
2. **Mark stubs clearly** - Use ⏳ or "stub" / "WP2" markers
3. **Keep it short** - Skeletal docs, not comprehensive
4. **Be truthful** - No aspirational claims
5. **Reference sprints** - Note when features are planned (WP2, WP3, etc.)

---

## Notes

- **No images** - Use markdown lists/diagrams only
- **Check existing ADRs** - Match format of existing ADR files
- **Verify components** - Ensure documented components actually exist
- **Keep it simple** - These are foundation docs, not comprehensive guides

