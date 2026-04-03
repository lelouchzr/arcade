#include "arcade/CellStyle.hpp"
#include "arcade/Font.hpp"
#include "baseline/Support.hpp"
#include <exception>
#include <iostream>

int main() {
    using namespace arcade::baseline;
    try {
        snakeTests();
        mineTests();
        scoreTests();
        pluginTests();
#ifdef ARCADE_BONUS
        nibblerTests();
#endif
        require(arcade::glyph('A')[0] == 14, "pixel font A invalid");
        for (int cell = 0; cell <= static_cast<int>(arcade::Cell::FastProjectile); ++cell) {
            const char character = arcade::cellStyle(static_cast<arcade::Cell>(cell)).glyph;
            if (character != 0 && cell != static_cast<int>(arcade::Cell::Question))
                require(arcade::glyph(character) != arcade::glyph('?'),
                        "cell style has no pixel glyph");
        }
        std::cout << checks << " checks passed\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "FAILED after " << checks << " checks: " << e.what() << '\n';
        return 1;
    }
}
