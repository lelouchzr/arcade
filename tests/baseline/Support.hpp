#pragma once
#include "Minesweeper.hpp"
#include "arcade/Api.hpp"

namespace arcade::baseline {
extern int checks;
void require(bool condition, const char *message);
int count(const Snapshot &state, Cell cell);
Point locate(const Snapshot &state, Cell cell);
void moveCursor(Minesweeper &game, Point target);
void snakeTests();
void mineTests();
void nibblerTests();
void scoreTests();
void pluginTests();
} // namespace arcade::baseline
