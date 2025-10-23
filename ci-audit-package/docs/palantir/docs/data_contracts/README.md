# Phoenix ↔ Bedrock Data Contracts

**Version:** 1.0.0 | **Status:** Stable (Phase 2)

**Scope:** Plotting/visualization datasets and control flows for Phoenix (Qt UI) consuming Bedrock (C++) outputs over Arrow Flight (data) + gRPC/DoAction (control).

## Contents
- `shared_conventions.md`: Versioning, units, coords, stats sidecar, backpressure.
- `phoenix_extensions_schema.md`: Valid `phoenix:{}` block for Vega-Lite specs.
- Dataset contracts:
  1. `01_mtf_curves.md`
  2. `02_spot_diagrams.md`
  3. `03_wavefront_maps.md`
  4. `04_tolerancing_histograms.md`
  5. `05_ray_fans.md`
- `validation/`: CI checklist + starter tests.
- `examples/`: Tiny illustrative Vega-Lite specs.

## Transport & Binding (summary)
- **Spec:** Vega-Lite JSON (renderer-agnostic) + optional `phoenix:{}`.
- **Data:** Arrow Flight streams (RecordBatches) or shared memory (hot path).
- **Binding:** VL `data.name` → dataset ID (Flight descriptor). Phoenix resolves and feeds buffers to the renderer.
- **Exports/CI:** Phoenix can request Bedrock SVG/PDF for canonical artifacts.

## Renderer Selection (default policy)
- Interactive (small/medium) → Vega-Lite.
- Dense scatter → WebGL scatter.
- Huge heatmaps/PSF → tiled Flight + heatmap; export via Bedrock SVG/PNG.
- 3D/layouts → Qt3D + Bedrock SVG snapshot for reports.
