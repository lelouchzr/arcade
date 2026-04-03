#include "Minesweeper.hpp"
#include "Snake.hpp"
#include "Support.hpp"
#include "arcade/Font.hpp"
#include "arcade/Plugin.hpp"
#include "arcade/Scores.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <set>
#include <stdexcept>
#include <unistd.h>

namespace arcade::baseline {
void mineTests() {
    Minesweeper game;
    for (std::uint64_t seed = 0; seed < 200; ++seed) {
        game.reset(seed);
        const auto before = game.snapshot();
        require(count(before, Cell::Hidden) == 81, "mines initial grid must be hidden");
        game.update(1, {});
        require(game.snapshot().secondsLeft == 180, "timer ran before reveal");
        game.update(0, {{Action::Secondary}});
        require(count(game.snapshot(), Cell::Flag) == 1, "flag toggle failed");
        game.update(0, {{Action::Primary}});
        require(game.snapshot().score == 0, "revealed flagged square");
        game.update(0, {{Action::Secondary}, {Action::Primary}});
        const auto first = game.snapshot();
        require(
            first.cells[static_cast<std::size_t>(first.cursor.y * first.width + first.cursor.x)] ==
                Cell::Zero,
            "first reveal is not zero");
        require(first.score >= 90, "zero flood did not reveal neighbours");
        game.update(60, {});
        game.update(60, {});
        game.update(60, {});
        require(game.snapshot().status == Status::Lost, "mines timer did not expire");
        require(count(game.snapshot(), Cell::Mine) == 10, "loss must reveal all mines");
    }
    // Learn a seeded board by timing it out, reset same seed, reveal exactly its safe cells.
    game.reset(777);
    game.update(0, {{Action::Primary}});
    game.update(60, {});
    game.update(60, {});
    game.update(60, {});
    const auto known = game.snapshot();
    game.reset(777);
    game.update(0, {{Action::Primary}});
    for (int y = 0; y < 9; ++y)
        for (int x = 0; x < 9; ++x)
            if (known.cells[static_cast<std::size_t>(y * 9 + x)] != Cell::Mine) {
                moveCursor(game, {x, y});
                game.update(0, {{Action::Primary}});
            }
    require(game.snapshot().status == Status::Won, "revealing all safe cells must win");
    require(game.snapshot().score == 71 * 10 + 180, "remaining time was not added to score");
    const auto won = game.snapshot().score;
    game.update(1, {{Action::Primary}});
    require(game.snapshot().score == won, "win bonus credited multiple times");
    game.reset(777);
    game.update(0, {{Action::Primary}});
    for (int y = 0; y < 9; ++y)
        for (int x = 0; x < 9; ++x)
            if (known.cells[static_cast<std::size_t>(y * 9 + x)] == Cell::Mine) {
                moveCursor(game, {x, y});
                game.update(0, {{Action::Primary}});
                goto revealedMine;
            }
revealedMine:
    require(game.snapshot().status == Status::Lost, "mine collision must lose");
#ifdef ARCADE_BONUS
    game.update(0, {{Action::Difficulty}});
    require(game.snapshot().width == 16, "intermediate difficulty missing");
    game.update(0, {{Action::Difficulty}});
    require(game.snapshot().width == 30, "expert difficulty missing");
    game.update(0, {{Action::Difficulty}});
    require(game.snapshot().width == 9, "difficulty cycle broken");
#endif
}
} // namespace arcade::baseline
