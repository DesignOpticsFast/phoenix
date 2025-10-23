# MTF Curves

**Version:** 1.0.0 | **Status:** Stable | **Last Updated:** 2025-10-07

## Purpose
Modulation Transfer Function vs spatial frequency, optionally across wavelengths, field positions, and orientations.

## Dataset ID Pattern
`optics/<proj>/<sess>/mtf/<series>` (e.g., `onaxis`, `tan_sag`, `through_focus`)

## Arrow Schema
```
frequency_cyc_per_mm: float64
mtf:                  float32
wavelength_nm:        int16     // dict-encoded if few values
field_deg:            float32
series_id:            int32     // e.g., 0=tan,1=sag,2=mean
```

## Required Metadata
- `phoenix_version`: "1.0.0"
- Units: `units:frequency_cyc_per_mm=cycles/mm`, `units:mtf=ratio`

## Optional Metadata
- `nyquist_cyc_per_mm`, `sensor_pitch_um`
- `diffraction_limit: bool` (overlay theoretical curve)
- `defocus_mm: float32` (for through-focus series)
- `field_label: utf8`
- `orientation: int8` (0=tan,1=sag,2=mean)

## Invariants
- `0 ≤ mtf ≤ 1`, `frequency_cyc_per_mm ≥ 0`, no NaN/Inf.

## Typical Performance (defaults)
```json
{"typical_performance": {"typical_rows": 5000, "max_rows": 50000, "expected_update_hz": 5}}
```

## Instance Overrides
```json
{"performance_profile": {"actual_rows": 12345, "update_hz": 10, "decimation_threshold": 10000}}
```

## Minimal Vega-Lite
```json
{
  "$schema": "https://vega.github.io/schema/vega-lite/v5.json",
  "data": {
    "name": "mtf_data"
  },
  "mark": "line",
  "encoding": {
    "x": {
      "field": "frequency_cyc_per_mm",
      "type": "quantitative",
      "title": "Spatial Frequency (cycles/mm)"
    },
    "y": {
      "field": "mtf",
      "type": "quantitative",
      "title": "MTF",
      "scale": {
        "domain": [
          0,
          1
        ]
      }
    },
    "color": {
      "field": "wavelength_nm",
      "type": "nominal"
    },
    "strokeDash": {
      "field": "series_id",
      "type": "nominal"
    }
  },
  "phoenix": {
    "version": "1.0.0",
    "optical": {
      "units": {
        "frequency_cyc_per_mm": "cycles/mm",
        "mtf": "ratio"
      }
    }
  }
}
```

## Renderer Selection
| Condition | Renderer |
|---|---|
| ≤ 50k rows | Vega-Lite |
| > 50k rows | Server decimation + Vega-Lite |
| Export | Bedrock SVG/PDF |

## Failure Modes & Mitigation
- Out-of-range MTF: clamp + UI flag; log invalids in stats.
- Missing Nyquist: compute from `sensor_pitch_um` if present; else omit guide.

## Changelog
- 1.0.0 — Initial stable contract.
