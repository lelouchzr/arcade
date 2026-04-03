#include "Fixtures.hpp"
#include "TestSupport.hpp"
#include <algorithm>
#include <cmath>
#include <queue>

namespace arcade::test {

const bonus::Grid &PacmanAccess::board(const bonus::Pacman &g) {
    return g.board_;
}

const std::array<bonus::Ghost, 4> &PacmanAccess::ghosts(const bonus::Pacman &g) {
    return g.ghosts_;
}

void PacmanAccess::player(bonus::Pacman &g, Point p, Point d) {
    g.player_ = p;
    g.heading_ = g.desired_ = d;
    g.playerClock_ = 0;
}

void PacmanAccess::ghost(bonus::Pacman &g, std::size_t i, Point p, bonus::GhostMode mode) {
    g.ghosts_.at(i) = {p, {}, mode, 0, 0};
}

double PacmanAccess::power(const bonus::Pacman &g) {
    return g.power_;
}

int PacmanAccess::level(const bonus::Pacman &g) {
    return g.level_;
}

void pacmanTests() {
    using namespace bonus;
    Pacman normal;
    const auto initial = normal.snapshot();
    require(count(initial, Cell::PowerPellet) == 4, "Pacman must have exactly four power pellets");
    const auto &board = PacmanAccess::board(normal);
    require(std::count(board.cells().begin(), board.cells().end(), Cell::Home) == 20,
            "ghost house is not 5x4");
    for (int y = 8; y <= 11; ++y)
        for (int x = 10; x <= 14; ++x)
            require(board.at({x, y}) == Cell::Home, "ghost house shape");
    PacmanAccess::player(normal, {12, 12}, {0, -1});
    normal.update(.14, {});
    require(locate(normal.snapshot(), Cell::Player) == Point{12, 12}, "player entered ghost house");
    normal.reset(1);
    PacmanAccess::player(normal, {1, 1}, {-1, 0});
    normal.update(9.99, {});
    for (const auto &g : PacmanAccess::ghosts(normal))
        require(g.mode == GhostMode::Caged, "ghost escaped before ten seconds");
    normal.update(.01, {});
    for (const auto &g : PacmanAccess::ghosts(normal))
        require(g.mode == GhostMode::Active, "ghost not released at ten seconds");
    normal.update(1, {});
    bool outside = false;
    for (const auto &g : PacmanAccess::ghosts(normal))
        outside = outside || PacmanAccess::board(normal).at(g.position) != Cell::Home;
    require(outside, "released ghosts cannot leave house");
    Pacman safe(PacmanRules{10000, 10, 2, .14, .20});
    PacmanAccess::player(safe, {0, 10}, {-1, 0});
    safe.update(.14, {});
    require(locate(safe.snapshot(), Cell::Player) == Point{24, 10},
            "horizontal tunnel does not wrap");
    PacmanAccess::player(safe, {12, 0}, {0, -1});
    safe.update(.14, {});
    require(locate(safe.snapshot(), Cell::Player) == Point{12, 20},
            "vertical tunnel does not wrap");
    safe.reset(1);
    PacmanAccess::player(safe, {2, 1}, {-1, 0});
    safe.update(.14, {});
    require(std::abs(PacmanAccess::power(safe) - 10) < 1e-6,
            "power pellet duration must be ten seconds");
    PacmanAccess::ghost(safe, 0, {3, 1}, GhostMode::Active);
    safe.update(.32, {});
    require(manhattan(PacmanAccess::ghosts(safe)[0].position, {1, 1}) > 2,
            "frightened ghost does not flee");
    PacmanAccess::ghost(safe, 0, {1, 1}, GhostMode::Active);
    safe.update(.01, {});
    require(PacmanAccess::ghosts(safe)[0].mode == GhostMode::Eyes,
            "eaten ghost did not become eyes");
    PacmanAccess::ghost(safe, 0, {12, 9}, GhostMode::Eyes);
    safe.update(.07, {});
    require(PacmanAccess::ghosts(safe)[0].mode == GhostMode::Healing, "eyes did not heal in house");
    safe.update(1.9, {});
    require(PacmanAccess::ghosts(safe)[0].mode == GhostMode::Healing, "healing ended too soon");
    safe.update(.2, {});
    require(PacmanAccess::ghosts(safe)[0].mode == GhostMode::Active,
            "healed ghost did not respawn");
    PacmanAccess::ghost(safe, 0, {20, 10}, GhostMode::Eyes);
    safe.update(1, {});
    require(PacmanAccess::ghosts(safe)[0].position == Point{12, 9} &&
                PacmanAccess::ghosts(safe)[0].mode == GhostMode::Healing,
            "eyes did not navigate back to their house");
    safe.update(10, {});
    require(PacmanAccess::power(safe) == 0, "power did not expire");
    normal.reset(1);
    PacmanAccess::player(normal, {2, 1}, {0, -1});
    PacmanAccess::ghost(normal, 0, {3, 1}, GhostMode::Active);
    normal.update(.2, {});
    require(normal.snapshot().status == Status::Lost, "normal ghost collision must lose");
    // Collect every real pellet in the generated maze through public controls.
    safe.reset(42);
    for (int step = 0; step < 3000 && PacmanAccess::level(safe) == 1; ++step) {
        const auto &map = PacmanAccess::board(safe);
        const auto start = locate(safe.snapshot(), Cell::Player);
        std::vector<int> parents(map.cells().size(), -1);
        std::queue<Point> pending;
        pending.push(start);
        const int origin = static_cast<int>(map.index(start));
        parents[map.index(start)] = origin;
        int target = -1;
        while (!pending.empty() && target < 0) {
            const auto p = pending.front();
            pending.pop();
            for (auto d : Directions) {
                const auto next = map.wrap(add(p, d));
                const auto i = map.index(next);
                if (parents[i] >= 0 || map.at(next) == Cell::Wall || map.at(next) == Cell::Home)
                    continue;
                parents[i] = static_cast<int>(map.index(p));
                if (map.at(next) == Cell::Pellet || map.at(next) == Cell::PowerPellet) {
                    target = static_cast<int>(i);
                    break;
                }
                pending.push(next);
            }
        }
        require(target >= 0, "Pacman contains unreachable pellets");
        while (parents[static_cast<std::size_t>(target)] != origin)
            target = parents[static_cast<std::size_t>(target)];
        const auto next = map.point(static_cast<std::size_t>(target));
        Point d{next.x - start.x, next.y - start.y};
        if (std::abs(d.x) > 1)
            d.x = d.x > 0 ? -1 : 1;
        if (std::abs(d.y) > 1)
            d.y = d.y > 0 ? -1 : 1;
        safe.update(.14, {{action(d)}});
        require(safe.snapshot().status == Status::Running, "safe maze traversal lost");
    }
    require(PacmanAccess::level(safe) == 2, "Pacman did not advance after all pellets");
    require(safe.snapshot().score > 3000, "Pacman pellet and power scores missing");
}

} // namespace arcade::test
