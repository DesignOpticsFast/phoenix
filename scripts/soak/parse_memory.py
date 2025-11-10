#!/usr/bin/env python3
"""Parse Phoenix soak CSV output and compute memory drift statistics.

Usage:
    python scripts/soak/parse_memory.py /tmp/phoenix_soak_memory.csv [--app-log /tmp/phoenix_soak_app.log]
"""

from __future__ import annotations

import argparse
import csv
import math
import statistics
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, List, Optional

TARGET_INTERVAL = 5.0
TREND_WINDOW_SEC = 30 * 60  # 30 minutes
MAX_DRIFT_MB = 5.0
MAX_DRIFT_PCT = 3.0
MIN_SAMPLE_RATIO = 0.9


@dataclass
class Sample:
    timestamp: str
    elapsed_sec: float
    rss_mb: float
    rss_kb: float
    threads: Optional[int]
    fds: Optional[int]
    handles: Optional[int]


def parse_csv(path: Path) -> List[Sample]:
    samples: List[Sample] = []
    with path.open(newline="", encoding="utf-8") as fh:
        reader = csv.DictReader(fh)
        required = {"timestamp", "elapsed_sec", "rss_kb", "rss_mb"}
        if not required.issubset(reader.fieldnames or []):
            raise ValueError(f"CSV missing required headers: {sorted(required)}")
        for row in reader:
            try:
                elapsed = float(row["elapsed_sec"])
            except (TypeError, ValueError):
                continue
            try:
                rss_kb = float(row["rss_kb"]) if row["rss_kb"] else math.nan
            except ValueError:
                rss_kb = math.nan
            try:
                rss_mb = float(row["rss_mb"]) if row["rss_mb"] else (rss_kb / 1024.0 if not math.isnan(rss_kb) else math.nan)
            except ValueError:
                rss_mb = math.nan
            def _parse_int(value: Optional[str]) -> Optional[int]:
                if value in (None, ""):
                    return None
                try:
                    return int(value)
                except ValueError:
                    return None
            samples.append(
                Sample(
                    timestamp=row.get("timestamp", ""),
                    elapsed_sec=elapsed,
                    rss_mb=rss_mb,
                    rss_kb=rss_kb,
                    threads=_parse_int(row.get("threads")),
                    fds=_parse_int(row.get("fds")),
                    handles=_parse_int(row.get("handles")),
                )
            )
    if not samples:
        raise ValueError("CSV contains no valid samples")
    samples.sort(key=lambda s: s.elapsed_sec)
    return samples


def quantile(data: List[float], q: float) -> float:
    if not data:
        return float("nan")
    if len(data) == 1:
        return data[0]
    return statistics.quantiles(data, n=100, method="inclusive")[int(q * 100) - 1]


def compute_stats(samples: List[Sample]) -> dict:
    rss_values = [s.rss_mb for s in samples if not math.isnan(s.rss_mb)]
    if not rss_values:
        raise ValueError("No RSS data available in samples")

    min_rss = min(rss_values)
    max_rss = max(rss_values)
    avg_rss = statistics.mean(rss_values)
    drift_mb = max_rss - min_rss
    base = min_rss if min_rss > 0 else max(avg_rss, 1e-6)
    drift_pct = (drift_mb / base) * 100.0

    elapsed_values = [s.elapsed_sec for s in samples]
    actual_elapsed = elapsed_values[-1] - elapsed_values[0]
    delta_secs = [elapsed_values[i] - elapsed_values[i - 1] for i in range(1, len(elapsed_values))]

    jitter_p50 = quantile(delta_secs, 0.50) if delta_secs else float("nan")
    jitter_p95 = quantile(delta_secs, 0.95) if delta_secs else float("nan")

    expected_samples = math.floor(actual_elapsed / TARGET_INTERVAL) if actual_elapsed > 0 else 0
    required_samples = math.floor(expected_samples * MIN_SAMPLE_RATIO)

    nonneg_prefix = [0]
    for i in range(1, len(samples)):
        delta = samples[i].rss_mb - samples[i - 1].rss_mb
        nonneg_prefix.append(nonneg_prefix[-1] + (1 if delta >= 0 else 0))

    review = False
    if len(samples) >= 2 and actual_elapsed >= TREND_WINDOW_SEC:
        for start in range(len(samples) - 1):
            for end in range(start + 1, len(samples)):
                duration = samples[end].elapsed_sec - samples[start].elapsed_sec
                if duration < TREND_WINDOW_SEC:
                    continue
                total_deltas = end - start
                if total_deltas <= 0:
                    continue
                nonneg = nonneg_prefix[end] - nonneg_prefix[start]
                if nonneg / total_deltas >= 0.80:
                    review = True
                    break
            if review:
                break

    return {
        "min_rss": min_rss,
        "max_rss": max_rss,
        "avg_rss": avg_rss,
        "drift_mb": drift_mb,
        "drift_pct": drift_pct,
        "actual_elapsed": actual_elapsed,
        "expected_samples": expected_samples,
        "sample_count": len(rss_values),
        "required_samples": required_samples,
        "jitter_p50": jitter_p50,
        "jitter_p95": jitter_p95,
        "review": review,
    }


def check_log(app_log: Optional[Path]) -> List[str]:
    if not app_log:
        return []
    if not app_log.exists():
        return [f"app log '{app_log}' not found"]
    issues: List[str] = []
    keywords = ("warning", "timeout", "crash", "segfault", "exception")
    with app_log.open(encoding="utf-8", errors="ignore") as fh:
        for line in fh:
            lower = line.lower()
            if any(word in lower for word in keywords):
                issues.append(line.strip())
                if len(issues) >= 5:
                    break
    return issues


def _fmt(value: float, suffix: str = "") -> str:
    if isinstance(value, float) and math.isnan(value):
        return f"NaN{suffix}"
    return f"{value:.2f}{suffix}"


def format_stats(stats: dict) -> str:
    lines = [
        "Soak Summary:",
        f"  Samples           : {stats['sample_count']} (expected ≥ {stats['required_samples']})",
        f"  Duration (s)      : {stats['actual_elapsed']:.1f}",
        f"  RSS min / avg / max (MB): {_fmt(stats['min_rss'])} / {_fmt(stats['avg_rss'])} / {_fmt(stats['max_rss'])}",
        f"  Drift (MB / %)    : {_fmt(stats['drift_mb'])} MB / {_fmt(stats['drift_pct'])}%",
        f"  Jitter p50 / p95  : {_fmt(stats['jitter_p50'],'s')} / {_fmt(stats['jitter_p95'],'s')}",
        f"  Trend review flag : {'YES' if stats['review'] else 'no'}",
    ]
    return "\n".join(lines)


def main(argv: Iterable[str]) -> int:
    parser = argparse.ArgumentParser(description="Phoenix soak memory parser")
    parser.add_argument("csv", type=Path, help="path to memory CSV")
    parser.add_argument("--app-log", type=Path, help="path to application log", dest="app_log")
    args = parser.parse_args(list(argv))

    samples = parse_csv(args.csv)
    stats = compute_stats(samples)
    log_issues = check_log(args.app_log)

    sample_ok = stats["sample_count"] >= stats["required_samples"]
    drift_ok = stats["drift_mb"] <= MAX_DRIFT_MB or stats["drift_pct"] <= MAX_DRIFT_PCT
    log_ok = not log_issues

    verdict = "PASS"
    exit_code = 0
    if not (sample_ok and drift_ok and log_ok) or stats["review"]:
        verdict = "REVIEW" if stats["review"] else "FAIL"
        exit_code = 1

    print(format_stats(stats))
    if log_issues:
        print("  Log issues detected:")
        for issue in log_issues:
            print(f"    - {issue}")
    print(f"Verdict: {verdict}")
    return exit_code


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv[1:]))
    except Exception as exc:  # pragma: no cover
        print(f"ERROR: {exc}", file=sys.stderr)
        sys.exit(1)
