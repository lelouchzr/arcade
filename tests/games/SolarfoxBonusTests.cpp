#include "BonusFeatures.hpp"
#include "TestSupport.hpp"
#include <algorithm>
#include <limits>
#include <queue>
#include <stdexcept>

namespace arcade::test {

const std::vector<bonus::EnemyGun> &SolarfoxBonusAccess::guns(const bonus::Solarfox &game) {
    return game.guns_;
}

const std::vector<bonus::SolarShot> &SolarfoxBonusAccess::enemyShots(const bonus::Solarfox &game) {
    return game.enemyShots_;
}

void SolarfoxBonusAccess::speeds(bonus::Solarfox &game) {
    game.guns_.clear();
    game.enemyShots_ = {{{5, 5}, {1, 0}, 40, bonus::SolarShotType::Slow, 0},
                        {{5, 7}, {1, 0}, 40, bonus::SolarShotType::Fast, 0}};
}

void SolarfoxBonusAccess::spread(bonus::Solarfox &game) {
    game.guns_ = {{{8, 2}, {0, 1}, game.rules_.gunPeriod - .005, bonus::SolarShotType::Spread, 1}};
    game.enemyShots_.clear();
}

void SolarfoxBonusAccess::player(bonus::Solarfox &game, Point point) {
    game.player_ = point;
    game.heading_ = game.desired_ = {1, 0};
    game.turnQueued_ = false;
    game.movementClock_ = game.shotClock_ = game.enemyShotClock_ = game.accumulated_ = 0;
    game.guns_.clear();
    game.shots_.clear();
    game.enemyShots_.clear();
}

void SolarfoxBonusAccess::obstacleShot(bonus::Solarfox &game, bool enemy) {
    player(game, {12, 10});
    if (enemy)
        game.enemyShots_ = {{{8, 8}, {1, 0}, 40, bonus::SolarShotType::Fast, 0}};
    else
        game.shots_ = {{{8, 8}, {1, 0}, 2}};
}

std::size_t SolarfoxBonusAccess::playerShots(const bonus::Solarfox &game) {
    return game.shots_.size();
}

const bonus::Grid &SolarfoxBonusAccess::board(const bonus::Solarfox &game) {
    return game.board_;
}

int SolarfoxBonusAccess::level(const bonus::Solarfox &game) {
    return game.level_;
}

namespace {

void collectBonusStage() {
    using namespace bonus;
    Solarfox game(SolarfoxRules{.18, .09, 10000, true});
    int facing = 0;
    for (int turn = 0; turn < 2000 && SolarfoxBonusAccess::level(game) == 1; ++turn) {
        const auto &map = SolarfoxBonusAccess::board(game);
        const auto position = locate(game.snapshot(), Cell::Ship);
        const auto safe = [&](Point point) {
            return map.contains(point) && map.at(point) != Cell::Wall &&
                   map.at(point) != Cell::EnemyGun && map.at(point) != Cell::Obstacle &&
                   map.at(point) != Cell::BadPowerup;
        };
        const auto target = [&](Point point, int heading) {
            for (int range = 0; range < 2; ++range) {
                point = add(point, Directions[static_cast<std::size_t>(heading)]);
                if (!safe(point))
                    return false;
                if (map.at(point) == Cell::Powerup)
                    return true;
            }
            return false;
        };
        if (target(position, facing)) {
            game.update(.18, {{Action::Primary}});
            continue;
        }
        const int start = static_cast<int>(map.index(position)) * 4 + facing;
        std::vector<int> parents(map.cells().size() * 4, -1);
        std::queue<int> pending;
        pending.push(start);
        parents[static_cast<std::size_t>(start)] = start;
        int goal = -1;
        while (!pending.empty() && goal < 0) {
            const auto state = pending.front();
            pending.pop();
            const auto point = map.point(static_cast<std::size_t>(state / 4));
            for (int heading = 0; heading < 4; ++heading) {
                if (heading == (state % 4 + 2) % 4)
                    continue;
                const auto next = add(point, Directions[static_cast<std::size_t>(heading)]);
                if (!safe(next))
                    continue;
                const int key = static_cast<int>(map.index(next)) * 4 + heading;
                if (parents[static_cast<std::size_t>(key)] >= 0)
                    continue;
                parents[static_cast<std::size_t>(key)] = state;
                if (target(next, heading)) {
                    goal = key;
                    break;
                }
                pending.push(key);
            }
        }
        require(goal >= 0, "Solarfox bonus stage contains a powerup without a legal firing route");
        while (parents[static_cast<std::size_t>(goal)] != start)
            goal = parents[static_cast<std::size_t>(goal)];
        facing = goal % 4;
        game.update(.18, {{action(Directions[static_cast<std::size_t>(facing)])}});
        require(game.snapshot().status == Status::Running,
                "Solarfox bonus traversal hit scenery on a legal route");
    }
    require(SolarfoxBonusAccess::level(game) == 2,
            "Solarfox bonus public controls failed to clear every powerup");
    require(game.snapshot().score >= 1900, "Solarfox bonus stage completion lost collection score");
}

} // namespace

void solarfoxBonusTests() {
    using namespace bonus;
    const auto bonusHud = Solarfox(SolarfoxRules{.18, .09, 1.1, true}).snapshot();
    require(bonusHud.information.size() <= 7, "Solarfox bonus HUD exceeds seven lines");
    for (const auto &line : bonusHud.information)
        require(line.size() <= 23, "Solarfox bonus HUD contains a truncated control line");
    const SolarfoxRules rules{.18, .09, 1.1, true};
    Solarfox game(rules);
    require(count(game.snapshot(), Cell::Obstacle) == 4, "Solarfox bonus obstacles missing");
    const auto initialGuns = SolarfoxBonusAccess::guns(game);
    game.update(.24, {});
    require(SolarfoxBonusAccess::guns(game)[0].position != initialGuns[0].position &&
                count(game.snapshot(), Cell::EnemyGun) == 4,
            "Solarfox guns did not patrol their border");
    Solarfox patrol(SolarfoxRules{10000, .09, 10000, true});
    for (int interval = 0; interval < 100; ++interval) {
        patrol.update(.24, {});
        for (const auto &gun : SolarfoxBonusAccess::guns(patrol)) {
            const bool side = (gun.position.x == 2 || gun.position.x == 22) &&
                              gun.position.y >= 3 && gun.position.y <= 17;
            const bool topBottom = (gun.position.y == 2 || gun.position.y == 18) &&
                                   gun.position.x >= 3 && gun.position.x <= 21;
            require(side || topBottom, "Solarfox gun left its border while reversing patrol");
        }
    }
    game.reset(1);
    SolarfoxBonusAccess::speeds(game);
    game.update(.18, {});
    const auto &shots = SolarfoxBonusAccess::enemyShots(game);
    require(shots.size() == 2 && shots[0].position == Point{6, 5} && shots[1].position.x >= 8,
            "Solarfox slow and fast projectiles do not have distinct speeds");
    require(count(game.snapshot(), Cell::SlowProjectile) == 1 &&
                count(game.snapshot(), Cell::FastProjectile) == 1,
            "Solarfox projectile types are not visibly distinct");
    game.reset(1);
    SolarfoxBonusAccess::spread(game);
    game.update(.005, {});
    require(SolarfoxBonusAccess::enemyShots(game).size() == 3 &&
                SolarfoxBonusAccess::enemyShots(game)[1].direction !=
                    SolarfoxBonusAccess::enemyShots(game)[2].direction,
            "Solarfox spread gun did not emit distinct directions");
    game.update(.09, {});
    require(SolarfoxBonusAccess::enemyShots(game)[0].position == Point{8, 3} &&
                SolarfoxBonusAccess::enemyShots(game)[1].position !=
                    SolarfoxBonusAccess::enemyShots(game)[2].position,
            "Solarfox spread projectiles do not separate in flight");
    game.reset(1);
    SolarfoxBonusAccess::player(game, {8, 8});
    game.update(.18, {});
    require(game.snapshot().status == Status::Lost, "Solarfox ship passed through an obstacle");
    game.reset(1);
    SolarfoxBonusAccess::obstacleShot(game, false);
    game.update(.045, {});
    require(SolarfoxBonusAccess::playerShots(game) == 0 &&
                count(game.snapshot(), Cell::Obstacle) == 4,
            "Solarfox obstacle failed to absorb player projectile");
    SolarfoxBonusAccess::obstacleShot(game, true);
    game.update(.05, {});
    require(SolarfoxBonusAccess::enemyShots(game).empty(),
            "Solarfox enemy projectile passed through an obstacle");
    const auto view = game.snapshot();
    std::vector<bool> reachable(view.cells.size(), false);
    std::queue<Point> pending;
    pending.push({12, 10});
    reachable[static_cast<std::size_t>(10 * view.width + 12)] = true;
    while (!pending.empty()) {
        const auto point = pending.front();
        pending.pop();
        for (const auto delta : Directions) {
            const auto next = add(point, delta);
            if (next.x < 3 || next.x > 21 || next.y < 3 || next.y > 17)
                continue;
            const auto index = static_cast<std::size_t>(next.y * view.width + next.x);
            if (reachable[index] || view.cells[index] == Cell::Obstacle ||
                view.cells[index] == Cell::BadPowerup)
                continue;
            reachable[index] = true;
            pending.push(next);
        }
    }
    for (std::size_t index = 0; index < view.cells.size(); ++index)
        if (view.cells[index] == Cell::Powerup)
            require(reachable[index], "Solarfox scenery sealed an unreachable powerup");
    collectBonusStage();
    Solarfox deterministic(rules), partitioned(rules);
    SolarfoxBonusAccess::speeds(deterministic);
    SolarfoxBonusAccess::speeds(partitioned);
    deterministic.update(.3, {});
    partitioned.update(.1, {});
    partitioned.update(.2, {});
    require(deterministic.snapshot().cells == partitioned.snapshot().cells,
            "Solarfox bonus projectiles or patrols depend on update partition");
    game.update(0, {{Action::Difficulty}});
    require(count(game.snapshot(), Cell::Obstacle) == 0,
            "Solarfox classic mode retained bonus scenery");
    const auto classicGuns = SolarfoxBonusAccess::guns(game);
    game.update(.24, {});
    require(SolarfoxBonusAccess::guns(game)[0].position == classicGuns[0].position,
            "classic Solarfox guns unexpectedly patrol");
    bool rejected = false;
    try {
        game.update(std::numeric_limits<double>::infinity(), {});
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    require(rejected, "Solarfox accepted non-finite delta in bonus mode");
    require(game.snapshot().information.size() <= 7, "Solarfox bonus HUD exceeds seven lines");
    for (const auto &line : game.snapshot().information)
        require(line.size() <= 23, "Solarfox HUD line exceeds visible width");
}

} // namespace arcade::test
