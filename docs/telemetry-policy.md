# Telemetry Policy (Phoenix)

Phoenix records **Feature** usage at the UI↔Bedrock boundary to improve UX and performance.

## Principles

- **Opt‑in** (default off in developer builds).
- **No PII** and no project/lens contents.
- Local buffering and periodic upload.
- Clear in-app controls to view/opt-out.

## Events

- `feature.requested` (feature_id, params_hash)
- `feature.finished` (duration_ms, success, payload_size)
- `feature.failed` (error_code, error_reason)

Transport is pluggable: start with local NDJSON logs; add HTTPS batch upload later.
