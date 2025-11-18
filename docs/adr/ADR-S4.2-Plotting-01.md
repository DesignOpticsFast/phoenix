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

