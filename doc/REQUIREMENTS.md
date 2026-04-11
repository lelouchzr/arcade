# Arcade requirements

Core: Snake and Minesweeper. Extensions: five more games, [25 bonuses](BONUSES.md), [ten extra backends](GRAPHICS.md).

## Platform

| Requirement | Implementation | Evidence |
|---|---|---|
| C++ executable, Make/CMake, exits 0/84 | `src/main.cpp`, Makefile, CMake | `tests/integration.py`: arguments, missing module, game passed as backend, help |
| Dynamic ncurses/SDL2/SFML | Separate `src/graphics/` classes, `.so` files in `lib/` | Real `display_smoke`, core `ldd`/`otool` |
| Encapsulated `dlopen/dlsym/dlerror/dlclose` | `SharedLibrary`, `Plugin` in `src/loader/` | 100 load/destruction cycles, missing symbols/files |
| Uniform plugin handling | Common descriptor and `arcade_plugin` entry; no backend classes in core | All games through real core and two scripted backends |
| Rendering-independent games | ABI 3 `IGame`, `Snapshot`, `Cell`, semantic camera/sprites | No backend linkage/includes; old SDK rejected by `presentation` |
| Four-panel menu | `CorePresentation.cpp`, editable name, scrolling lists, scores | Name integration, all seven games reached, screenshots |
| Switch backend/game, restart/menu/quit | `CoreInput.cpp`, ownership in `Core.cpp` | Real integration actions and native backend tests |
| Extension documentation | `doc/PLUGINS.md`, public SDK | Standalone headers, module example |
| Clean source organization | `.hpp` types, `.cpp` implementations, domain directories | `header_checks`, `source_quality`, default `-Werror` |

## Snake, Minesweeper, Nibbler

| Game | Verified rules | Evidence |
|---|---|---|
| Snake | Four initial cells, fixed step, no reversal/double turn, growth, vacated tail, loss, full-board win | `tests/baseline/SnakeTests.cpp` |
| Minesweeper | 200 grids, protected first zero, flood fill, flags, timer starts at first reveal, mines revealed on loss, time awarded once on win | `tests/baseline/MinesweeperTests.cpp` |
| Nibbler | Four-cell start, forced turns, T-junction stop, food/timer, 32 generated maps played through | `tests/baseline/NibblerTests.cpp` |

Historical pack: independently verified data and 32 transitions, without full playthrough or hardware emulation. [Sources and limits](../bonus/games/nibbler/original/README.md).

## Pacman

| Rule | Implementation / test |
|---|---|
| Maze and wrapping tunnels | `Pacman.cpp`, horizontal/vertical wrapping |
| Pellets on every walkable cell | Generation and BFS collection of every pellet |
| Central 5×4 home, player below | Twenty Home cells checked; player entry rejected |
| Four ghosts released after 10 s | Checks at 9.99/10 s and actual home exit |
| Four power pellets, 10 s effect | Count/duration; ghost model independent of rendering |
| Blue, slower, fleeing ghosts | `GhostMode`, `FrightenedGhost`, period ×1.6, fleeing test |
| Normal contact is lethal | `update()` scenario returns `Lost` |
| Eaten ghost becomes eyes, returns, recovers | Score, distant navigation, home stay, respawn |
| Win advances to faster map | Public-control full level, score preserved, level 2 |

## Qix

| Rule | Implementation / test |
|---|---|
| Walkable/captured/boundary types | Distinct cells, boundaries recalculated after cuts |
| Autonomous multi-cell monster | Three segments, movement, seeded reproducibility |
| Trail cannot be crossed | Re-entering own trail rejected |
| Capture region without Qix | Flood fill seeded from **all** segments; monster side retained |
| Win below 25% remaining area | Four public-control cuts return `Won` |
| Fuse while stopped | Ignition, fire progression, loss, return to start |
| Qix crosses trail/player | Lethal collision checked |
| Sparks chase on old boundaries/trails | Persistent paths, pursuit through captured regions, lethal contact |
| No Spark reversal | Dead-end behavior checked |

## Centipede

| Rule | Implementation / test |
|---|---|
| Player in bottom 20%, full width | Bounds y=17..20, x=0..24 |
| Bodies descend from the top | Horizontal movement, obstacle/edge row change |
| Shots split bodies | Two independent fragments after central hit |
| Hit segment becomes obstacle | New obstacle with five hit points |
| Head continues, tail descends/reverses | Horizontal/vertical split tests; new obstacle not entered |
| Five hits destroy obstacle | Exact hit count and score |
| One shot at a time | Repeated firing does not create extra projectiles |
| Lethal contact, bottom penalty | Collision and score deduction |
| New map after twenty waves | Fragments do not count as waves; twenty waves via public controls, level 2 |
| Random map | Seeded obstacles, regenerated next level |

## Solar Fox

| Rule | Implementation / test |
|---|---|
| Central arena, three-cell margin | Margin blocked, lethal edge |
| Continuous motion, no reversal | Direct reversal and two turns in one tick rejected |
| Boost | Actual double speed |
| Collect by shooting, not contact | Separate ship/projectile tests |
| Two-cell range, shots four times faster | Collection at 2 but not 3; shot expires before ship movement |
| Enemy cannons/shots | Four cannons, projectiles under default rules |
| Interception | Both shots cancelled, including simultaneous arrival on last range cell |
| Enemy-shot/wrong-item collision | Scenarios return `Lost` |
| Map completion | All powerups shot via public controls, next map, score retained |

## Evidence scope

Public-control playthroughs complete 32 **generated** Nibbler maps, one Pacman map, Qix in four cuts, twenty Centipede waves, and one Solar Fox map. Some enemies are controlled to isolate traversal; normal collisions/shots are tested separately.

| Cross-cutting check | Evidence |
|---|---|
| Scores | 8 processes, 96 writes, merged top 10, fresh readers |
| Legacy Snake | Archived source, real ABI/tick/status, 50 unload cycles |
| Legacy display | 13 actions, HUD/cursor/percent signs, 20 cycles, real terminal ncurses |
| Interface sharing | External agreement and cross-project tests still required |

[Validation](VALIDATION.md) · [Legacy adapters](../bonus/legacy/README.md).
