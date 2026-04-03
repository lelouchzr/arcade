#include "Support.hpp"
#include "Minesweeper.hpp"
#include "Snake.hpp"
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
int checks = 0;

void require(bool condition, const char *message) {
    ++checks;
    if (!condition)
        throw std::runtime_error(message);
}

int count(const Snapshot &s, Cell c) {
    return static_cast<int>(std::count(s.cells.begin(), s.cells.end(), c));
}

Point locate(const Snapshot &s, Cell c) {
    const auto found = std::find(s.cells.begin(), s.cells.end(), c);
    if (found == s.cells.end())
        return {-1, -1};
    const auto offset = static_cast<int>(found - s.cells.begin());
    return {offset % s.width, offset / s.width};
}

void moveCursor(Minesweeper &game, Point target) {
    auto cursor = game.snapshot().cursor;
    while (cursor.x != target.x && game.snapshot().status == Status::Running) {
        game.update(0, {{cursor.x < target.x ? Action::Right : Action::Left}});
        cursor = game.snapshot().cursor;
    }
    while (cursor.y != target.y && game.snapshot().status == Status::Running) {
        game.update(0, {{cursor.y < target.y ? Action::Down : Action::Up}});
        cursor = game.snapshot().cursor;
    }
}
} // namespace arcade::baseline
