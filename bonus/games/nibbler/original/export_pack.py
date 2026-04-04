#!/usr/bin/env python3
"""Export the reviewed reference data as a strict, externally importable Nibbler pack."""
import argparse
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent
COARSE = [0] + [n for n in range(1, 18) for _ in range(1 if n % 2 else 2)] + [18]


def render(reference, hard=False):
    lines = [
        "NIBBLER_PACK 1",
        'PROVENANCE "FrankRizzo890/NibblerBot video transcription; '
        'see original/data/manifest.json"',
    ]
    for number, wave in enumerate(reference["waves"], 1):
        speed = wave["speed_hard" if hard else "speed_easy"]
        lines.append(f'LEVEL {number} {wave["maze"]} {wave["pattern"]} {speed} 27 27')
        geometry = reference["geometries19"][str(wave["maze"])]
        rows = [[geometry[y][x] for x in COARSE] for y in COARSE]
        for x, y in reference["fruit_patterns"][str(wave["pattern"])]:
            if rows[y][x] != " ":
                raise ValueError(f"wave {number}: fruit overlaps a wall")
            rows[y][x] = "."
        for x in (10, 11, 12):
            rows[25][x] = "o"
        rows[25][13] = "H"
        lines.extend("".join(row) for row in rows)
        lines.append("ENDLEVEL")
    lines.append("ENDPACK")
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output", type=Path, default=ROOT / "data" / "reference-pack.txt",
    )
    parser.add_argument(
        "--hard", action="store_true", help="use the documented Hard DIP speed codes",
    )
    parser.add_argument(
        "--check", action="store_true", help="verify an existing export without writing",
    )
    args = parser.parse_args()
    try:
        reference = json.loads((ROOT / "data" / "reference.json").read_text())
        text = render(reference, args.hard)
        if args.check:
            if args.output.read_text() != text:
                raise ValueError(f"reference export differs: {args.output}")
        else:
            args.output.write_text(text)
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"export_pack: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
