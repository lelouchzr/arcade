#include "core/Core.hpp"
#include <iostream>
#include <string_view>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "USAGE: ./arcade ./lib/arcade_ncurses.so\n";
        return 84;
    }
    if (std::string_view(argv[1]) == "--help" || std::string_view(argv[1]) == "-h") {
        std::cout
            << "USAGE: ./arcade <GRAPHICS_PLUGIN>\n"
               "Menu: arrows select game/display, type your name, Enter plays.\n"
               "F2 display | F3 game | F4 restart | F5 menu | F6 pause | Esc exit\n"
               "Arrows move; Space acts/shoots; F flags, stops Qix or boosts Solar Fox.\n"
               "D/F7 selects game bonuses or Minesweeper difficulty.\n"
               "Snake/Nibbler: Space boosts. Minesweeper: ? marks; Custom uses arrows/digits.\n"
               "Pacman: Space/Z jumps, V toggles camera. Centipede: IJKL/H controls player 2.\n"
               "Seven games with bonus enabled.\n";
        return 0;
    }
    try {
        arcade::Core core(argv[1]);
        core.run();
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 84;
    }
}
