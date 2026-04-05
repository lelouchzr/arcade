#pragma once

#include "../../bonus/games/centipede/Centipede.hpp"
#include "../../bonus/games/qix/Qix.hpp"
#include "../../bonus/games/solarfox/Solarfox.hpp"

namespace arcade::test {

struct QixBonusAccess {
    static void pickup(bonus::Qix &game, Point point, bonus::QixPower power, double lifetime = 8);
    static void trailPickup(bonus::Qix &game);
    static void monsterAt(bonus::Qix &game, Point point);
    static double shield(const bonus::Qix &game);
    static double slow(const bonus::Qix &game);
    static std::size_t pickups(const bonus::Qix &game);
    static const bonus::Grid &board(const bonus::Qix &game);
};

struct CentipedeBonusAccess {
    static const std::vector<bonus::CentipedeBody> &bodies(const bonus::Centipede &game);
    static Point second(const bonus::Centipede &game);
    static std::optional<Point> shot(const bonus::Centipede &game);
    static void field(bonus::Centipede &game, std::vector<bonus::CentipedeBody> bodies);
    static void projectile(bonus::Centipede &game, Point point);
};

struct SolarfoxBonusAccess {
    static const std::vector<bonus::EnemyGun> &guns(const bonus::Solarfox &game);
    static const std::vector<bonus::SolarShot> &enemyShots(const bonus::Solarfox &game);
    static void speeds(bonus::Solarfox &game);
    static void spread(bonus::Solarfox &game);
    static void player(bonus::Solarfox &game, Point point);
    static void obstacleShot(bonus::Solarfox &game, bool enemy);
    static std::size_t playerShots(const bonus::Solarfox &game);
    static const bonus::Grid &board(const bonus::Solarfox &game);
    static int level(const bonus::Solarfox &game);
};

void qixBonusTests();
void centipedeBonusTests();
void solarfoxBonusTests();

} // namespace arcade::test
