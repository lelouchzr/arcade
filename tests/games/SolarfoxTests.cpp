#include "Fixtures.hpp"
#include "TestSupport.hpp"
#include <algorithm>
#include <array>
#include <queue>

namespace arcade::test {

void SolarfoxAccess::player(bonus::Solarfox &g, Point p, Point d) {
    g.player_ = p;
    g.heading_ = g.desired_ = d;
    g.turnQueued_ = false;
    g.movementClock_ = g.shotClock_ = g.enemyShotClock_ = g.accumulated_ = 0;
    g.shots_.clear();
    g.enemyShots_.clear();
}

void SolarfoxAccess::clearField(bonus::Solarfox &g) {
    for (int y = 3; y < 18; ++y)
        for (int x = 3; x < 22; ++x)
            g.board_.at({x, y}) = Cell::Empty;
    g.powerups_ = 0;
    g.guns_.clear();
}

void SolarfoxAccess::powerup(bonus::Solarfox &g, Point p, bool bad) {
    if (g.board_.at(p) == Cell::Powerup)
        --g.powerups_;
    g.board_.at(p) = bad ? Cell::BadPowerup : Cell::Powerup;
    if (!bad)
        ++g.powerups_;
}

void SolarfoxAccess::enemyShot(bonus::Solarfox &g, Point p, Point d) {
    g.enemyShots_.push_back({p, d, 40});
}

const std::vector<bonus::SolarShot> &SolarfoxAccess::shots(const bonus::Solarfox &g) {
    return g.shots_;
}

const std::vector<bonus::SolarShot> &SolarfoxAccess::enemyShots(const bonus::Solarfox &g) {
    return g.enemyShots_;
}

const bonus::Grid &SolarfoxAccess::board(const bonus::Solarfox &g) {
    return g.board_;
}

int SolarfoxAccess::level(const bonus::Solarfox &g) {
    return g.level_;
}

int SolarfoxAccess::remaining(const bonus::Solarfox &g) {
    return g.powerups_;
}

void SolarfoxAccess::simultaneousShots(bonus::Solarfox &g) {
    g.shots_ = {bonus::SolarShot{{5, 5}, {1, 0}, 1}};
    g.enemyShots_ = {bonus::SolarShot{{7, 5}, {-1, 0}, 10}};
    g.shotClock_ = .040;
    g.enemyShotClock_ = .085;
}

void solarfoxTests() {
    using namespace bonus;
    Solarfox game(SolarfoxRules{.18, .09, 10000});
    const auto initial = game.snapshot();
    require(count(initial, Cell::EnemyGun) == 4, "Solarfox enemy guns missing");
    require(count(initial, Cell::Powerup) > 0, "Solarfox powerups missing");
    game.update(.18, {{Action::Left}});
    require(locate(game.snapshot(), Cell::Ship) == Point{13, 10},
            "Solarfox accepted direct reversal");
    game.reset(1);
    game.update(.18, {{Action::Up}, {Action::Left}});
    require(locate(game.snapshot(), Cell::Ship) == Point{12, 9},
            "Solarfox accepted rapid double reversal");
    game.reset(1);
    SolarfoxAccess::clearField(game);
    SolarfoxAccess::powerup(game, {14, 10});
    SolarfoxAccess::powerup(game, {20, 15});
    SolarfoxAccess::player(game, {12, 10}, {1, 0});
    game.update(.09, {{Action::Primary}});
    require(SolarfoxAccess::remaining(game) == 1 && game.snapshot().score == 100,
            "two-cell shot did not collect powerup");
    require(SolarfoxAccess::shots(game).empty(), "projectile did not expire after two cells");
    require(locate(game.snapshot(), Cell::Ship) == Point{12, 10},
            "projectile is not four times faster than ship");
    SolarfoxAccess::player(game, {12, 10}, {1, 0});
    SolarfoxAccess::powerup(game, {15, 10});
    game.update(.135, {{Action::Primary}});
    require(SolarfoxAccess::remaining(game) == 2, "projectile exceeded two-cell range");
    SolarfoxAccess::player(game, {14, 10}, {1, 0});
    game.update(.18, {});
    require(locate(game.snapshot(), Cell::Ship) == Point{15, 10} &&
                SolarfoxAccess::remaining(game) == 2,
            "contact collected powerup without shooting");
    SolarfoxAccess::player(game, {12, 10}, {1, 0});
    SolarfoxAccess::enemyShot(game, {13, 10}, {-1, 0});
    game.update(.045, {{Action::Primary}});
    require(SolarfoxAccess::shots(game).empty() && SolarfoxAccess::enemyShots(game).empty(),
            "projectiles failed to intercept");
    SolarfoxAccess::player(game, {12, 10}, {1, 0});
    SolarfoxAccess::simultaneousShots(game);
    const auto before = game.snapshot().score;
    game.update(.005, {});
    require(SolarfoxAccess::shots(game).empty() && SolarfoxAccess::enemyShots(game).empty() &&
                game.snapshot().score == before + 25,
            "simultaneous interception on final range cell failed");
    game.reset(1);
    SolarfoxAccess::player(game, {12, 10}, {1, 0});
    game.update(.09, {{Action::Secondary}});
    require(locate(game.snapshot(), Cell::Ship) == Point{13, 10},
            "Solarfox boost did not accelerate ship");
    game.reset(1);
    SolarfoxAccess::player(game, {21, 10}, {1, 0});
    game.update(.18, {});
    require(game.snapshot().status == Status::Lost, "Solarfox border collision did not lose");
    game.reset(1);
    SolarfoxAccess::player(game, {6, 7}, {1, 0});
    game.update(.18, {});
    require(game.snapshot().status == Status::Lost, "bad powerup collision did not lose");
    game.reset(1);
    SolarfoxAccess::player(game, {12, 10}, {1, 0});
    SolarfoxAccess::enemyShot(game, {13, 10}, {-1, 0});
    game.update(.09, {});
    require(game.snapshot().status == Status::Lost, "enemy projectile did not kill ship");
    game.reset(1);
    SolarfoxAccess::clearField(game);
    SolarfoxAccess::powerup(game, {14, 10});
    SolarfoxAccess::player(game, {12, 10}, {1, 0});
    game.update(.09, {{Action::Primary}});
    require(SolarfoxAccess::level(game) == 2 && SolarfoxAccess::remaining(game) > 0,
            "Solarfox completion did not reload next level");
    require(game.snapshot().score == 100, "Solarfox progression lost score");
    Solarfox dangerous;
    dangerous.update(1.2, {});
    require(count(dangerous.snapshot(), Cell::EnemyProjectile) > 0, "enemy guns never fired");
    Solarfox complete(SolarfoxRules{.18, .09, 10000});
    int facing = 0;
    for (int turn = 0; turn < 2000 && SolarfoxAccess::level(complete) == 1; ++turn) {
        const auto &map = SolarfoxAccess::board(complete);
        const auto player = locate(complete.snapshot(), Cell::Ship);
        const auto canShoot = [&](Point p, int d) {
            for (int range = 1; range <= 2; ++range) {
                p = add(p, Directions[static_cast<std::size_t>(d)]);
                if (!map.contains(p) || map.at(p) == Cell::Wall || map.at(p) == Cell::BadPowerup)
                    return false;
                if (map.at(p) == Cell::Powerup)
                    return true;
            }
            return false;
        };
        if (canShoot(player, facing)) {
            complete.update(.18, {{Action::Primary}});
            continue;
        }
        const int start = static_cast<int>(map.index(player)) * 4 + facing;
        std::vector<int> parent(map.cells().size() * 4, -1);
        std::queue<int> pending;
        pending.push(start);
        parent[static_cast<std::size_t>(start)] = start;
        int goal = -1;
        while (!pending.empty() && goal < 0) {
            const int state = pending.front();
            pending.pop();
            const auto position = map.point(static_cast<std::size_t>(state / 4));
            const int heading = state % 4;
            for (int d = 0; d < 4; ++d) {
                if (d == (heading + 2) % 4)
                    continue;
                const auto next = add(position, Directions[static_cast<std::size_t>(d)]);
                if (!map.contains(next) || map.at(next) == Cell::Wall ||
                    map.at(next) == Cell::BadPowerup || map.at(next) == Cell::EnemyGun)
                    continue;
                const int key = static_cast<int>(map.index(next)) * 4 + d;
                if (parent[static_cast<std::size_t>(key)] >= 0)
                    continue;
                parent[static_cast<std::size_t>(key)] = state;
                if (canShoot(next, d)) {
                    goal = key;
                    break;
                }
                pending.push(key);
            }
        }
        require(goal >= 0, "Solarfox contains a powerup unreachable by shots");
        while (parent[static_cast<std::size_t>(goal)] != start)
            goal = parent[static_cast<std::size_t>(goal)];
        facing = goal % 4;
        complete.update(.18, {{action(Directions[static_cast<std::size_t>(facing)])}});
        require(complete.snapshot().status == Status::Running,
                "Solarfox public-control traversal lost");
    }
    require(SolarfoxAccess::level(complete) == 2,
            "Solarfox public controls did not clear every powerup");
    require(complete.snapshot().score >= 1900, "Solarfox complete-stage collection score missing");
}

} // namespace arcade::test
