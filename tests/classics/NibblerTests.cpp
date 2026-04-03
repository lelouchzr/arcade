#include "Access.hpp"
#include "Cases.hpp"
#include "games/nibbler/original/OriginalLevels.hpp"
#include <algorithm>
#include <array>
#include <cmath>

namespace arcade::test {
namespace {
void fruitTests() {
    bonus::Nibbler game;
    game.update(0, {{Action::Difficulty}});
    requireHud(game.snapshot());
    const int food = bonus::test::NibblerBonusAccess::food(game);
    bonus::test::NibblerBonusAccess::pickup(game, {13, 19});
    game.update(0.157, {});
    requireClassic(game.snapshot().score == 500 && countClassic(game.snapshot(), Cell::Body) == 3 &&
                       bonus::test::NibblerBonusAccess::food(game) == food,
                   "nibbler bonus must award points without changing level food or growth");
    bonus::Nibbler waiting;
    waiting.update(0, {{Action::Difficulty}});
    bonus::test::NibblerBonusAccess::junction(waiting);
    bonus::Nibbler partitioned;
    partitioned.update(0, {{Action::Difficulty}});
    bonus::test::NibblerBonusAccess::junction(partitioned);
    waiting.update(10.01, {});
    for (int i = 0; i < 1001; ++i)
        partitioned.update(0.01, {});
    requireClassic(waiting.snapshot().cells == partitioned.snapshot().cells,
                   "bonus spawn must be deterministic across delta partitions");
    requireClassic(countClassic(waiting.snapshot(), Cell::BonusFood) == 1 &&
                       countClassic(waiting.snapshot(), Cell::Food) == 1,
                   "timed bonus must spawn on a free square while retaining regular food");
    waiting.update(5, {});
    requireClassic(countClassic(waiting.snapshot(), Cell::BonusFood) == 0,
                   "uncollected nibbler bonus must expire");
    waiting.update(10.01, {});
    requireClassic(countClassic(waiting.snapshot(), Cell::BonusFood) == 1,
                   "nibbler bonus must respawn after its cooldown");
}

void accessiblePickupTests() {
    for (std::uint64_t seed = 0; seed < 32; ++seed) {
        bonus::Nibbler game;
        game.reset(seed);
        game.update(0, {{Action::Difficulty}});
        bonus::test::NibblerBonusAccess::isolatedFloor(game);
        game.update(10.01, {});
        requireClassic(game.snapshot().cells[11 * 25 + 13] == Cell::BonusFood &&
                           game.snapshot().cells[25 + 1] == Cell::Empty,
                       "nibbler bonus must not appear in an enclosed decorative floor area");
    }
}

void boostTests() {
    bonus::Nibbler turning;
    turning.update(0, {{Action::Difficulty}});
    turning.update(0.0785, {{Action::Up}, {Action::Primary}});
    requireClassic(turning.snapshot().cells[18 * 25 + 12] == Cell::Head,
                   "boost input must not overwrite a queued nibbler turn");
    bonus::Nibbler single, split;
    single.update(0, {{Action::Difficulty}});
    split.update(0, {{Action::Difficulty}});
    single.update(0.7, {{Action::Primary}});
    split.update(0.01, {{Action::Primary}});
    for (int i = 1; i < 70; ++i)
        split.update(0.01, {});
    requireClassic(single.snapshot().cells == split.snapshot().cells &&
                       single.snapshot().score == split.snapshot().score,
                   "nibbler boost must be independent of delta partition");
    requireClassic(single.snapshot().status == Status::Running, "nibbler boost test must survive");
    single.reset(1);
    requireClassic(countClassic(single.snapshot(), Cell::BonusFood) == 0 &&
                       single.snapshot().score == 0,
                   "nibbler reset must clear pickup and score");
}

void levelTimePartitionTests() {
    for (int mode = 0; mode < 3; ++mode) {
        bonus::Nibbler single, split;
        for (int enable = 0; enable < mode; ++enable) {
            single.update(0, {{Action::Difficulty}});
            split.update(0, {{Action::Difficulty}});
        }
        bonus::test::NibblerBonusAccess::lastFood(single);
        bonus::test::NibblerBonusAccess::lastFood(split);
        const double period = mode == 2 ? bonus::OriginalLevels::level(1).period : 0.157;
        single.update(period * 2, {});
        split.update(period, {});
        split.update(period, {});
        const auto a = single.snapshot(), b = split.snapshot();
        requireClassic(bonus::test::NibblerBonusAccess::level(single) == 2 &&
                           bonus::test::NibblerBonusAccess::level(split) == 2,
                       "time partition scenario must cross a level boundary");
        requireClassic(a.cells == b.cells && a.score == b.score && a.status == b.status &&
                           std::abs(a.secondsLeft - b.secondsLeft) < 1e-9 && a.secondsLeft < 90,
                       "level transition must retain elapsed time and use the new level speed");
    }
}

void originalPackTests() {
    constexpr std::array<std::size_t, 11> fruitCounts{28, 28, 28, 29, 29, 26, 29, 28, 29, 27, 30};
    bonus::Nibbler game;
    game.update(0, {{Action::Difficulty}});
    game.reset(1);
    const auto classic = game.snapshot().cells;
    game.update(0, {{Action::Difficulty}});
    const std::string evidence =
        bonus::OriginalLevels::level(1).certified ? "ORIGINAL 1982" : "UNVERIFIED";
    requireClassic(game.snapshot().information.back().find(evidence) != std::string::npos,
                   "reconstructed pack must expose evidence status");
    requireClassic(game.snapshot().information.size() <= 7, "nibbler HUD must fit all backends");
    for (int wave = 1; wave <= 32; ++wave) {
        bonus::test::NibblerBonusAccess::originalLevel(game, wave);
        const auto view = game.snapshot();
        requireHud(view);
        const auto &source = bonus::OriginalLevels::level(wave);
        requireClassic(view.width == 27 && view.height == 27 &&
                           countClassic(view, Cell::Head) == 1 &&
                           countClassic(view, Cell::Body) == 3 &&
                           countClassic(view, Cell::Food) ==
                               fruitCounts.at(static_cast<std::size_t>(source.fruitPattern - 1)),
                       "original wave must load its 27x27 board, four snake cells, and historical "
                       "fruit count");
        const auto head = static_cast<std::size_t>(
            std::find(view.cells.begin(), view.cells.end(), Cell::Head) - view.cells.begin());
        requireClassic(view.camera && !view.camera->isometric && view.camera->columns == 27 &&
                           view.camera->rows == 21 &&
                           view.camera->center ==
                               Point{static_cast<int>(head % 27), static_cast<int>(head / 27)},
                       "original wave camera must follow head for terminal navigation");
        for (int y = 0; y < source.height; ++y)
            for (int x = 0; x < source.width; ++x)
                requireClassic(
                    (view.cells[static_cast<std::size_t>(y * view.width + x)] == Cell::Food) ==
                        (source.rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] ==
                         '.'),
                    "original wave must preserve every historical fruit coordinate");
        bonus::test::NibblerBonusAccess::lastFood(game);
        game.update(bonus::OriginalLevels::level(wave).period, {});
        requireClassic(wave == 32 ? game.snapshot().status == Status::Won
                                  : bonus::test::NibblerBonusAccess::level(game) == wave + 1,
                       "reconstructed wave must advance after its last fruit");
    }
    game.update(0, {{Action::Difficulty}});
    requireClassic(game.snapshot().cells == classic,
                   "mode cycle must restore classic generated layout");
}
} // namespace

void nibblerBonusTests() {
    fruitTests();
    accessiblePickupTests();
    boostTests();
    originalPackTests();
    levelTimePartitionTests();
}
} // namespace arcade::test
