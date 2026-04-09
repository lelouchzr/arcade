#include "Workspace.hpp"
#include "arcade/Scores.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <stdexcept>
#include <unistd.h>

namespace arcade::benchmark {

Workspace::Workspace(const std::filesystem::path &modules, const std::filesystem::path &graphics)
    : previous_(std::filesystem::current_path()) {
    std::array<char, 64> name{};
    const std::string pattern = "/tmp/arcade-benchmark-XXXXXX";
    std::copy(pattern.begin(), pattern.end(), name.begin());
    const char *temporary = ::mkdtemp(name.data());
    if (!temporary)
        throw std::runtime_error("cannot create benchmark workspace");
    root_ = std::filesystem::canonical(temporary);
    try {
        std::filesystem::create_directory(root_ / "lib");
        for (const auto &game :
             {"snake", "minesweeper", "nibbler", "pacman", "qix", "centipede", "solarfox"}) {
            const auto filename = std::string("arcade_") + game + ".so";
            const auto source = std::filesystem::canonical(modules / filename);
            std::filesystem::create_symlink(source, root_ / "lib" / filename);
        }
        std::filesystem::create_symlink(std::filesystem::canonical(graphics),
                                        root_ / "lib/arcade_bench_headless.so");
        const auto scores = root_ / "scores.tsv";
        setenv("ARCADE_SCORE_FILE", scores.c_str(), 1);
        unsetenv("ARCADE_LEGACY_GAME");
        unsetenv("ARCADE_LEGACY_GRAPHICS");
        Scores fixture(scores);
        for (const auto &game :
             {"snake", "minesweeper", "nibbler", "pacman", "qix", "centipede", "solarfox"})
            for (int rank = 0; rank < 10; ++rank)
                fixture.add(game, "PLAYER" + std::to_string(rank), 1000 - rank * 10);
        std::filesystem::current_path(root_);
    } catch (...) {
        std::filesystem::remove_all(root_);
        throw;
    }
}

Workspace::~Workspace() {
    std::error_code ignored;
    std::filesystem::current_path(previous_, ignored);
    std::filesystem::remove_all(root_, ignored);
}

const std::filesystem::path &Workspace::root() const {
    return root_;
}

} // namespace arcade::benchmark
