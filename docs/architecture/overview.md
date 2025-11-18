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

