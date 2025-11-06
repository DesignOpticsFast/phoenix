# Phoenix Sprint 4 — System Architecture & Work Statement (Rev 5.1)

> ⚠️ **Superseded by CONTROL_REV_5.3.md and Phase 0.5 Gate Decision (2025-10-18): Qt 6.10 + Graphs / gRPC UDS baseline.**

**Owner / Validation Authority:** Mark Nicholson  
**Repositories:** DesignOpticsFast/phoenix, DesignOpticsFast/bedrock  
**Languages / Stack:**

- Phoenix: C++17 · Qt 6 (latest stable) · Protobuf · Palantir (LocalSocket)
- Bedrock: C++20 · OpenMP · TBB · Protobuf · Palantir server  
**Target Platforms:** macOS · Windows · Linux

---

## 🎯 Gate 0.5 Decisions (Final)

### Transport Protocol Choice

**Decision:** Use LocalSocket + Protobuf for Palantir communication  
**Rationale:** gRPC UDS overhead (7.5%) exceeds 5% threshold  
**ADR:** [ADR-20251017-transport-choice.md](ADR-20251017-transport-choice.md)  

### Qt Version (dev-01)

**Decision:** Proceed with Qt 6.9.3 + QtCharts for development  
**Rationale:** Qt 6.10.x requires Qt Account credentials; 6.9.3 performs excellently  
**Note:** Qt 6.10.x upgrade planned once headless installer resolved  

### Performance Validation

- ✅ Qt Charts: 50 idle windows CPU < 1% (target: < 5%)
- ✅ Transport: LocalSocket baseline 100ms vs gRPC 107.5ms (7.5% overhead)
- ✅ All acceptance criteria met for Gate 0.5

---

## 🧭 Intent

Build a responsive, crash-resistant, multilingual Phoenix IDE shell and demonstrate the first end-to-end analysis loop via Palantir between Phoenix (C++17/Qt) and Bedrock (C++20).

**Objectives:**

1. Zero-freeze UI (< 50 ms chrome response)
2. Independent, concurrent Analysis windows (~ 50)
3. Async progress / cancel / debug channels
4. Stable cross-platform behavior
5. Architecture ready for Python and remote compute later

---

## 🧩 Structure & Milestones

| Phase | Goal | Milestone |
|-------|------|-----------|
| 0.5 (Gate) | ✅ Validate Qt Charts + Palantir transport | ✅ Prereq to Phase 1 |
| 1 | Phoenix MainWindow + chrome | Milestone A start |
| 2 | Palantir v0 + XY Sine Analysis | Milestone A Gate |
| 3 | 2D Data Scaffold (stub) | – |
| 4 | Bedrock multi-thread compute + progress/cancel | – |
| 5 | Apply MT pattern to 2D jobs | Milestone B Gate |

---

## 🪟 Phase 1 — MainWindow & Chrome

**Scope:** Standard menus, dockable ribbons, status bar slots, Preferences → Environment, multilingual UI theme.

**Implementation:**

- Menu bar: File / Editors / Analysis / Tools / Help (actions stubbed)
- Ribbons: top (horizontal) & right (vertical), dockable to any side, floatable, resizable
- Use QToolBars with FontAwesome icons (placeholder icons OK)
- Status bar: left half = transient status messages, right half subdivided into 5 "slots"
- Preferences → Environment (two-pane dialog)
- Localization plumbing with QLocale usage
- Telemetry hooks for UI latency logging

**Acceptance:**

- Chrome actions respond < 50 ms
- Dock/float ribbons without flicker
- Preferences opens, stores settings, and updates locale/decimal selection

---

## 📈 Phase 2 — Palantir v0 & XY Sine Analysis

**Transport:** LocalSocket + Protobuf (4-byte length prefix + protobuf payload)  
**Service Pattern:** Unary Start/Cancel/Result + server-streamed progress (≤ 5 Hz → UI 2 Hz)  
**Compute Settings:** amplitude, frequency, phase, cycles, n_samples (default 500, 100–10,000)  
**Update All:** enqueue per window jobs, respect max_active_jobs  

**XY Sine Analysis Window:**

- Toolbar: Calculator (Compute Settings), Display (style), Update
- Tabs: Graph (Qt Charts via QChartView), Data (table with copy), Debug (if requested)
- Downsampling: None when n_samples ≤ 2,000; Enable LTTB above that
- Update All: enqueue per window; respect max_active_jobs

**Acceptance:**

- ≥ 50 Analysis windows can open; independent Start/Cancel/Update with no UI freeze
- First paint after data arrival ≤ 200 ms on realistic datasets
- Cancel to CANCELLING transition < 200 ms

---

## 🧵 Threading Architecture

**Phoenix (C++17):** QThreadPool = min(2×HW_CONCURRENCY, 32); per-window queues (round-robin); overflow → Queued  
**Bedrock (C++20):** reports Capabilities(max_concurrency); uses TBB + OpenMP thread pools; leverages C++20 std::jthread, barriers, and latch constructs  
**Phoenix caps active jobs to that limit**

---

## 🔒 Failure Modes & Recovery

| Failure | Detection | Phoenix Action | User Visible | Recovery |
|---------|-----------|----------------|--------------|----------|
| Bedrock crash | Conn lost | Auto-restart + re-Ping | Toast "Engine restarting…" | Jobs → FAILED:ENGINE_LOST |
| IPC timeout | No Pong 5 s | Reconnect ×3 (backoff) | Status "Reconnecting…" | Retry or manual restart |
| Job timeout | No progress > 5 min | Cancel(JobId) | CANCELLED:TIMEOUT | Adjustable timeout |
| Cancel fails | Cancel > 5 s | Force terminate | Dialog "Engine unresponsive…" | Restart engine |
| Result too large | > 5 GB | Cancel stream | Error toast | Graceful fail |

---

## ⚙️ Engine Supervision Policy

Bedrock child process + watchdog; stdout/stderr rolling logs (3 × 5 MB)  
Restart backoff 0→2→5→10 s (max). Heartbeat Ping every 2 s (> 5 s miss = timeout)  
Version check on connect (protocol_version)

---

## 💬 Palantir Status & Error Codes

State machine: QUEUED → RUNNING → (SUCCEEDED \| FAILED \| CANCELLING → CANCELLED)  
Canonical codes: OK, INVALID_ARGUMENT, UNIMPLEMENTED, FAILED_PRECONDITION, RESOURCE_EXHAUSTED, DEADLINE_EXCEEDED, CANCELLED, ABORTED, INTERNAL, UNAVAILABLE, DATA_TOO_LARGE, DEBUG_DISABLED

---

## 🗃 Data Model — AnalysisSession

```cpp
struct AnalysisSession {
    QString     feature_id;
    QVariantMap compute_params;
    ResultMeta  meta;
    QByteArray  payload;     // offload ≥ 1 GB
    DebugLog    debug;
    QLocale     locale;
    QDateTime   created_at;
};
```

RAII (shared_ptr + weak_ptr cache); offload ≥ 1 GB; hard fail > 5 GB

---

## 💾 Result Size Policy

Soft 512 MB → offload 1 GB → fail 5 GB; Temp dirs: platform defaults; LRU 10 GB cache  
Status: "Offloaded to disk (X.Y GB)"

---

## 📊 Telemetry & Testing

UI < 50 ms; open ≤ 300/150 ms; progress 2 Hz; cancel < 200 ms; 50 idle windows CPU < 5 %  
CSV header subset: ...,kernel_id,grid_n,iterations,initial_threads,effective_bandwidth_gb_s

---

## 🧮 Reference Datasets

XY Sine N = 100–1000 (realistic) / 10,000 (stress)  
2D Grid 256×256 · 1024×1024 · 4096×4096 (float32)

---

## 🧠 Version Policy (Qt)

Use current stable Qt at sprint start; upgrade on new stable if CI green (all OS)

---

## 📈 System-Level Threading & Soak Policy

Log every 5 s (cores, threads, CPU %, jobs). Pool = min(2×cores, 32); max_active = min(bedrock.max_concurrency, cores)  
Bedrock per-job threads = max(1, floor(cores/active_jobs))  
Stop-the-line > 30 s if threads > 3×cores or sys CPU > 10 % or UI lat +20 % → Throttle max jobs −1 and reduce UI update rate 1 Hz. ADR if persistent or threads > 4×cores

---

## 🔥 Phase 4 — Bedrock MT Compute (Heat Diffusion Kernel)

Reference Kernel: 2D Heat Diffusion (Jacobi Iteration) · C++20 (OpenMP collapse(2))  
Grid 2000×2000 · 1000 iterations (~30 s). Progress = iter/max_iters. Cancellation flag checked each iteration  
Output = 2D temperature field + compute_elapsed_ms, bytes_total, labels  

Acceptance: smooth progress · responsive cancel · no leaks · concurrency respected

---

## 🌈 Phase 5 — Apply MT Pattern to 2D

Reuse Heat Diffusion kernel → route to 2D renderer  
Validate grid metadata, cancel/progress parity, UI responsiveness

---

## 🧱 Annex E — Reference Kernel Note (Jacobi 5-Point Stencil)

Core Loop (Pseudocode)

```cpp
// Bedrock C++20 reference
const int N = 2000;
const int iterations = 1000;
std::vector<std::vector<double>> T(N,std::vector<double>(N,0.0));
std::vector<std::vector<double>> Tnew = T;

// Initial condition (center hot spot)
for (int i=0;i<N;++i)
  for (int j=0;j<N;++j)
    if ((i-N/2)*(i-N/2)+(j-N/2)*(j-N/2)<(N/10)*(N/10))
      T[i][j]=100.0;

for (int iter=0;iter<iterations;++iter){
  if (cancel_requested()) break;
  #pragma omp parallel for collapse(2)
  for (int i=1;i<N-1;++i)
    for (int j=1;j<N-1;++j)
      Tnew[i][j]=0.25*(T[i+1][j]+T[i-1][j]+T[i][j+1]+T[i][j-1]);
  std::swap(T,Tnew);
  report_progress(100.0*(iter+1)/iterations);
}
```

---

## 🧾 ADR Triggers (Milestone A)

Record if transport ≠ LocalSocket+Protobuf · threading model changed · Qt Charts replaced

---

## 🏁 Definition of Done

All acceptances met; Milestones A and B signed off by Mark; CI green (macOS/Win/Linux); no new CodeQL highs; perf & soak data attached; issues logged

---

## 🚨 Known Deviations

### Qt Charts Usage (Phase 1-2)

- **Deviation:** Used Qt Charts (deprecated) for plotting functionality
- **Cause:** Failed to identify deprecation status before implementation
- **Policy Violation:** "No deprecated libraries" policy violated
- **Mitigation:** Migration to Qt Graphs planned for Phase 3-5
- **Reference:** [Project Notes](project_notes.md#post-incident-note--qt-charts-deprecated-usage)

---

## ✅ Status: Final — Approved for UnderLord Execution (Rev 5.1)

This Rev 5.1 supersedes all prior revisions and includes:

- Gate 0.5 decisions (Transport: LocalSocket+Protobuf, Qt: 6.9.3)
- Phoenix → C++17 (Qt-aligned)
- Bedrock → C++20 (OpenMP/TBB/multi-config)
- Known deviations documented with corrective actions

UnderLord may now commence Phase 1 & 2 execution.
