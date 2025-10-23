# CI Validation Checklist

## Schema & Metadata
- [ ] `phoenix_version` present and major compatible
- [ ] Required columns exist with correct Arrow types
- [ ] Units metadata present for axis fields
- [ ] Provenance: `bedrock_version`, `calc_time`, `config_hash`

## Stats Sidecar
- [ ] First batch includes stats sidecar under metadata key `phoenix.stats`
- [ ] No NaN/Inf; ranges sane for domain
- [ ] Change-triggered stats updates applied (>10% min/max delta)

## Invariants (per dataset)
- MTF: 0 ≤ mtf ≤ 1; freq ≥ 0
- Spot: finite coords; ≤1 chief per field/λ
- Wavefront: mean≈0 (piston removed); PSF ≥ 0; normalization consistent
- Tolerancing: Σcount = samples_total
- Ray Fan: pupil ∈ [-1,1]; chief≈0 µm

## Performance & Backpressure
- [ ] Honor `decimation_threshold` / renderer auto-switch logged
- [ ] `viewport`, `rate`, `decimate`, `pause`, `resume`, `release` handled
- [ ] ACKs returned (`ack_seq`) with status and applied params

## Exports
- [ ] Bedrock SVG/PDF export succeeds for each plot template
- [ ] Golden image diffs within tolerance
