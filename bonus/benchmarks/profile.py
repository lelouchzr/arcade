#!/usr/bin/env python3
"""Collect native CPU samples separately from benchmark timings."""
import argparse
from pathlib import Path
import re
import subprocess
import sys
import time

DEFAULT_CASES = ["menu_frame", "pacman_frame", "pacman_render", "model_pacman"]


def collect(args, case):
    """Attach sample to a long-lived workload and reject empty reports."""
    report = (args.output / f"{case}.sample.txt").resolve()
    report.unlink(missing_ok=True)  # A previous successful capture must not mask an empty new one.
    command = [
        str(args.binary.resolve()), "--module-dir", str(args.modules.resolve()),
        "--graphics", str(args.graphics.resolve()), "--case", case,
        "--iterations", "2000", "--profile-seconds", "7",
    ]
    with (args.output / f"{case}.jsonl").open("w") as output:
        child = subprocess.Popen(command, stdout=output)
        try:
            time.sleep(0.25)
            subprocess.run(
                ["/usr/bin/sample", str(child.pid), "3", "1", "-mayDie", "-file", str(report)],
                check=True, stdout=subprocess.DEVNULL,
            )
            if child.wait(timeout=15) != 0:
                raise RuntimeError(f"profile workload failed: {case}")
        finally:
            if child.poll() is None:
                child.terminate()
                try:
                    child.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    child.kill()
                    child.wait()
    contents = report.read_text()
    graph = contents.partition("Call graph:")[2].partition("Total number in stack")[0]
    if not re.search(r"\b[1-9]\d* Thread_", graph) or "arcade_benchmark" not in graph:
        raise RuntimeError(f"sample returned no workload call graph: {report}")
    print(f"Profile recorded: {case}", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--modules", type=Path, required=True)
    parser.add_argument("--graphics", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--cases", nargs="+", default=DEFAULT_CASES)
    args = parser.parse_args()
    if any(not re.fullmatch(r"[a-z_]+", case) for case in args.cases):
        parser.error("case names must contain lowercase letters and underscores")
    try:
        args.output.mkdir(parents=True, exist_ok=True)
        for case in args.cases:
            collect(args, case)
    except (OSError, RuntimeError, subprocess.SubprocessError) as error:
        print(f"profile: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
