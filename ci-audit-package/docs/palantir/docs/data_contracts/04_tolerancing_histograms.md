# Tolerancing Histograms

**Version:** 1.0.0 | **Status:** Stable | **Last Updated:** 2025-10-07

## Purpose
Distribution of operand values across Monte Carlo samples; pass/fail vs specs.

## Dataset ID
`optics/<proj>/<sess>/tolerancing/<operand_key>/hist`

## Arrow Schema (pre-binned)
```
bin_center:  float32
count:       int32
operand_key: int16
units:       utf8
```

### Optional raw samples
```
sample_id:   int32
value:       float32
operand_key: int16
```

## Required Metadata
- `phoenix_version`
- `samples_total`, `bins`, `bin_width`
- Optional: `spec_limit_low`, `spec_limit_high`, `target`

## Optional Metadata (metrics)
- `percentile_10`, `percentile_50`, `percentile_90`, `pass_rate`

## Invariants
- `count ≥ 0`, `Σcount = samples_total`

## Typical Performance (defaults)
```json
{"typical_performance": {"typical_rows": 50, "max_rows": 200, "expected_update_hz": 1}}
```

## Minimal Vega-Lite
```json
{
  "$schema": "https://vega.github.io/schema/vega-lite/v5.json",
  "data": {
    "name": "tol_hist"
  },
  "mark": "bar",
  "encoding": {
    "x": {
      "field": "bin_center",
      "type": "quantitative",
      "title": "Operand"
    },
    "y": {
      "field": "count",
      "type": "quantitative",
      "title": "Count"
    }
  },
  "layer": [
    {
      "mark": "bar"
    },
    {
      "mark": {
        "type": "rule",
        "strokeDash": [
          4,
          4
        ]
      },
      "encoding": {
        "x": {
          "datum": "spec_limit_low"
        }
      }
    },
    {
      "mark": {
        "type": "rule",
        "strokeDash": [
          4,
          4
        ]
      },
      "encoding": {
        "x": {
          "datum": "spec_limit_high"
        }
      }
    }
  ]
}
```

## Changelog
- 1.0.0 — Initial stable contract.
