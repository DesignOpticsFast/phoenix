# ADR 0001: Feature boundary & Execution Copy residency

**Status:** Accepted  
**Date:** 2025-10-03

## Context
Phoenix must remain highly responsive. Editors, Analysis Windows, and Tools are UI controllers; Bedrock performs all computation. We need a clear boundary that avoids blocking the GUI thread and keeps model state centralized.

## Decision
- **Execution Copies (ECs)** are created, owned, and managed **only by Bedrock**.
- Phoenix invokes Bedrock via a **Feature Manager** facade that is **asynchronous, cancellable, and coalescing**; the GUI thread never blocks.
- **Analysis Windows** specify a feature + params and render **typed results** (e.g., spot data arrays). View-local transforms (log scale, contouring) are UI-only.
- **Tools** operate on a Bedrock-held EC; **commit()** promotes EC → canonical SOM and emits a “SOMChanged” event; **discard()** drops it.
- The boundary is transport-agnostic: in-process now; IPC later without changing the UI API.

## Consequences
- UI code remains simple and responsive.
- All compute and copy semantics are centralized in Bedrock.
- Licensing/telemetry can be enforced cleanly at the Feature boundary.

## References
- `docs/feature-manager.md`
- Bedrock `docs/engine-api.md`
- Rosetta `docs/som-overview.md`
