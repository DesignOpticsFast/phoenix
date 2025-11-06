# Feature Manager (Phoenix facade)

A non-blocking, cancellable, telemetry-aware boundary between Phoenix and Bedrock. Communicates via Palantir IPC layer.

## Goals

- Zero GUI-thread blocking (async only).
- Coalesce rapid changes; cancel stale jobs.
- Enforce licensing at the call site.
- Emit progress and results via signals.

## API (UI-facing sketch)

- `request(feature_id, params, exec_id, priority) -> ticket`
- Signals: `started(job_id)`, `progress(job_id, %)`, `finished(job_id, payload, meta)`, `failed(job_id, code, reason)`
- `cancel(ticket)`, `cancelByWindow(ctx)`, `setThrottle(ctx, ms)`

## Threading

- GUI thread: dispatches requests; never blocks.
- Dispatcher thread: licensing + telemetry; forwards to Bedrock.
- Bedrock threadpool: performs compute; emits queued signals back.

## Licensing & Telemetry

- License gate by `feature_id` (stub OK for MVP).
- Telemetry per call: feature_id, params-hash, duration, success (opt‑in; no PII). See `docs/telemetry-policy.md`.

## Payloads

Results are typed blobs (e.g., FITS/HDF5/binary) plus small JSON/metadata suitable for charts/tables. Phoenix applies only view-local transforms.
