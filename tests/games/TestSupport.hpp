#pragma once

#include "arcade/Api.hpp"
#include <string>

namespace arcade::test {

extern int checks;
void require(bool condition, const std::string &message);
int count(const Snapshot &view, Cell cell);
Point locate(const Snapshot &view, Cell cell);
Action action(Point direction);
void pacmanTests();
void qixTests();
void centipedeTests();
void solarfoxTests();

} // namespace arcade::test
