# Testing

Strict builds use `-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`. Test availability depends on build options, installed backends and platform tools.

| Area | Coverage |
|---|---|
| Games | Rules, collisions, bonuses, board generation and historical Nibbler data |
| Core | Menus, pause/restart, switching, concurrent scores and failed-backend recovery |
| Presentation | Glyphs, sprites, clipping, cameras and isometric projection |
| Plugins | ABI rejection, ownership, unloading and legacy contracts |
| Native backends | Offscreen captures, real-window checks and terminal/display recovery |

```sh
make test
cmake -S . -B build-basic -DARCADE_BONUS=OFF -DARCADE_STRICT=ON
cmake --build build-basic --parallel
ctest --test-dir build-basic --output-on-failure
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  -DARCADE_BUILD_GRAPHICS=OFF -DARCADE_SANITIZERS=ON
cmake --build build-asan --parallel
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  ctest --test-dir build-asan --output-on-failure
```

Leak checking is qualified on Linux. The NDK backend releases its libpanel state; native graphical libraries can retain process-wide caches. Checked terminal/nonwindow paths and third-party graphical windows have different memory scopes. Keep raw memory reports and machine diagnostics outside Git.

[Requirements](REQUIREMENTS.md) · [Backends](GRAPHICS.md) · [Nibbler data](../bonus/games/nibbler/original/README.md) · [Profiling](performance.md).
