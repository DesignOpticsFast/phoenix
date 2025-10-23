# Wavefront / PSF Maps

**Version:** 1.0.0 | **Status:** Stable | **Last Updated:** 2025-10-07

## Purpose
2D scalar fields: wavefront OPD over pupil; PSF intensity over image plane.

## Dataset IDs
- Wavefront: `optics/<proj>/<sess>/wavefront/pupil_map`
- PSF: `optics/<proj>/<sess>/psf/image_map`

## Arrow Schema (tiled stream)
```
tile_x:   int16
tile_y:   int16
row:      int16
col:      int16
value:    float32   // OPD (waves) or intensity
layer_id: int8      // optional multi-layer
```

## Required Metadata
- `phoenix_version`
- Wavefront: `units:value=waves`, `ref_wavelength_nm`
- PSF: `units:value=dimensionless`, `normalized_to_peak=true|false`
- Grid: `{tile_rows,tile_cols,tile_height,tile_width}`

## Optional Metadata
- Pupil: `shape`, `epsilon`, `obscurations`, `apodization`, `FFT_padding`
- Metrics: `zernike_coefficients[36]`, `pv_waves`, `rms_waves`, `strehl_ratio`

## Invariants
- Wavefront mean (after piston removal) ≈ 0 (|mean| < ε)
- PSF ≥ 0; if normalized, max≈1
- No NaN/Inf; mask outside pupil (mask stream optional)

## Typical Performance (defaults)
```json
{"typical_performance": {"typical_rows": 65536, "max_rows": 4194304, "expected_update_hz": 1}}
```

## Minimal Vega-Lite (heatmap)
```json
{
  "$schema": "https://vega.github.io/schema/vega-lite/v5.json",
  "data": {
    "name": "wf_tiles"
  },
  "mark": "rect",
  "encoding": {
    "x": {
      "field": "col",
      "type": "ordinal",
      "title": null
    },
    "y": {
      "field": "row",
      "type": "ordinal",
      "title": null
    },
    "fill": {
      "field": "value",
      "type": "quantitative",
      "title": "OPD (waves)"
    }
  },
  "config": {
    "view": {
      "stroke": null
    }
  },
  "phoenix": {
    "version": "1.0.0",
    "optical": {
      "units": {
        "value": "waves"
      }
    }
  }
}
```

## Renderer Selection
- Tiled Vega-Lite for mid sizes; server raster tiles for huge/real-time.
- Export via Bedrock SVG/PNG (fixed scale bar).

## Changelog
- 1.0.0 — Initial stable contract.
