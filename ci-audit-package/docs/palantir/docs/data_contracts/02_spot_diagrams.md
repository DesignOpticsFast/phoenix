# Spot Diagrams

**Version:** 1.0.0 | **Status:** Stable | **Last Updated:** 2025-10-07

## Purpose
Ray intercept distribution at image plane; quick blur/aberration visualization.

## Dataset ID
`optics/<proj>/<sess>/spot/<field_label>`

## Arrow Schema
```
x_um:          float32
y_um:          float32
field_deg:     float32
wavelength_nm: int16
ray_class:     int8      // dict: 0=chief,1=marginal,2=regular
sample_id:     int32     // tolerancing id (optional)
```

## Required Metadata
- `phoenix_version`
- Units: `units:x_um=µm`, `units:y_um=µm`

## Optional Metadata
- `image_scale_um_per_px`, `stop_surface_index`
- `rms_radius_um`, `encircled_energy_80_um`
- `centroid_x_um`, `centroid_y_um`

## Invariants
- Finite `x_um`,`y_um`; ≤1 chief per (field, λ).

## Typical Performance (defaults)
```json
{"typical_performance": {"typical_rows": 5000, "max_rows": 100000, "expected_update_hz": 10}}
```

## Instance Overrides
```json
{"performance_profile": {"actual_rows": 15000, "update_hz": 8, "decimation_threshold": 10000}}
```

## Minimal Vega-Lite
```json
{
  "$schema": "https://vega.github.io/schema/vega-lite/v5.json",
  "data": {
    "name": "spot_points"
  },
  "mark": {
    "type": "point",
    "filled": true,
    "size": 10
  },
  "encoding": {
    "x": {
      "field": "x_um",
      "type": "quantitative",
      "title": "X (\u00b5m)"
    },
    "y": {
      "field": "y_um",
      "type": "quantitative",
      "title": "Y (\u00b5m)"
    },
    "color": {
      "field": "wavelength_nm",
      "type": "nominal"
    }
  },
  "transform": [
    {
      "filter": "isValid(datum.x_um) && isValid(datum.y_um)"
    }
  ]
}
```

## Renderer Selection
| Condition | Renderer |
|---|---|
| ≤ 10k points | Vega-Lite scatter |
| 10k–100k | WebGL scatter |
| >100k | Server raster/heatmap (tiles) |

## Failure Modes
- Huge point clouds → switch renderer + show `DECIMATED` badge.
- Units mismatch → Phoenix converts or warns.

## Changelog
- 1.0.0 — Initial stable contract.
