#!/usr/bin/env python3
"""Reproduce the reviewed Nibbler transcription from its primary video and Lua notes.

Requires Pillow and ffmpeg only for this optional research tool; the game needs neither.
The Lua files are parsed as data and never executed. No ROM is read or downloaded.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

try:
    from PIL import Image
except ImportError:
    print("verify_reference: install Pillow to run this optional research tool", file=sys.stderr)
    sys.exit(1)

ROOT = Path(__file__).resolve().parent
COARSE = [0] + [n for n in range(1, 18) for _ in range(1 if n % 2 else 2)] + [18]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def frame(directory, index):
    with Image.open(directory / f"frame-{index:04d}.png") as source:
        image = source.convert("RGB")
    if image.size != (210, 240):
        raise ValueError("reference video must decode to the documented 210x240 format")
    return image


def components(image, threshold):
    """Find bright connected components within the playfield, excluding the HUD."""
    seen = set()
    result = []
    for y in range(31, 232):
        for x in range(1, 202):
            if (x, y) in seen or max(image.getpixel((x, y))) < threshold:
                continue
            points = [(x, y)]
            seen.add((x, y))
            for px, py in points:
                for nx, ny in ((px - 1, py), (px + 1, py), (px, py - 1), (px, py + 1)):
                    if ((nx, ny) not in seen and 0 < nx < 202 and 30 < ny < 232
                            and max(image.getpixel((nx, ny))) >= threshold):
                        seen.add((nx, ny))
                        points.append((nx, ny))
            result.append(points)
    return result


def extent(points):
    return (max(x for x, _ in points) - min(x for x, _ in points),
            max(y for _, y in points) - min(y for _, y in points))


def geometry(image):
    keep = set()
    for points in components(image, 90):
        if max(extent(points)) >= 10:
            keep.update(points)
    rows = []
    for y in range(19):
        row = ""
        for x in range(19):
            cx, cy = round((x * 12 - 1) * .9375), round((y * 12 + 31) * .9375)
            bright = sum((px, py) in keep for py in range(cy - 2, cy + 3)
                         for px in range(cx - 2, cx + 3))
            # Every maze has the same isolated post lattice. The last lane is the spawn lane.
            wall = (x in (0, 18) or y in (0, 18) or (x % 2 == 0 and y % 2 == 0)
                    or (y != 17 and (x % 2 == 0 or y % 2 == 0) and bright >= 3))
            row += "#" if wall else " "
        rows.append(row)
    return rows


def food(image, layout):
    counts = Counter()
    for points in components(image, 110):
        if len(points) > 100 or max(extent(points)) >= 10:
            continue
        for px, py in points:
            x = int((px + .5) / .9375 / 8)
            y = int(((py + .5) / .9375 - 32) / 8)
            if (0 < x < 26 and 0 < y < 26 and layout[y][x] != "#"
                    and not (y == 25 and 8 <= x <= 18)):
                counts[x, y] += 1
    return {point for point, count in counts.items() if count >= 8}


def verify_frames(directory, reference, manifest):
    layouts = {}
    for maze, index in manifest["geometry_frames"].items():
        actual = geometry(frame(directory, index))
        if actual != reference["geometries19"][maze]:
            raise ValueError(f"maze {maze}: video geometry differs")
        layouts[maze] = ["".join(actual[y][x] for x in COARSE) for y in COARSE]
    evidence = {i: Counter() for i in range(1, 12)}
    for wave in manifest["waves"]:
        index = wave["frame_index"]
        image = frame(directory, index)
        if hashlib.sha256(image.tobytes()).hexdigest() != wave["frame_rgb_sha256"]:
            raise ValueError(f'wave {wave["wave"]}: decoded RGB hash differs')
        for offset in range(5):
            points = food(frame(directory, index + offset), layouts[str(wave["maze"])])
            evidence[wave["fruit_pattern"]].update(points)
    for pattern, counter in evidence.items():
        actual = sorted(point for point, count in counter.items() if count >= 2)
        expected = [tuple(point) for point in reference["fruit_patterns"][str(pattern)]]
        if actual != expected:
            raise ValueError(f"fruit pattern {pattern}: video positions differ")


def lua_routes(source):
    text = re.sub(r"--\[\[.*?\]\]", "", source, flags=re.S)
    text = re.sub(r"--[^\n]*", "", text)
    for match in re.finditer(r"Maze_(\d+)_Pattern_(\d+)\s*=\s*\{", text):
        depth = 0
        start = None
        tables = []
        for index in range(match.end() - 1, len(text)):
            if text[index] == "{":
                depth += 1
                if depth == 2:
                    start = index
            elif text[index] == "}":
                if depth == 2:
                    tables.append(text[start:index + 1])
                depth -= 1
                if depth == 0:
                    break
        for table in tables:
            entries = re.findall(
                r"address\s*=\s*(0x[\da-fA-F]+),\s*character\s*=\s*0x[\da-fA-F]+,"
                r"\s*action\s*=\s*(\w+)", table,
            )
            if entries:
                yield int(match.group(1)), entries


def verify_sources(directory, reference, manifest):
    for name, source in manifest["sources"].items():
        if digest(directory / name) != source["sha256"]:
            raise ValueError(f"primary source hash differs: {name}")
    maze_ids = {0xB400: 1, 0xB800: 2, 0xB000: 3, 0xBC00: 4,
                0x9000: 5, 0x9400: 6, 0x9C00: 7, 0x9800: 8}
    fruit_ids = {0xB780: 1, 0xBF80: 2, 0xB7C0: 3, 0xBB80: 4, 0x9380: 5,
                 0xBBC0: 6, 0x9780: 7, 0x9B80: 8, 0x93C0: 9, 0xBFC0: 10, 0xB380: 11}
    notes = (directory / "NibblerNotes.txt").read_text()
    entries = re.findall(
        r"^\s*(\d+)\s+\.WORD\s+\$([\dA-Fa-f]+)\s+\.WORD\s+\$([\dA-Fa-f]+)"
        r"\s+(\d+)\s+(\d+)", notes, flags=re.M,
    )
    if len(entries) != 32:
        raise ValueError("primary notes must define exactly 32 wave compositions")
    for number, fruit_address, maze_address, easy, hard in entries:
        number = int(number) or 32
        expected = reference["waves"][number - 1]
        actual = {"maze": maze_ids[int(maze_address, 16)],
                  "pattern": fruit_ids[int(fruit_address, 16)],
                  "speed_easy": int(easy), "speed_hard": int(hard)}
        if actual != expected:
            raise ValueError(f"wave {number}: composition or speed codes differ from notes")
    paths = 0
    for maze in range(1, 9):
        coarse = reference["geometries19"][str(maze)]
        layout = ["".join(coarse[y][x] for x in COARSE) for y in COARSE]
        for _, entries in lua_routes((directory / f"Maze{maze}.lua").read_text()):
            x, y = 13, 25
            for address, action in entries:
                if action == "END":
                    break
                offset = int(address, 16) - 0x400
                nx, ny = 27 - offset // 32, offset % 32 - 4
                if x != nx and y != ny:
                    raise ValueError(f"maze {maze}: non-axial primary route")
                for px in range(min(x, nx), max(x, nx) + 1):
                    for py in range(min(y, ny), max(y, ny) + 1):
                        if not (0 <= px < 27 and 0 <= py < 27) or layout[py][px] == "#":
                            raise ValueError(f"maze {maze}: primary route crosses a wall")
                x, y = nx, ny
            paths += 1
    return paths


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--video", type=Path, required=True)
    parser.add_argument(
        "--nibblerbot", type=Path, required=True, help="primary Lua files directory",
    )
    parser.add_argument(
        "--frames-dir", type=Path, help="reuse frames decoded with ffmpeg -vf fps=2",
    )
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args()
    try:
        reference = json.loads((ROOT / "data" / "reference.json").read_text())
        manifest = json.loads((ROOT / "data" / "manifest.json").read_text())
        if digest(args.video) != manifest["source_video"]["sha256"]:
            raise ValueError("video hash differs; use the documented format 133 reference")
        with tempfile.TemporaryDirectory(prefix="nibbler-proof-") as temporary:
            directory = args.frames_dir or Path(temporary)
            if not args.frames_dir:
                maximum = max(wave["frame_index"] for wave in manifest["waves"]) + 4
                subprocess.run(
                    ["ffmpeg", "-v", "error", "-i", str(args.video), "-vf", "fps=2",
                     "-frames:v", str(maximum), str(directory / "frame-%04d.png")],
                    check=True, timeout=180,
                )
            verify_frames(directory, reference, manifest)
        paths = verify_sources(args.nibblerbot, reference, manifest)
        report = {"geometries": 8, "fruit_patterns": 11, "waves": 32,
                  "decoded_frame_hashes": 32, "primary_routes": paths,
                  "route_wall_collisions": 0, "fruit_counts": manifest["fruit_counts"]}
        args.report.write_text(json.dumps(report, indent=2) + "\n")
        print(json.dumps(report))
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        print(f"verify_reference: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
