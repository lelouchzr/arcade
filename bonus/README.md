# Arcade extensions

`ARCADE_BONUS=ON` enables extensions; `OFF` keeps the core. D/F7 selects a mode or difficulty. [Controls and all 25 bonus proposals](../doc/BONUSES.md).

| Directory | Contents |
|---|---|
| `games/classics/` | Shared temporary pickup: delay, lifetime, single collection, respawn |
| `games/snake/` | Bonus food, length-based speed, Space boost |
| `games/minesweeper/` | Custom editor, question marks, faces, end animations |
| `games/nibbler/` | Model, pickups, boost, generated/historical levels; data and loader in `original/` |
| `games/pacman/` | Ghosts, fruit, jumps, height-based collisions, Pacmania camera |
| `games/qix/` | Capture, trails, fuse, shield, slowdown, solid obstacles |
| `games/centipede/` | Splitting, obstacles, simultaneous arrivals, Racer/Weaver, second player |
| `games/solarfox/` | Shooting-based collection, interception, moving cannons, projectile types, obstacles |
| `MineDifficulties.hpp` | Beginner 9×9/10, intermediate 16×16/40, expert 30×16/99 |
| `legacy/` | Game/display adapters, archived SDK, fixture provenance |
| `benchmarks/` | Deterministic CPU harness, A/B comparison, separate profiling runs |

Types live in `.hpp`, methods in `.cpp` by responsibility. The SDK carries camera/sprite data without graphics dependencies in game rules.

Nibbler separates generated maps from its historical pack: 32 waves, 8 layouts, 11 fruit patterns. [Data and limitations](games/nibbler/original/README.md).

Build variants use separate directories. `make` restores bonus staging at the root after a core-only build.

[Extra backends](../doc/GRAPHICS.md) · [Tests](../doc/VALIDATION.md) · [Legacy compatibility](legacy/README.md).
