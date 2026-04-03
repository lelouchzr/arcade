#include "Cases.hpp"
#include <algorithm>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace arcade::test {
void requireClassic(bool condition, const std::string &message) {
    if (!condition)
        throw std::runtime_error(message);
}

void requireHud(const Snapshot &view) {
    requireClassic(view.information.size() <= 7, "game HUD exceeds seven visible lines");
    for (const auto &line : view.information)
        requireClassic(line.size() <= 23, "game HUD line is clipped: " + line);
}

std::size_t countClassic(const Snapshot &view, Cell cell) {
    return static_cast<std::size_t>(std::count(view.cells.begin(), view.cells.end(), cell));
}
} // namespace arcade::test

int main() {
    try {
        arcade::test::pickupTests();
        arcade::test::snakeBonusTests();
        arcade::test::minesBonusTests();
        arcade::test::nibblerBonusTests();
        std::cout << "Classic game bonuses passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
