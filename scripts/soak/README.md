# Phoenix Soak Harness

These scripts provide a cross-platform soak harness for Phoenix builds. Each harness wraps the Phoenix binary, samples memory usage every ~5 seconds, and records the results into a CSV file that can be parsed by `parse_memory.py`.

## Outputs

* Application log: `/tmp/phoenix_soak_app.log` on Unix, `%TEMP%\phoenix_soak_app.log` on Windows (override via `SOAK_APP_LOG`).
* Memory samples: `/tmp/phoenix_soak_memory.csv` (or `%TEMP%\phoenix_soak_memory.csv`) with header:

```
timestamp,elapsed_sec,rss_kb,rss_mb,threads,fds,handles
```

Empty fields are left blank on platforms that cannot provide the value (e.g., file descriptors on macOS).

## Environment Variables

| Variable        | Description                                                        | Default                                  |
|-----------------|--------------------------------------------------------------------|------------------------------------------|
| `SOAK_BIN`      | Path to the Phoenix binary to execute.                             | `build/dev-01-relwithdebinfo/phoenix_app`|
| `SOAK_ARGS`     | Additional arguments passed to Phoenix.                            | `--test-i18n --lang=en`                  |
| `DURATION_MIN`  | Duration of the soak in minutes.                                   | `60`                                     |
| `SOAK_APP_LOG`  | Path to the application log file.                                  | `/tmp/phoenix_soak_app.log`              |
| `SOAK_CSV`      | Path to the memory CSV file.                                       | `/tmp/phoenix_soak_memory.csv`           |

All harnesses set `QT_QPA_PLATFORM=offscreen` unless already defined.

## Usage

```bash
# Linux
scripts/soak/linux_soak.sh

# macOS
scripts/soak/macos_soak.zsh

# Windows (PowerShell)
./scripts/soak/windows_soak.ps1
```

## Parser

Run `python3 scripts/soak/parse_memory.py /tmp/phoenix_soak_memory.csv` to compute summary statistics, drift limits, jitter percentiles, and trend analysis. The parser exits with:

* `0` when drift ≤ 5 MB or ≤ 3% and sample coverage ≥ 90% of `floor(actual_elapsed / 5)`.
* `1` otherwise.

The parser also reports a single-line verdict (`PASS` or `REVIEW/FAIL`).

## Make Targets

* `make soak-build` – configure & build Phoenix (no soak executed).
* `make soak-quick` – 30-minute run via `linux_soak.sh`.
* `make soak-1h` – 60-minute run.
* `make soak-2h` – 120-minute run.

Each run target invokes the parser after sampling and propagates its exit code.

## Acceptance Criteria

* Recorded samples ≥ 0.9 × `floor(actual_elapsed / 5)`.
* Memory drift ≤ 5 MB **or** drift percentage ≤ 3%.
* Trend check: if ≥ 80% of consecutive RSS deltas are non-negative within any ≥ 30-minute window, the harness flags `REVIEW`.
* Application log must not contain warnings, timeouts, or crashes.
* Parser exit code `0` (PASS) for sign-off.

## CI Policy

* Pull Requests – no soak to keep CI fast.
* Nightly (Linux) – `make soak-quick` (30 min); upload app log & CSV; job fails on parser failure.
* Release tag / manual workflow – `make soak-1h` (Linux); optional macOS soak weekly; same artifact policy.

## Example

```bash
make soak-build
make soak-quick
python3 scripts/soak/parse_memory.py /tmp/phoenix_soak_memory.csv
```

The parser output includes min/max/avg RSS, drift, jitter percentiles, sample coverage, and verdict.
