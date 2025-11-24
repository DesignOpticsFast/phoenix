# ADR 0001: Feature boundary & Palantir IPC layer

**Status:** Accepted  
**Date:** 2025-10-03  
**Updated:** 2025-11-24

## Context

Phoenix must remain highly responsive. Editors, Analysis Windows, and Tools are UI controllers; computation occurs in Bedrock via the Palantir IPC layer. We need a clear boundary that avoids blocking the GUI thread and keeps model state centralized.

## Decision

- **Architecture**: Phoenix UI → **Palantir IPC** → Bedrock compute
- **PalantirClient** provides the feature boundary with:
  - Non-blocking connection state machine (FSM: Idle → Connecting → Connected → ErrorBackoff → PermanentFail)
  - Framed binary protocol (PLTR magic, BigEndian, version/type/length header)
  - Exponential backoff for connection errors (1s, 2s, 4s, 8s, 16s, max 5 attempts)
  - Message dispatcher (type→handler registration) for protocol frames
- **Execution Copies (ECs)** are created, owned, and managed **only by Bedrock** (via Palantir).
- Phoenix invokes features via **PalantirClient** which is **asynchronous and event-driven**; the GUI thread never blocks.
- **Analysis Windows** specify a feature + params and render **typed results** (e.g., spot data arrays). View-local transforms (log scale, contouring) are UI-only.
- **Tools** operate on Bedrock-held ECs via Palantir; **commit()** promotes EC → canonical SOM and emits events; **discard()** drops it.
- **Phoenix never spawns Bedrock**; the Palantir service is managed separately.

## Consequences

- UI code remains simple and responsive.
- All compute and copy semantics are centralized in Bedrock (via Palantir).
- Licensing/telemetry can be enforced cleanly at the Palantir boundary.
- Connection failures are handled gracefully with exponential backoff.
- Protocol framing ensures safe, versioned IPC communication.

## References

- `docs/feature-manager.md`
- `docs/DEVELOPMENT.md` (Palantir IPC Client section)
- Bedrock `docs/engine-api.md`
- Rosetta `docs/som-overview.md`
