# Optional CPU benchmarks

Scenarios, results, commands, and limitations: [profiling report](../../doc/performance.md). The normal build excludes these tools; they are not game plugins shipped in `./lib`.

| Tool | Purpose |
|---|---|
| `Scenario` | Real game models and `Core::frame()` |
| `Workspace` | Isolated plugins and temporary scores |
| `Digest` | Output hashes outside the timed interval |
| `Headless` | Real `drawLabel()` glyph expansion and rectangle consumption; no window/GPU |
| `FrameVariants`, `GlyphProbe` | Independent output checks |
| `compare.py` | Alternating A/B runs, raw JSONL retained |
| `profile.py` | Native macOS call graph in a separate run |
