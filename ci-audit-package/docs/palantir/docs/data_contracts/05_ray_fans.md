# Ray Fan Diagrams

**Version:** 1.0.0 | **Status:** Stable | **Last Updated:** 2025-10-07

## Purpose
Transverse ray aberration vs normalized pupil coordinate; classic aberration diagnostic.

## Dataset ID
`optics/<proj>/<sess>/rayfan/<orientation>`

## Arrow Schema
```
pupil_coord:   float32   // [-1..1]
transverse_um: float32   // image-plane transverse aberration
field_deg:     float32
wavelength_nm: int16
orientation:   utf8      // "tangential" | "sagittal" | "x" | "y"
coordinate_sys:utf8      // "object_space" | "image_space"
```

## Required Metadata
- `phoenix_version`
- Units: `units:transverse_um=µm`
- `chief_ray_reference: bool`

## Invariants
- `pupil_coord ∈ [-1,1]`
- Chief (0) ≈ 0 µm when referenced to chief ray

## Typical Performance (defaults)
```json
{"typical_performance": {"typical_rows": 1000, "max_rows": 20000, "expected_update_hz": 5}}
```

## Minimal Vega-Lite
```json
{
  "$schema": "https://vega.github.io/schema/vega-lite/v5.json",
  "data": {
    "name": "rayfan"
  },
  "mark": "line",
  "encoding": {
    "x": {
      "field": "pupil_coord",
      "type": "quantitative",
      "scale": {
        "domain": [
          -1,
          1
        ]
      },
      "title": "Pupil"
    },
    "y": {
      "field": "transverse_um",
      "type": "quantitative",
      "title": "Transverse (\u00b5m)"
    },
    "color": {
      "field": "wavelength_nm",
      "type": "nominal"
    }
  }
}
```

## Changelog
- 1.0.0 — Initial stable contract (orientation separated from coord system).
