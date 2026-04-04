# Nibbler: 32 historical waves

**8 maze layouts, 11 fruit patterns, 32 waves**, transcribed on a 27×27 grid from primary sources. Generated maps remain a separate mode.

`certified` means verified topology, fruit, sequence, and speed codes. It is not official certification, ROM/hardware emulation, cycle timing, or reproduction of the original palette, lives, growth, and bugs.

## Sources and evidence

| Source | Use |
|---|---|
| [NibblerBot video](https://www.youtube.com/watch?v=60gVbJTOUOo) | 32 frames and RGB hashes; format 133 video not included |
| [NibblerBot notes](https://github.com/FrankRizzo890/NibblerBot/blob/9511a82f8cb9a19ee4892d179581896d1a3de4d3/NibblerNotes.txt) | Composition, Easy/Hard codes, pinned revision |
| `Maze1..8` Lua routes | 90 paths without wall crossings; parsed, never executed |
| [Tim McVey guide](https://craftymech.com/nibbler-guide/) | Visual cross-check of eight waves |
| [Manifest](data/manifest.json) | Sources, nominal times, frame indices, SHA256 |
| [Reference](data/reference.json) · [Verification](data/verification.json) | Topology, fruit, results |

Fruit positions combine multiple frames to handle blinking/compression, requiring at least two observations per position. Original 19×19 maps were not the fruit reference.

| Pattern | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Fruits | 28 | 28 | 28 | 29 | 29 | 26 | 29 | 28 | 29 | 27 | 30 |

Period: `8/(60×speed_code)` seconds/cell, a **60 Hz model**, not cycle-accurate measurement. The built-in pack uses Easy; the exporter also preserves Hard.

## Repeat verification

Requires Python, Pillow, FFmpeg, format 133 video, and NibblerBot at the pinned revision. The C++ engine does not depend on them.

```sh
python3 bonus/games/nibbler/original/verify_reference.py \
  --video /path/to/nibblerbot.mp4 \
  --nibblerbot /path/to/NibblerBot \
  --report /tmp/nibbler-verification.json
```

The tool checks hashes, eight layouts, eleven patterns, 32 compositions, and 90 routes. Use `fps=2`, rather than separate `-ss` extraction, to reproduce frames.

## External pack

```sh
ARCADE_NIBBLER_LEVELS=bonus/games/nibbler/original/data/reference-pack.txt \
  ./arcade ./lib/arcade_ncurses.so
python3 bonus/games/nibbler/original/export_pack.py --check
python3 bonus/games/nibbler/original/export_pack.py --hard --output /tmp/nibbler-hard.txt
```

| Format | Rule |
|---|---|
| Header / provenance | `NIBBLER_PACK 1`, then `PROVENANCE "text"` |
| Wave | `LEVEL number maze pattern speed width height`, grid, `ENDLEVEL` |
| End | Exactly 32 ordered blocks, then `ENDPACK` |
| Dimensions | Square 19×19 or 27×27 |
| Symbols | `#` wall, space empty, `.` fruit, `H` head, `o` body |
| Validity | One head + three contiguous body segments, closed outer walls, reachable fruit |
| Budgets | 64 KiB/file, 2,048 bytes/line |

Imports remain **unverified**, regardless of claimed provenance. Invalid files produce a diagnostic without silent replacement.

[C++ tests](OriginalLevelsTests.cpp): data, round-trip import/export, invalid packs. Engine tests cover all 32 loads and last-fruit transitions, not full historical-map playthroughs.
