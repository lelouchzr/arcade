# Arcade bonuses

**25 subject proposals**, enabled by `ARCADE_BONUS=ON` (default). `OFF` keeps Snake, Minesweeper, and the three required backends. Games start in classic mode.

## Access

| Game | D or F7 | Other bonus controls |
|---|---|---|
| Snake | Toggle pickups, acceleration, and boost during play | Space: boost |
| Minesweeper | Beginner → intermediate → expert → custom editor → beginner | ?: cycle marks; F: flag; arrows/digits: edit; Enter: apply; D/Backspace: cancel |
| Nibbler | Generated classic → generated bonus → historical pack → generated classic; starts a new session | Arrows: direction; Space: boost in bonus modes |
| Pacman | Classic ↔ Pacmania; starts a new session | Z/Space: jump; V: isometric/full-grid camera |
| Qix | Classic ↔ bonus; starts a new session | Arrows: move/cut; F: stop; collect by contact or trail |
| Centipede | Classic ↔ bonus; starts a new session | Player 1: arrows/Space; player 2: I up, K down, J left, L right, H shoot |
| Solarfox | Classic ↔ bonus; starts a new session | Arrows: turn; Space: shoot; F: boost |

F2/F3/F4/F5/F6: backend, game, restart, menu, pause. The HUD lists controls for the current mode.

## Coverage

| Game | Subject proposal | Implementation and evidence |
|---|---|---|
| Snake | Temporary bonus food | Separate +250 pickup; spawn/expiry/collection in `TimedPickup`, `tests/classics/SnakeTests.cpp`, `PickupTests.cpp` |
| Snake | Distinct head/body | Separate `Cell::Head`/`Cell::Body` styles; cells/growth in `tests/baseline/SnakeTests.cpp` |
| Snake | Increasing speed | Length-based interval with a floor; `tests/classics/SnakeTests.cpp` |
| Snake | Space boost | Temporary double speed, same simulation under split deltas; `tests/classics/SnakeTests.cpp` |
| Minesweeper | Question mark | Hidden → flag → question → hidden; F still toggles flags; `tests/classics/MinesTests.cpp` |
| Minesweeper | Reactive character | Normal, scared, victory-glasses, dead ASCII faces; `markingAndFacesTests()` |
| Minesweeper | Four difficulties | Exact subject presets and bounded custom editor; `customDifficultyTests()`, `editorBoundsTests()`, `largeBoardTests()` |
| Minesweeper | Win/loss animation | Six animated sprites and alternating text, stable score/mines/time; `markingAndFacesTests()`, `tests/Presentation.cpp` |
| Nibbler | Temporary score items | +500 without consuming level fruit or growing the snake; `fruitTests()` in `tests/classics/NibblerTests.cpp` |
| Nibbler | Distinct head/body | Head/body styles, four initial cells, growth in Nibbler suites |
| Nibbler | 32 levels from 1982 | Documented 32-wave reconstruction: 8 layouts, 11 fruit patterns, per-wave manifest and engine transitions |
| Nibbler | Increasing speed | Generated progression and historical per-wave speed; `NibblerBonus.cpp`, progression tests |
| Nibbler | Space boost | Double speed for 0.6 s, simultaneous turn retained; `boostTests()` |
| Pacman | Delayed fruit at spawn | Temporary fruit, +1000×level; spawn/collection/expiry/landing in `tests/games/PacmanBonusTests.cpp` |
| Pacman | Player/ghost jumps | Height, cooldown, two jumping ghosts, height-based collisions; `jumpTests()`, `heightCollisionTests()`, `ghostJumpTests()` |
| Pacman | Time-based acceleration | Capped acceleration plus level progression; `cameraAndSpeedTests()` |
| Pacman | Pacmania camera | Moving isometric view, V toggle, elevated sprites; `PacmanBonusTests.cpp`, `tests/Presentation.cpp`, `tests/bonus_integration.py` |
| Qix | Pickups by player/trail | Shield/slowdown, direct and trailing collection, expiry; `tests/games/QixBonusTests.cpp` |
| Qix | Obstacles/scenery | Four solid obstacles retained after captures; collision/flood-fill tests in `QixBonusTests.cpp` |
| Centipede | Arrivals while bodies remain | Three initial bodies and periodic arrivals; `tests/games/CentipedeBonusTests.cpp` |
| Centipede | Different body types | Standard/Racer/Weaver lengths, speeds, movement; fragments retain type; `CentipedeBonusTests.cpp` |
| Centipede | Two controllable characters | Independent input, common bounds, one shared shot, lethal second-player contact; `CentipedeBonusTests.cpp`, `tests/bonus_integration.py` |
| Solarfox | Multiple enemy shots | Normal/slow/fast/fan shots, speed/direction/interception; `tests/games/SolarfoxBonusTests.cpp` and classic suite |
| Solarfox | Moving enemies | Patrols on four edges, endpoint bounce; 100 patrol intervals in `SolarfoxBonusTests.cpp` |
| Solarfox | Arena obstacles | Four lethal rocks absorb both sides' shots; public-control collection of all powerups in `SolarfoxBonusTests.cpp` |

## Default parameters

| Game | Settings |
|---|---|
| Snake | +250 after 10 s, visible 5 s; period `max(0.06, 0.14−0.004×(length−4))`; ×2 boost for 0.6 s |
| Generated Nibbler | +500, 10/5 s timing; period `max(0.06, 0.16−0.003×level)`; ×2 boost for 0.6 s |
| Historical Nibbler | 32 waves, 8 layouts, 11 patterns; 27×27, 26..30 fruit; 90 s/wave; period `8/(60×code)` |
| Pacman | Fruit after 12 s, lasts 8 s, +1000×level; jump 0.8 s, height 1.4, cooldown 1.4 s; 17×13 camera |
| Qix | Pickups every 4 s, last 8 s, maximum 3; +250; shield/slowdown 5 s |
| Centipede | Standard/Racer/Weaver: 8/5/10 segments, periods 0.16/0.096/0.208 s; arrivals every 5 s, spawn cap 6 bodies |
| Solar Fox | Cannons: one cell/0.24 s; normal/slow/fast/fan shots; maximum 256 projectiles; four rocks |

| Difficulty | Dimensions | Mines | Initial time |
|---|---:|---:|---:|
| Beginner | 9×9 | 10 | 180 s |
| Intermediate | 16×16 | 40 | 420 s |
| Expert | 30×16 | 99 | 600 s |
| Custom | Width 5..40, height 5..24 | 1..area−9 | 30..3600 s |

Minesweeper editor: Up/Down selects a field; Left/Right or digits edit it; Enter applies; D/F7 or Backspace cancels. The first 3×3 area is protected and the camera follows the cursor.

## Historical Nibbler scope

The pack is a **verified data transcription**, not hardware emulation. `certified` covers topology, fruit, sequence, and speed codes; timing remains a 60 Hz model. Generated maps are a separate mode.

[Wave manifest](../bonus/games/nibbler/original/data/manifest.json) · [Data](../bonus/games/nibbler/original/data/reference.json) · [Verification](../bonus/games/nibbler/original/data/verification.json) · [Sources and guide](../bonus/games/nibbler/original/README.md).

Tests verify all 32 historical loads/transitions without playing every maze completely. All 32 generated maps are played through. External `ARCADE_NIBBLER_LEVELS` imports remain `PACK UNVERIFIED`.

## Tests

| CTest suite / build target | Scope |
|---|---|
| `unit` | Required rules, growth, mines, generated Nibbler progression, plugins, storage |
| `classic_bonus_gameplay` | `TimedPickup`, Snake bonuses, enhanced Minesweeper, Nibbler modes |
| `bonus_gameplay` | Pacman/Qix/Centipede/Solarfox classic/bonus rules, collisions, timing, controlled scenarios |
| `presentation` | Viewport, projection, sprites, overlapping actors, small camera, old ABI rejection |
| `bonus_integration` | Real core/plugins: jump, camera, question marks, editor, second player |
| `original_level_export` | Text export matches reference JSON; C++ compares all 32 compiled grids to this independent export |
| `header_checks`, `source_quality` | Standalone headers, definition placement, formatting |

Some playthroughs control enemies to isolate accessibility; shooting and collisions are tested separately. [Validation](VALIDATION.md) · [Ten extra backends](GRAPHICS.md).
