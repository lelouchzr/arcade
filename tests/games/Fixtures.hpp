#pragma once

#include "../../bonus/games/centipede/Centipede.hpp"
#include "../../bonus/games/pacman/Pacman.hpp"
#include "../../bonus/games/qix/Qix.hpp"
#include "../../bonus/games/solarfox/Solarfox.hpp"

namespace arcade::test {

// These accessors only arrange exceptional board positions. Tests always advance public update().
struct PacmanAccess {
    static const bonus::Grid &board(const bonus::Pacman &game);
    static const std::array<bonus::Ghost, 4> &ghosts(const bonus::Pacman &game);
    static void player(bonus::Pacman &game, Point point, Point heading);
    static void ghost(bonus::Pacman &game, std::size_t index, Point point, bonus::GhostMode mode);
    static double power(const bonus::Pacman &game);
    static int level(const bonus::Pacman &game);
};

struct QixAccess {
    static void trail(bonus::Qix &game, const std::vector<Point> &trail, Point anchor);
    static void monster(bonus::Qix &game, const std::deque<Point> &points);
    static void sparkTrack(bonus::Qix &game, const std::vector<Point> &track, bonus::Spark spark,
                           Point player);
    static const bonus::Grid &board(const bonus::Qix &game);
    static const std::vector<bonus::Spark> &sparks(const bonus::Qix &game);
};

struct CentipedeAccess {
    static void scenario(bonus::Centipede &game, std::vector<bonus::CentipedeBody> bodies,
                         Point player);
    static void obstacle(bonus::Centipede &game, Point point, int health);
    static int health(const bonus::Centipede &game, Point point);
    static void shot(bonus::Centipede &game, Point point);
    static const std::vector<bonus::CentipedeBody> &bodies(const bonus::Centipede &game);
    static void score(bonus::Centipede &game, std::int64_t value);
    static void survived(bonus::Centipede &game, int value);
    static int level(const bonus::Centipede &game);
};

struct SolarfoxAccess {
    static void player(bonus::Solarfox &game, Point point, Point heading);
    static void clearField(bonus::Solarfox &game);
    static void powerup(bonus::Solarfox &game, Point point, bool bad = false);
    static void enemyShot(bonus::Solarfox &game, Point point, Point heading);
    static const std::vector<bonus::SolarShot> &shots(const bonus::Solarfox &game);
    static const std::vector<bonus::SolarShot> &enemyShots(const bonus::Solarfox &game);
    static int level(const bonus::Solarfox &game);
    static int remaining(const bonus::Solarfox &game);
    static const bonus::Grid &board(const bonus::Solarfox &game);
    static void simultaneousShots(bonus::Solarfox &game);
};

} // namespace arcade::test
