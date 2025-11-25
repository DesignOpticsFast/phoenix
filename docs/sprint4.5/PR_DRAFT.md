# Draft PR: Sprint 4.5 — Envelope Transport Migration & IPC Hardening

**Title:** Sprint 4.5 — Envelope Transport Migration & IPC Hardening

**Branch:** `sprint/4.5-envelope-migration` → `main`

**Type:** Feature / Hardening

---

## Summary

This PR implements Sprint 4.5, which hardens the Phoenix ↔ Bedrock IPC (Palantir) transport layer through envelope-based protocol migration, deadlock elimination, and comprehensive integration testing.

---

## Changes Summary

### Core Changes

1. **Envelope-Based Protocol Migration**
   - Migrated from legacy `[length][type][payload]` to `[length][MessageEnvelope]` format
   - Implemented `MessageEnvelope` protobuf with version, type, payload, metadata fields
   - Removed all legacy framing code
   - Added envelope helpers (`makeEnvelope`, `parseEnvelope`) on both Phoenix and Bedrock

2. **Deadlock Elimination**
   - Refactored `parseIncomingData()` to use `extractMessage()` helper (no locking)
   - Narrowed lock scope to buffer manipulation only
   - Removed mutex from `sendMessage()` to prevent nested locking
   - Transport layer now deadlock-free and safe under concurrency

3. **Integration Test Harness**
   - Created `IntegrationTestServerFixture` for in-process server testing
   - Implemented `IntegrationTestClient` as minimal test client
   - Added Capabilities RPC integration test (✅ passing)
   - Added XY Sine RPC integration test (✅ passing)

4. **Documentation**
   - Created ADR-0002 for envelope-based framing decision
   - Added comprehensive integration test documentation
   - Created IPC protocol specification
   - Updated all sprint docs with completion markers

5. **Versioning**
   - Bumped Phoenix version to 0.0.4
   - Updated CHANGELOG.md files for both Phoenix and Bedrock

---

## Test Results

### Unit Tests
- ✅ Phoenix envelope helpers: 14/14 passing
- ✅ Bedrock envelope helpers: 15/15 passing

### Integration Tests
- ✅ Capabilities RPC: PASS (330ms)
- ✅ XY Sine RPC: PASS (326ms)
- ✅ No deadlocks or hangs observed

### Sanitizers
- ✅ TSAN: No data races detected
- ⚠️ ASAN: One false positive in test code (documented, not blocking)

---

## Files Changed

### Phoenix
- `src/transport/EnvelopeHelpers.*` (new)
- `src/transport/LocalSocketChannel.cpp` (envelope-based transport)
- `tests/envelope_helpers_test.cpp` (new)
- `CMakeLists.txt` (proto generation, version bump)
- `CHANGELOG.md` (Sprint 4.5 entry)

### Bedrock
- `src/palantir/EnvelopeHelpers.*` (new)
- `src/palantir/PalantirServer.cpp/hpp` (envelope-based transport, deadlock fix)
- `tests/palantir/EnvelopeHelpers_test.cpp` (new)
- `tests/integration/*` (new integration test framework)
- `CMakeLists.txt` (proto generation, integration tests)
- `CHANGELOG.md` (Sprint 4.5 entry)

### Documentation
- `docs/adr/ADR-0002-Envelope-Based-Palantir-Framing.md` (new)
- `docs/ipc/IPC_ENVELOPE_PROTOCOL.md` (new)
- `docs/sprint4.5/*` (multiple new/updated docs)

---

## Breaking Changes

⚠️ **Protocol Breaking Change**

This PR introduces a breaking change to the Palantir IPC protocol:
- **Before:** `[4-byte length][1-byte type][payload]`
- **After:** `[4-byte length][serialized MessageEnvelope]`

**Impact:**
- Phoenix and Bedrock must be updated in lockstep
- No backward compatibility with pre-4.5 versions
- Git-based rollback if needed (no runtime compatibility mode)

**Rationale:**
- Protocol still in active development
- No deployed external base
- Cleaner implementation without compatibility shims
- Versioning and extensibility built-in

---

## Documentation

Comprehensive documentation has been created:

- **Architecture:** `ADR-0002-Envelope-Based-Palantir-Framing.md`
- **Protocol Spec:** `docs/ipc/IPC_ENVELOPE_PROTOCOL.md`
- **Integration Tests:** `docs/sprint4.5/WS3_INTEGRATION_HARNESS.md`
- **Sprint Report:** `docs/sprint4.5/SPRINT_4.5_FINAL_REPORT.md`
- **CI Integration:** `docs/sprint4.5/CI_INTEGRATION_NOTES.md`

---

## Checklist

- [x] All tests passing
- [x] Documentation complete
- [x] CHANGELOG updated
- [x] Version bumped
- [x] No deadlocks or hangs
- [x] Integration tests validated
- [x] Code reviewed (self-review complete)

---

## Next Steps

After merge:
1. Add integration tests to CI pipeline (see `CI_INTEGRATION_NOTES.md`)
2. Monitor for any production issues
3. Plan Sprint 4.6 (job threading cleanup, additional RPCs)

---

## Related Issues

- Sprint 4.5 Fixup (control document: `docs/sprint4.5/FIXUP_SPRINT_CONTROL.md`)

---

## Review Notes

This is a large PR that touches core transport logic. Key areas to review:

1. **Deadlock Fix:** Verify lock scope is correct in `parseIncomingData()`
2. **Envelope Helpers:** Check error handling and validation logic
3. **Integration Tests:** Verify test framework correctly validates end-to-end flow
4. **Protocol Breaking Change:** Confirm breaking change is acceptable

All changes have been tested locally and integration tests pass.

