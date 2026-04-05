#pragma once

#include "../../bonus/games/pacman/Pacman.hpp"

namespace arcade::test {

struct PacmanBonusAccess {
    static void park(bonus::Pacman &game, Point point = {2, 1}, Point heading = {0, -1});
    static void ghost(bonus::Pacman &game, std::size_t index, Point point, double jump = -1);
    static double playerHeight(const bonus::Pacman &game);
    static double ghostHeight(const bonus::Pacman &game, std::size_t index);
    static double acceleration(const bonus::Pacman &game);
};

void pacmanBonusTests();

} // namespace arcade::test
