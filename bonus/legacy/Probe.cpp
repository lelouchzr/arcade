#include "LegacyGame.hpp"
#include <exception>
#include <iostream>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "USAGE: arcade_legacy_probe <HISTORICAL_GAME.so>\n";
        return 84;
    }
    try {
        arcade::legacy::LegacyGame game(argv[1]);
        const auto before = game.snapshot();
        game.update(.1, {});
        const auto after = game.snapshot();
        std::cout << before.information[1] << " | " << before.width << 'x' << before.height
                  << " | score " << after.score << " | moved " << (before.cells != after.cells)
                  << '\n';
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 84;
    }
}
