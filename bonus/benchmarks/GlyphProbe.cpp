#include "arcade/Font.hpp"
#include <clocale>
#include <iostream>

int main(int argc, char **argv) {
    if (argc > 2)
        return 84;
    if (!std::setlocale(LC_CTYPE, argc == 2 ? argv[1] : "C"))
        return 84;
    for (unsigned int character = 0; character < 256; ++character) {
        std::cout << character;
        for (auto row : arcade::glyph(static_cast<char>(character)))
            std::cout << ' ' << static_cast<unsigned int>(row);
        std::cout << '\n';
    }
    return 0;
}
