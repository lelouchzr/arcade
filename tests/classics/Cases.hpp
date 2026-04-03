#pragma once
#include "arcade/Api.hpp"
#include <string>

namespace arcade::test {
void requireClassic(bool condition, const std::string &message);
[[nodiscard]] std::size_t countClassic(const Snapshot &view, Cell cell);
void requireHud(const Snapshot &view);
void pickupTests();
void snakeBonusTests();
void minesBonusTests();
void nibblerBonusTests();
} // namespace arcade::test
