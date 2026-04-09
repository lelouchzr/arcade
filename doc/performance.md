# Profiling

Benchmarks measure game updates, snapshots, glyph expansion and CPU frame preparation. They do not measure GPU work or display FPS.

| Optimization | Preserved behavior |
|---|---|
| Pacman BFS queue and direct neighbors | Traversal order and ghost decisions |
| Constant glyph table | All byte values and normalization rules |
| Reserved frame storage | Rectangles, labels and presentation order |

## Reproduce locally

```sh
cmake -S . -B build-profile -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS_RELEASE='-O3 -DNDEBUG -g -fno-omit-frame-pointer' \
  -DARCADE_BENCHMARKS=ON -DARCADE_BUILD_GRAPHICS=OFF \
  -DARCADE_BONUS=ON -DARCADE_STRICT=ON -DBUILD_TESTING=OFF
cmake --build build-profile --target arcade_benchmark arcade_frame_validation arcade_glyph_probe --parallel 4
build-profile/benchmarks/arcade_benchmark --module-dir build-profile/lib \
  --graphics build-profile/benchmarks/arcade_bench_headless.so \
  --case all --iterations 50000 --verify
build-profile/benchmarks/arcade_frame_validation build-profile/lib \
  build-profile/benchmarks/arcade_bench_headless.so
build-profile/benchmarks/arcade_glyph_probe
```

For A/B comparisons, build both revisions with the same harness, flags and dependencies. Warm up and alternate repeated trials; compare traces and complete frames before interpreting speed changes. Use `bonus/benchmarks/compare.py` and `profile.py` with output paths under `/tmp/`. Do not commit profiler captures, machine metadata or memory reports. [Tests](VALIDATION.md).
