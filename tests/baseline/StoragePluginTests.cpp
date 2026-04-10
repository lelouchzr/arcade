#include "Minesweeper.hpp"
#include "Snake.hpp"
#include "Support.hpp"
#include "arcade/Font.hpp"
#include "arcade/Plugin.hpp"
#include "arcade/Scores.hpp"
#include <algorithm>
#include <cmath>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <set>
#include <stdexcept>
#include <unistd.h>

namespace arcade::baseline {
void scoreTests() {
    const auto path =
        std::filesystem::temp_directory_path() / ("arcade-scores-" + std::to_string(getpid()));
    {
        std::ofstream file(path);
        file << "garbage\n\"snake\" \"bad\" -5\n\"snake\" \"Ada\" 120\n";
    }
    Scores scores(path);
    require(scores.top("snake").size() == 1, "score corruption not ignored");
    scores.add("snake", "Grace", 250);
    scores.add("minesweeper", "Alan", 100);
    Scores reloaded(path);
    require(reloaded.top("snake")[0].player == "Grace", "score atomic persistence failed");
    require(reloaded.top("minesweeper").size() == 1, "per-game scores mixed");
    std::filesystem::remove(path);
}

void pluginTests() {
    for (int i = 0; i < 100; ++i) {
        Plugin plugin("lib/arcade_snake.so");
        require(plugin.info().kind == PluginKind::Game, "plugin metadata invalid");
        auto &game = plugin.instance<IGame>();
        game.reset(static_cast<std::uint64_t>(i));
        game.update(.14, {});
        require(count(game.snapshot(), Cell::Head) == 1, "plugin instance invalid");
    }
#if defined(__linux__) && defined(RTLD_NOLOAD)
    void *retained = dlopen("lib/arcade_snake.so", RTLD_NOW | RTLD_NOLOAD);
    if (retained)
        dlclose(retained);
    require(!retained, "dlclose retained the game module after its final owner disappeared");
#endif
    bool missing = false;
    try {
        Plugin plugin("lib/does-not-exist.so");
    } catch (const std::runtime_error &) {
        missing = true;
    }
    require(missing, "missing library accepted");
}
} // namespace arcade::baseline
