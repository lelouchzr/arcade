#include "BonusFeatures.hpp"
#include "TestSupport.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace arcade::test {

void QixBonusAccess::pickup(bonus::Qix &game, Point point, bonus::QixPower power, double lifetime) {
    game.pickups_ = {{point, power, lifetime}};
}

void QixBonusAccess::trailPickup(bonus::Qix &game) {
    game.trail_ = {{1, 10}, {2, 10}, {3, 10}};
    for (const auto point : game.trail_)
        game.board_.at(point) = Cell::Trail;
    game.player_ = {3, 10};
    game.pickups_ = {{{1, 10}, bonus::QixPower::Slow, 8}};
}

void QixBonusAccess::monsterAt(bonus::Qix &game, Point point) {
    game.monster_ = {point, {point.x, point.y + 1}, {point.x, point.y + 2}};
}

double QixBonusAccess::shield(const bonus::Qix &game) {
    return game.shield_;
}

double QixBonusAccess::slow(const bonus::Qix &game) {
    return game.slow_;
}

std::size_t QixBonusAccess::pickups(const bonus::Qix &game) {
    return game.pickups_.size();
}

const bonus::Grid &QixBonusAccess::board(const bonus::Qix &game) {
    return game.board_;
}

void qixBonusTests() {
    using namespace bonus;
    const auto bonusHud = Qix(QixRules{.08, .14, .18, .6, .07, true}).snapshot();
    require(bonusHud.information.size() <= 7, "Qix bonus HUD exceeds seven lines");
    for (const auto &line : bonusHud.information)
        require(line.size() <= 23, "Qix bonus HUD contains a truncated control line");
    const QixRules rules{.08, 10000, 10000, .6, .07, true};
    Qix game(rules);
    require(count(game.snapshot(), Cell::Obstacle) == 4, "Qix bonus scenery missing");
    require(count(game.snapshot(), Cell::Powerup) == 1 &&
                count(game.snapshot(), Cell::BonusFood) == 1,
            "Qix bonus pickups missing");
    game.update(.24, {{Action::Right}});
    require(game.snapshot().score == 250 && QixBonusAccess::shield(game) > 4.9,
            "Qix player touching pickup did not grant shield and score");
    QixBonusAccess::monsterAt(game, {4, 10});
    game.update(.08, {{Action::Right}});
    require(game.snapshot().status == Status::Running,
            "Qix shield did not protect player and trail from monster contact");
    game.update(1, {{Action::Stop}});
    require(game.snapshot().status == Status::Running && count(game.snapshot(), Cell::Fire) == 0,
            "Qix shield did not protect stopped trail from fire");
    game.update(5, {});
    require(QixBonusAccess::shield(game) == 0, "Qix shield did not expire");
    game.reset(17);
    QixBonusAccess::trailPickup(game);
    game.update(.01, {});
    require(game.snapshot().score == 250 && QixBonusAccess::slow(game) > 4.9 &&
                QixBonusAccess::pickups(game) == 0,
            "Qix trail did not collect a powerup behind the player");
    Qix slowed(QixRules{.08, .14, 10000, .6, .07, true});
    QixBonusAccess::pickup(slowed, {0, 10}, QixPower::Slow);
    slowed.update(.01, {});
    const auto slowStart = slowed.snapshot().cells;
    slowed.update(.14, {});
    require(slowed.snapshot().cells == slowStart, "Qix slow effect did not delay enemy movement");
    Qix normal(QixRules{.08, .14, 10000, .6, .07, false});
    const auto normalStart = normal.snapshot().cells;
    normal.update(.15, {});
    require(normal.snapshot().cells != normalStart, "Qix control monster did not move");
    game.reset(17);
    QixBonusAccess::pickup(game, {4, 4}, QixPower::Shield, .02);
    game.update(.03, {});
    require(QixBonusAccess::pickups(game) == 0 && game.snapshot().score == 0,
            "Qix uncollected pickup did not expire");
    game.reset(17);
    game.update(.4, {{Action::Up}});
    game.update(.48, {{Action::Right}});
    const auto beforeObstacle = locate(game.snapshot(), Cell::Player);
    game.update(.08, {{Action::Down}});
    require(locate(game.snapshot(), Cell::Player) == beforeObstacle,
            "Qix player walked through scenery");
    game.reset(17);
    game.update(.4, {{Action::Up}});
    game.update(1.92, {{Action::Right}});
    require(count(game.snapshot(), Cell::Obstacle) == 4 && game.snapshot().score > 0,
            "Qix capture erased scenery or failed to claim around it");
    Qix deterministic(rules), partitioned(rules);
    deterministic.reset(27);
    partitioned.reset(27);
    deterministic.update(.4, {{Action::Up}});
    partitioned.update(.1, {{Action::Up}});
    partitioned.update(.3, {});
    require(deterministic.snapshot().cells == partitioned.snapshot().cells,
            "Qix bonus simulation depends on update partition");
    game.update(0, {{Action::Difficulty}});
    require(count(game.snapshot(), Cell::Obstacle) == 0 && QixBonusAccess::pickups(game) == 0,
            "Qix classic mode retained bonus objects");
    game.update(0, {{Action::Difficulty}});
    require(count(game.snapshot(), Cell::Obstacle) == 4, "Qix bonus mode cannot be re-enabled");
    bool rejected = false;
    try {
        game.update(std::numeric_limits<double>::quiet_NaN(), {});
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    require(rejected, "Qix accepted invalid delta in bonus mode");
    require(game.snapshot().information.size() <= 7, "Qix bonus HUD exceeds seven lines");
    for (const auto &line : game.snapshot().information)
        require(line.size() <= 23, "Qix HUD line exceeds visible width");
}

} // namespace arcade::test
