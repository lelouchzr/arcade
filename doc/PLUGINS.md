# Arcade SDK — ABI 3

Contract: `include/arcade/Api.hpp`, independent of graphics libraries.

```cpp
extern "C" const arcade::PluginDescriptor *arcade_plugin();
```

`ARCADE_PLUGIN` supplies the descriptor, `create`, and `destroy`. Unique ID: 1..48 characters from `a-z`, `0-9`, `_`, `-`.

## Interfaces

| Interface | Method | Contract |
|---|---|---|
| `IGame` | `reset(seed)` | Full reset, reproducible randomness |
| | `update(seconds, inputs)` | Simulated time and abstract actions; no SDL/SFML types |
| | `snapshot()` | Row-major grid, score, status, generation |
| `IGraphics` | `open()` | Initialize here, never during discovery |
| | `poll()` | Translate to `Action` / `Input`; characters use `Text` |
| | `draw(frame)` | Logical 1280×800 canvas or terminal grid |
| | `close()` | `noexcept`, repeatable, cleans up partial failures |

## Snapshot

| Field | Rule |
|---|---|
| Grid | 1..200 cells per axis; index `y * width + x` |
| `camera` | Optional; center and dimensions clamped by `BoardViewport` |
| `sprites` | Grid positions, finite elevation; overlays do not affect collisions |
| `information` | At most 7 lines of 23 characters |
| `secondsLeft` / `cursor` | −1: no countdown / `{-1,-1}`: hidden |
| `generation` | Changes for a new session, not a level that preserves the score |
| `Won` / `Lost` | Stable result; score saved once |

The core caps delta at 0.25 s and stops advancement while paused. Included games reject non-finite, negative, or >60 s deltas.

## Add a module

1. Use `src/games/snake/` or `src/graphics/sdl2/` as a complete example.
2. Declare the class in `.hpp`, implement its methods in `.cpp`.
3. Export `ARCADE_PLUGIN(MyGame, arcade::PluginKind::Game, "id", "NAME")`; use `Graphics` for a backend.
4. Add an `arcade_plugin(...)` CMake target and include it in staging. The core discovers the `.so` at startup.
5. Test startup, hot switching, restart, menu, and destruction.

## Lifetime and compatibility

`destroy` must release the object before `dlclose`. The descriptor remains valid while its module is loaded. The loader uses `RTLD_NOW | RTLD_LOCAL`; an explicitly requested incompatible backend returns **84**.

An interactive switch that fails restores the previous backend and marks the failed entry unavailable for the session. Game state and pause are preserved. Startup or rollback failure still returns **84**. Recovery tests.

Local C++ ABI: matching architecture, SDK version, and compatible toolchain. Rebuild old modules; use the [adapters](../bonus/legacy/README.md) for the legacy Epitech contract.

Sharing between groups requires exchanging headers and running actual cross-project tests; no external agreement is claimed.
