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
void snakeTests() {
    Snake game;
    game.reset(42);
    auto original = game.snapshot();
    require(count(original, Cell::Head) == 1 && count(original, Cell::Body) == 3,
            "snake must start at length four");
    require(count(original, Cell::Food) == 1, "snake must always have food");
    game.update(.139, {});
    require(locate(game.snapshot(), Cell::Head) == locate(original, Cell::Head),
            "snake moved too soon");
    game.update(.001, {{Action::Left}});
    require(locate(game.snapshot(), Cell::Head).x == 13, "reverse direction was accepted");
    game.update(.14, {{Action::Up}, {Action::Left}});
    require(locate(game.snapshot(), Cell::Head) == Point{13, 9}, "rapid double reversal");
    game.update(5, {});
    require(game.snapshot().status == Status::Lost, "snake must lose at wall");
    game.reset(42);
    require(game.snapshot().cells == original.cells, "seeded snake not reproducible");
    // Route directly to food on a fresh board; one perpendicular turn is sufficient.
    auto food = locate(original, Cell::Food);
    auto head = locate(original, Cell::Head);
    if (food.x < head.x) {
        const int detour = head.y == 1 ? 1 : -1;
        game.update(.14, {{detour > 0 ? Action::Down : Action::Up}});
        head = locate(game.snapshot(), Cell::Head);
        while (head.x != food.x && game.snapshot().status == Status::Running) {
            game.update(.14, {{Action::Left}});
            head = locate(game.snapshot(), Cell::Head);
        }
        while (head.y != food.y && game.snapshot().status == Status::Running) {
            game.update(.14, {{head.y < food.y ? Action::Down : Action::Up}});
            head = locate(game.snapshot(), Cell::Head);
        }
    } else {
        while (head.x != food.x && game.snapshot().status == Status::Running) {
            game.update(.14, {{Action::Right}});
            head = locate(game.snapshot(), Cell::Head);
        }
        while (head.y != food.y && game.snapshot().status == Status::Running) {
            game.update(.14, {{head.y < food.y ? Action::Down : Action::Up}});
            head = locate(game.snapshot(), Cell::Head);
        }
    }
    require(game.snapshot().score == 100, "food score missing");
    require(count(game.snapshot(), Cell::Body) == 4, "food must grow snake one cell");
    require(count(game.snapshot(), Cell::Food) == 1, "food was not respawned");
    bool invalid = false;
    try {
        game.update(std::numeric_limits<double>::quiet_NaN(), {});
    } catch (const std::invalid_argument &) {
        invalid = true;
    }
    require(invalid, "invalid delta was accepted");
}
} // namespace arcade::baseline
