# Legacy Epitech compatibility

Adapters for the original `IGame` / `IGraphic` interfaces, isolated from ABI 3.

```sh
./build/bin/arcade_legacy_probe /path/to/old_snake.so
ARCADE_LEGACY_GAME=/path/to/old_snake.so ./arcade ./lib/arcade_sdl2.so
ARCADE_LEGACY_GRAPHICS=/path/to/old_ncurses.so ./arcade ./lib/arcade_legacy_graphics.so
```

The variables can be combined. Without its variable, a wrapper is hidden from the menu.

## Contracts

| Element | Adaptation |
|---|---|
| Game | `entryPoint` / `createGame` exports, virtual order from `sdk/IGame.hpp` |
| Snake | `update(events, 0)` every 100 ms, initial UP event; `isGameRunning()==true` means a finished board |
| Restart | Recreate object and score generation |
| Display | `entryPoint` / `createGraphic`; initialize only at `open()` |
| Ownership | Object destroyed before library; singleton factories need a different contract |
| Determinism | Original API has no seed; historical maps are not guaranteed reproducible |

## Display profiles

| Profile | Use |
|---|---|
| `terminal` (default) | Real terminal and TERM required; 90×30 text coordinates, module-centered tiles, banner on actual final terminal row |
| `raw` | Same coordinates/HUD without requiring a tty, for tests and text integrations |
| `sdl` | Invert original SDL text mapping: x×17.5+100, y×40+200 |
| `sfml` | Invert original SFML text mapping: x×20+600, y×40+500 |

Terminal/raw escape `%` in two old printf HUD methods; `drawText` receives literal text. Legacy GUIs require their original assets/dependencies: contract conversion is tested, their visual rendering is not qualified.

| Original ncurses | Controls |
|---|---|
| Navigation | G: backend; B: game; M: menu; Escape: quit; Space: play/action |
| In game | H: secondary; J: pause; K: restart; L: difficulty |
| Menu | H/J: letter; K/L: name cursor position |

## Archives and tests

Unchanged copies from the initial Epitech project: `tests/legacy/original/{snake,curses}.{cpp,hpp}` and `sdk/{ArcadeData,IGame,IGraphic}.hpp`. They are ABI fixtures, excluded from shipped modules.

| Test | Scope |
|---|---|
| `legacy_abi` | Real Snake exports/semantics, 50 cycles |
| `legacy_graphics_abi` | 13 actions, HUD/name/cursor, `%n/%s/100%`, 20 cycles |
| `original_graphics_smoke` | Real ncurses, rendering and `ungetch`, three cycles, final board row preserved |

```sh
TERM=xterm-256color ./build/tests/original_graphics_smoke ./build/tests/original_ncurses.so
shasum -a 256 -c bonus/legacy/PROVENANCE.sha256
```

`ARCADE_TEST_LEGACY=OFF` disables fixtures. Targeted archive exceptions cover conversions, missing Clang `override`, and historical ncurses printf formats. The modern wrapper retains strict warnings and project formatting.
