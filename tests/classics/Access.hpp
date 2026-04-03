#pragma once
#include "../../bonus/games/nibbler/Nibbler.hpp"
#include "../../src/games/minesweeper/Minesweeper.hpp"
#include "../../src/games/snake/Snake.hpp"

namespace arcade::test {
struct SnakeBonusAccess {
    static void pickup(Snake &game, Point point);
    static void longBody(Snake &game);
    static double interval(const Snake &game);
};

struct MinesBonusAccess {
    static void finish(Minesweeper &game, Status status);
};
} // namespace arcade::test

namespace arcade::bonus::test {
struct NibblerBonusAccess {
    static void pickup(Nibbler &game, Point point);
    static int food(const Nibbler &game);
    static void originalLevel(Nibbler &game, int level);
    static void lastFood(Nibbler &game);
    static void junction(Nibbler &game);
    static void isolatedFloor(Nibbler &game);
    static int level(const Nibbler &game);
};
} // namespace arcade::bonus::test
