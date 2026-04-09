#!/usr/bin/env python3
"""Run interleaved A/B trials with setup and warm-up outside recorded intervals."""
import argparse
import json
from pathlib import Path
import statistics
import subprocess
import sys

CASES = [
    "menu_frame", "pacman_frame", "pacman_render", "model_snake", "model_minesweeper",
    "model_nibbler", "model_pacman", "model_qix", "model_centipede", "model_solarfox",
]


def measure(root, case, iterations):
    command = [
        str(root / "benchmarks" / "arcade_benchmark"), "--module-dir", str(root / "lib"),
        "--graphics", str(root / "benchmarks" / "arcade_bench_headless.so"),
        "--case", case, "--iterations", str(iterations), "--warmups", "1", "--single",
    ]
    child = subprocess.run(command, check=True, text=True, capture_output=True)
    record = json.loads(child.stdout)
    required = {"wall_us", "cpu_us", "iterations", "checksum", "commands", "resets", "active_ticks"}
    if not isinstance(record, dict) or not required.issubset(record):
        raise ValueError(f"invalid benchmark record for {case}")
    return record


def compare(args):
    artifacts = {"baseline": args.baseline.resolve(), "candidate": args.candidate.resolve()}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w") as output:
        for case in args.cases:
            iterations = args.model_iterations if case.startswith("model_") else 10000
            rows = []
            for repeat in range(args.repetitions):
                order = ("baseline", "candidate") if repeat % 2 == 0 else ("candidate", "baseline")
                pair = {}
                for label in order:
                    record = measure(artifacts[label], case, iterations)
                    record.update(variant=label, pair=repeat, order=list(order))
                    pair[label] = record
                    rows.append(record)
                    output.write(json.dumps(record) + "\n")
                    output.flush()
                for key in ("checksum", "commands", "resets", "active_ticks"):
                    if pair["baseline"][key] != pair["candidate"][key]:
                        raise RuntimeError(f"{case}: baseline/candidate mismatch in {key}")
            values = {
                label: statistics.median(
                    row["wall_us"] / row["iterations"] for row in rows if row["variant"] == label
                )
                for label in artifacts
            }
            print(
                f'{case}: {values["baseline"]:.6f} -> {values["candidate"]:.6f} us/unit',
                flush=True,
            )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--repetitions", type=int, default=7)
    parser.add_argument("--cases", nargs="+", choices=CASES, default=CASES)
    parser.add_argument("--model-iterations", type=int, default=100000)
    args = parser.parse_args()
    if args.repetitions < 5:
        parser.error("at least five interleaved repetitions are required")
    if not 0 < args.model_iterations <= 100000000:
        parser.error("model iterations must be between 1 and 100000000")
    try:
        compare(args)
    except subprocess.CalledProcessError as error:
        print(f"compare: workload failed ({error.returncode}): {error.stderr}", file=sys.stderr)
        return 1
    except (OSError, ValueError, RuntimeError, KeyError, subprocess.SubprocessError) as error:
        print(f"compare: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
