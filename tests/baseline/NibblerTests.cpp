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
#ifdef ARCADE_BONUS
void nibblerTests() {
    Plugin plugin("lib/arcade_nibbler.so");
    auto &game = plugin.instance<IGame>();
    game.reset(0);
    auto view = game.snapshot();
    require(count(view, Cell::Head) == 1 && count(view, Cell::Body) == 3,
            "nibbler must start with four cells");
    require(locate(view, Cell::Head).y == 19, "nibbler must start at bottom");
    const double tick = .157;
    for (int i = 0; i < 11; ++i)
        game.update(tick, {});
    require(locate(game.snapshot(), Cell::Head) == Point{23, 19},
            "nibbler straight movement incorrect");
    game.update(tick, {});
    require(locate(game.snapshot(), Cell::Head) == Point{23, 18},
            "nibbler failed automatic wall turn");
    require(game.snapshot().status == Status::Running, "nibbler died on a wall");
    game.reset(0);
    for (int i = 0; i < 8; ++i)
        game.update(tick, {});
    for (int i = 0; i < 9; ++i)
        game.update(tick, {{Action::Up}});
    game.update(tick, {{Action::Left}});
    const auto junction = locate(game.snapshot(), Cell::Head);
    require(junction == Point{19, 10}, "nibbler T-junction setup incorrect");
    game.update(tick, {});
    require(locate(game.snapshot(), Cell::Head) == junction, "nibbler must stop at a T-junction");
    game.update(tick, {{Action::Up}});
    require(locate(game.snapshot(), Cell::Head) == Point{19, 9},
            "nibbler must leave junction on input");
    require(game.snapshot().score > 0, "nibbler food score missing");
    game.update(60, {});
    game.update(30, {});
    require(game.snapshot().status == Status::Lost, "nibbler timer must end game");
    // Play all 32 stages using only public actions and visible cells.
    game.reset(0);
    Point heading{1, 0};
    bool advanced = false;
    int stage = 1;
    std::set<std::vector<Cell>> layouts;
    layouts.insert(game.snapshot().cells);
    for (int tickNumber = 0; tickNumber < 16000 && !advanced; ++tickNumber) {
        const auto board = game.snapshot();
        const auto head = locate(board, Cell::Head);
        std::vector<int> parent(board.cells.size(), -1);
        std::queue<Point> pending;
        pending.push(head);
        const int start = head.y * board.width + head.x;
        parent[static_cast<std::size_t>(start)] = start;
        int target = -1;
        const std::array<Point, 4> directions{{{1, 0}, {0, -1}, {-1, 0}, {0, 1}}};
        while (!pending.empty() && target < 0) {
            const auto point = pending.front();
            pending.pop();
            for (auto direction : directions) {
                if (point == head && direction == Point{-heading.x, -heading.y})
                    continue;
                const Point next{point.x + direction.x, point.y + direction.y};
                if (next.x < 0 || next.y < 0 || next.x >= board.width || next.y >= board.height)
                    continue;
                const auto offset = static_cast<std::size_t>(next.y * board.width + next.x);
                if (parent[offset] >= 0 || board.cells[offset] == Cell::Wall ||
                    board.cells[offset] == Cell::Body)
                    continue;
                parent[offset] = point.y * board.width + point.x;
                if (board.cells[offset] == Cell::Food) {
                    target = static_cast<int>(offset);
                    break;
                }
                pending.push(next);
            }
        }
        require(target >= 0, "nibbler stage has no reachable food");
        while (parent[static_cast<std::size_t>(target)] != start)
            target = parent[static_cast<std::size_t>(target)];
        const Point next{target % board.width, target / board.width};
        heading = {next.x - head.x, next.y - head.y};
        const auto action = heading.x > 0   ? Action::Right
                            : heading.x < 0 ? Action::Left
                            : heading.y > 0 ? Action::Down
                                            : Action::Up;
        game.update(std::max(.06, .16 - .003 * stage), {{action}});
        const auto after = game.snapshot();
        require(after.status != Status::Lost, "nibbler failed while completing its stages");
        if (after.status == Status::Won) {
            advanced = true;
            break;
        }
        const int nextStage = std::stoi(after.information[4].substr(7));
        if (nextStage != stage) {
            stage = nextStage;
            heading = {1, 0};
            layouts.insert(after.cells);
        }
    }
    require(advanced, "nibbler did not complete its 32 stages");
    require(layouts.size() == 32, "nibbler layouts are not distinct");
}
#endif

} // namespace arcade::baseline
