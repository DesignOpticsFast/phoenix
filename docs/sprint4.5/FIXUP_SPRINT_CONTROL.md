# Sprint 4.5 Fixup – Control Document (v4)

**Sprint Type**: Fixup / Hardening  
**Scope**: Phoenix ↔ Bedrock IPC, Palantir, Tooling, Docs, Versioning  
**Status**: ✅ Approved – Ready for Implementation  
**Duration (expected)**: ~3–4 weeks wall-clock (56–82 hours of implementation work)

---

## 1. Objectives

By the end of this sprint:

1. **Palantir protocol is envelope-based, unambiguous, and implemented correctly on both Phoenix and Bedrock**:
   - Wire format: `[4-byte length][serialized MessageEnvelope]`
   - No legacy framing, no backwards-compat modes.

2. **Concurrency, lifetime, and security in Palantir and related paths are robust and well-documented**:
   - No obvious races or UAFs.
   - Threading model is clear.

3. **Integration tests prove the system works end-to-end (happy & error paths), and CI runs them reliably**.

4. **Toolchain baseline is clarified and updated where required**:
   - Qt 6.10.1 rolled out on Crucible/CI.
   - Bedrock solidly on C++20 (C++23 upgrade deferred to its own sprint).

5. **Documentation across Phoenix, Bedrock, and Palantir/data-contracts tells a coherent, current story**.

6. **Versioning reflects the new foundation**:
   - Phoenix version is bumped to 0.0.4.
   - Bedrock/Palantir versions and protocol versions are clearly documented.

---

## 2. Phase 0 – Upfront Decisions & Audits

Phase 0 is about locking decisions and doing small audits before we touch code.

### 2.1 MessageEnvelope on the Wire – FINAL

- **Decision**: Palantir uses an envelope-based wire format:

```protobuf
message MessageEnvelope {
    uint32           version   = 1;  // protocol version
    MessageType      type      = 2;  // enum
    bytes            payload   = 3;  // inner message
    map<string,string> metadata = 4; // optional tracing/flags
}
```

- **Wire format**: `wire = [4-byte length][serialized MessageEnvelope]`
- **No direct `[length][type][payload]` mode**.
- **No backward-compatibility framing**.

**ADR**:
- Phoenix `docs/adr/ADR-00XX-Envelope-Based-Palantir-Framing.md`
- (Context → Decision → Consequences)

---

### 2.2 Migration Strategy – Big Bang

- Use a **feature branch** for envelope migration:
  - Implement Envelope in Phoenix + Bedrock on the same branch.
  - Add/update unit & integration tests.
  - Run full CI on the branch.
  - Merge into main when green.
- **No runtime "dual format" compatibility**.
- **Rollback is git-based, not protocol-based**.

---

### 2.3 MainWindow / Lifetime Audit

- Audit Phoenix and Bedrock for:
  - `static .*Engine|Client`
  - `QFutureWatcher.*connect`
  - `"new … without parent"`
- **Outcome**:
  - If lifetime issues exist: fix in Workstream 2.
  - If not found: record in `LIFETIME_AUDIT.md` and treat review item as closed.

---

### 2.4 Bedrock C++20 vs C++23

- **This sprint**: Bedrock remains C++20.
- Create `docs/C++23_UPGRADE_PLAN.md` including:
  - Compiler versions on Crucible / dev-01 / (Windows if relevant).
  - Desired C++23 features.
  - Risks and migration considerations.
  - Rough estimate for a future dedicated C++23 upgrade sprint (before Thermal Diffusion).

---

### 2.5 Demo Mode Audit

- Confirm whether any runtime "demo mode" exists in Phoenix:
  - **Current assumption**: no runtime demo; `XYSineDemo` is test-only and docs are stale.
- **Outcomes**:
  - If code exists: either:
    - Hard-disable it; or
    - Guard it with `PHOENIX_DEMO_MODE` & mark as deprecated.
  - In all cases: align docs with reality in Workstream 5.

---

### 2.6 Integration Test Architecture

- Integration tests live in **Bedrock** (`tests/integration/`).
- Use an **in-process PalantirServer** instance launched from test fixtures.
- Use a **minimal C++ client** (not full Phoenix UI) to drive:
  - Capabilities RPC.
  - XY Sine RPC.
  - Error cases.

---

### 2.7 CI Matrix & Coverage Targets

- **CI platform**: GitHub Actions (or equivalent existing system).
- **Matrix**:
  - **Linux** (ubuntu-latest):
    - Release build.
    - ASAN + TSAN configurations.
  - **macOS** (macos-latest):
    - Release build + integration tests.
  - **Windows**:
    - Best-effort builds (non-blocking gate for this sprint).
- **Coverage targets**:
  - **Palantir-related code** (see below): ≥ 80% line coverage.
  - **Overall project**: ≥ 70% (aim, not a blocker).
- **Palantir-related scope**:
  - **Bedrock**: `src/palantir/*`
  - **Phoenix**: `src/transport/*` + `src/analysis/RemoteExecutor*`
  - **Exclusions**: generated protobuf, test harness code.

---

## 3. Workstreams

### 3.1 Workstream 1 – Transport & IPC Correctness (Envelope-based)

**Objective**: Implement the envelope-based Palantir protocol cleanly, remove legacy, and normalize error behavior.

**Highlights**:
- Implement Envelope framing end-to-end on both sides.
- Remove all legacy direct-type framing and old `readMessage()` logic.
- Implement and test canonical error semantics and size limits.
- Clean up any remaining demo references in docs (or code, per audit).
- All changes are part of the envelope feature branch and cut over in a single merge.

**Gate**:
- Unit tests for envelope framing and error paths.
- Integration tests (Capabilities + XY Sine, + key error cases) passing on the feature branch.

---

### 3.2 Workstream 2 – Concurrency, Lifetime & Security

**Objective**: Ensure PalantirServer, Bedrock threading, and Phoenix lifetime behavior are safe and understood.

**Highlights**:
- **PalantirServer**:
  - Consistent mutex protection for shared state.
  - Clear locking model and documented thread model.
  - Input validation at RPC boundaries; early failure for bad inputs.
- **Bedrock threading**:
  - Consolidated `THREADING.md` describing OpenMP/TBB usage and safe patterns.
  - Clear identification of thread-safe vs non-thread-safe components.
- **Phoenix**:
  - Confirm QFutureWatcher and client objects are used and cleaned up correctly.
  - Verify no static singletons block future multi-window/multi-session design.

**Gate**:
- TSAN/ASAN/Valgrind runs clean on Palantir & key execution paths.
- Threading docs reflect actual implementation.

---

### 3.3 Workstream 3 – Testing & CI Foundation

**Objective**: Provide proof that the IPC and engine boundary really work, and keep it that way.

**Highlights**:
- **Integration tests** (Bedrock repo):
  - Capabilities RPC happy path.
  - XY Sine RPC happy path (with math validation).
  - **Error cases**:
    - Unknown type.
    - Oversized messages.
    - Malformed payload.
    - Server disconnect mid-RPC.
  - **Edge cases**:
    - Concurrency: multiple clients / jobs.
    - Rapid connect/disconnect.
- **Unit tests**:
  - Envelope framing encode/decode.
  - Error mapping and size limit enforcement.
- **CI integration**:
  - Jobs for Linux + macOS running integration suite.
  - ASAN/TSAN runs in CI for Palantir paths.

**Gate**:
- Integration tests stable (no flakiness).
- Palantir-related code coverage ≥ 80%.
- CI green across gating platforms.

---

### 3.4 Workstream 4 – Toolchain & Versions

**Objective**: Normalize the build toolchain and version story prior to larger feature work.

**Highlights**:
- Update Qt to 6.10.1 on Crucible and relevant CI runners; validate Phoenix builds and runs.
- Create/update `VERSIONS.md` as single source of truth for:
  - Qt versions.
  - C++ standards: Phoenix C++17, Bedrock C++20.
  - Minimum CMake, Protobuf, etc.
- Update docs to reference `VERSIONS.md` instead of hard-coding versions.
- Ensure Bedrock's C++20 configuration is consistent across CMake, docs, and code (C++23 deferred to a future sprint).

**Gate**:
- Phoenix & Bedrock build and run with the documented toolchain.
- No conflicting version references in docs.

---

### 3.5 Workstream 5 – Documentation Harmonization

**Objective**: Align the story across Phoenix, Bedrock, and Palantir/data-contracts.

This is documentation-heavy and can run in larger chunks once Workstreams 1–3 are stable.

#### Substream 5.1 – Phoenix docs

- **Introduce**:
  - `VERSIONS.md`.
  - `DEVELOPMENT_ENVIRONMENTS.md`.
  - `docs/INDEX.md`.
- **Clarify**:
  - Where development happens (Crucible vs dev-01).
  - Demo mode status (per Phase 0 audit).
- **Normalize**:
  - Paths (`$PHOENIX_ROOT` style).
  - Qt version references.

#### Substream 5.2 – Palantir / Data Contracts docs

- **Clearly separate**:
  - Current implementation (LocalSocket+protobuf Palantir).
  - Future phases (Arrow Flight / gRPC, stats sidecar, backpressure).
- **Add**:
  - Implementation-status table.
  - Transport spec doc for future Flight work.
  - Glossary and migration overview.

#### Substream 5.3 – Bedrock docs

- Resolve Python vs C++ coding-standards confusion.
- **Add or expand**:
  - `BEDROCK_ARCHITECTURE.md`.
  - `SOM_OVERVIEW.md`.
  - `BEDROCK_BUILD.md`.
  - `THREADING.md`.
  - `TESTING.md`.
  - `DEPLOYMENT.md`.
  - `OPENCASCADE_INTEGRATION.md`.
  - `REPOSITORY_STRUCTURE.md`.
  - (Optionally) `GIT_WORKFLOW.md` and `API_REFERENCE.md` as seeds.

#### Substream 5.4 – Cross-Repo Coherence

- **Cross-check**:
  - Version references.
  - Naming and architecture diagrams.
  - Protocol and transport descriptions.

**Gate**:
- No contradictions remain across Phoenix, Bedrock, Palantir docs.
- Architecture and build docs are complete and coherent.

---

### 3.6 Workstream 6 – Versioning & Release Bookkeeping

**Objective**: Make the fixup visible in versioning and recorded history.

**Highlights**:
- **Phoenix**:
  - Bump version to 0.0.4.
  - Update About/CLI output (if present).
  - Add CHANGELOG entry summarizing:
    - Envelope-based Palantir.
    - IPC and concurrency hardening.
    - Integration tests & CI improvements.
- **Bedrock/Palantir**:
  - Ensure Capabilities reports a sensible version string.
  - Document protocol versions (pre-envelope vs envelope-based).
  - Tag Bedrock repo if it fits your release scheme.

**Gate**:
- Version strings in running binaries match docs.
- Git tags and changelog entries exist.

---

## 4. Definition of Done (Sprint 4.5 Fixup)

The sprint is done when:

1. **Envelope-only Palantir protocol is implemented and stable**:
   - MessageEnvelope is the only on-wire framing.
   - No legacy framing paths remain.

2. **Concurrency, lifetime, and security are solid**:
   - TSAN/ASAN/Valgrind clean on Palantir and key flows.
   - Threading docs reflect actual behavior.

3. **Integration tests**:
   - Capabilities + XY Sine + key error cases pass reliably.
   - CI runs them on Linux + macOS.

4. **Toolchain & versions**:
   - Qt 6.10.1 rolled out.
   - `VERSIONS.md` is the single source of truth.
   - Bedrock C++20 consistently documented and used.

5. **Documentation**:
   - Phoenix, Bedrock, Palantir/data-contracts docs no longer contradict each other.
   - Architecture and build docs exist and are accurate.

6. **Versioning & release**:
   - Phoenix version is 0.0.4.
   - Bedrock/Palantir version story is clear.
   - Changelogs and tags reflect the new foundation.

At that point, the foundation is ready for:

- A focused Bedrock C++23 upgrade sprint.
- A Phoenix C++20 + AnalysisWindow improvement sprint.
- Implementation of Thermal Diffusion, Arrow Flight, and other advanced features on top of a stable, well-verified base.

---

**This control document is the "single source of truth" for Sprint 4.5 Fixup.**

When you're ready, we'll open a fresh UnderLord chat and start turning this into concrete Phase 0 + Workstream chunks.

