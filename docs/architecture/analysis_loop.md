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

