#include "CoreAccess.hpp"
#include "Digest.hpp"
#include "Headless.hpp"
#include "Workspace.hpp"
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
    if (argc != 3)
        return 84;
    const auto modules = std::filesystem::canonical(argv[1]);
    const auto graphics = std::filesystem::canonical(argv[2]);
    try {
        const std::array<const char *, 13> names{"menu_populated",
                                                 "menu_empty",
                                                 "menu_scroll",
                                                 "pacman_initial",
                                                 "pacman_progressed",
                                                 "pacman_paused",
                                                 "snake_lost",
                                                 "minesweeper_hidden",
                                                 "minesweeper_revealed",
                                                 "solarfox",
                                                 "nibbler",
                                                 "qix",
                                                 "centipede"};
        for (std::size_t index = 0; index < names.size(); ++index) {
            arcade::benchmark::Workspace workspace(modules, graphics);
            arcade::Core core("lib/arcade_bench_headless.so");
            arcade::benchmark::CoreAccess::variant(core, static_cast<int>(index));
            if (index == 1) {
                std::ofstream empty(std::getenv("ARCADE_SCORE_FILE"), std::ios::trunc);
            }
            const auto frame = core.frame();
            arcade::benchmark::Digest digest;
            digest.add(frame);
            arcade::benchmark::Headless renderer;
            renderer.draw(frame);
            std::cout << "{\"variant\":\"" << names[index] << "\",\"checksum\":" << digest.value()
                      << ",\"commands\":" << renderer.commands()
                      << ",\"render_checksum\":" << renderer.checksum() << "}\n";
        }
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 84;
    }
}
