#include "TestSupport.hpp"
#include <exception>
#include <iostream>

int main() {
    try {
        arcade::test::pacmanTests();
        arcade::test::qixTests();
        arcade::test::centipedeTests();
        arcade::test::solarfoxTests();
        arcade::test::pacmanBonusTests();
        arcade::test::qixBonusTests();
        arcade::test::centipedeBonusTests();
        arcade::test::solarfoxBonusTests();
        arcade::test::originalLevelsTests();
        std::cout << arcade::test::checks << " bonus gameplay checks passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "FAILED after " << arcade::test::checks << " checks: " << error.what() << '\n';
        return 1;
    }
}
