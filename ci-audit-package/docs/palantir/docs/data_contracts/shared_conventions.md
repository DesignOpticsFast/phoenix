# Shared Conventions

**Phoenix schema version key**  
Every Arrow schema must include:
- `phoenix_version`: semantic string, e.g. `"1.0.0"` (major must match client)
- `bedrock_version`, `calc_time`, `config_hash` (provenance)

**Units**  
- Numeric columns remain numeric.  
- Units live in schema metadata: `units:<field>=<unit>` (e.g., `units:frequency_cyc_per_mm=cycles/mm`).

**Coordinate systems**
- **Image space (spot/PSF):** origin = optical axis × image plane; +X right, +Y up; default units µm.
- **Pupil space (wavefront):** normalized radius=1 at edge; origin at pupil center; +X horizontal, +Y vertical; obscurations in metadata.
- **Field:** `field_deg` signed; optional `field_x_deg`, `field_y_deg` (object-space).

**Performance Profiles**

**Documentation defaults** live in each contract under `typical_performance`.

**Instance overrides** in dataset metadata (`performance_profile`):
```json
{
  "performance_profile": {
    "actual_rows": 12345,
    "update_hz": 10,
    "decimation_threshold": 10000
  }
}
```

Phoenix uses instance values when present; otherwise defaults.

**Stats Sidecar (delivery & updates)**
- **Attachment key name:** `phoenix.stats` (Arrow Flight metadata key)
- **Mechanism:** attach JSON in Arrow Flight *metadata* on the **first** `RecordBatch` and whenever stats change materially (>10% min/max delta), invalids detected, or on explicit request.
Example JSON:
```json
{
  "dataset_id":"…",
  "row_count":12345,
  "byte_size":456789,
  "field_stats":{
    "mtf":{"min":0.0,"max":0.98,"mean":0.61,"stddev":0.12,"nulls":0,"invalid":false}
  }
}
```
- **Explicit request:** Phoenix may send `{"action":"get_stats","dataset_id":"…"}` via DoAction.

**Backpressure / Actions (JSON via Flight DoAction)**

Phoenix → Bedrock (include `seq` for ACK correlation):
```json
{"seq":42,"action":"viewport","dataset_id":"…","x_min":0,"x_max":200,"y_min":0,"y_max":1,"pixel_w":1200,"pixel_h":400}
{"seq":43,"action":"rate","dataset_id":"…","fps":30}
{"seq":44,"action":"decimate","dataset_id":"…","strategy":"PERCEPTUAL","target_points":5000}
{"seq":45,"action":"pause","dataset_id":"…"}
{"seq":46,"action":"resume","dataset_id":"…"}
{"seq":47,"action":"release","dataset_id":"…"}
```

**Backpressure ACK delivery**
- **Option A (preferred for streaming):** attach JSON under `phoenix.ack` on the next RecordBatch metadata:
```json
{"ack_seq":42,"status":"applied"}
```
- **Option B (for immediate ops like pause/resume):** return DoAction `Result` with the same JSON body.

**Renderer State Badges**

Phoenix displays plot state in UI:

| Badge | Condition | Color |
|---|---|---|
| LIVE | Receiving Flight updates < 1s old | Green |
| DECIMATED | Displaying < 100% of points | Yellow |
| KEYFRAMES | Animation/optimization playback | Blue |
| STALE | No updates > 5s; dataset may be old | Gray |

**Badge priority:** STALE > DECIMATED > KEYFRAMES > LIVE
