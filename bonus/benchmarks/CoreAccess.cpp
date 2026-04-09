#include "CoreAccess.hpp"
#include <algorithm>
#include <stdexcept>

namespace arcade::benchmark {

void CoreAccess::prepare(Core &core, const std::string &game, std::uint64_t seed) {
    core.player_ = "BENCH";
    if (game.empty()) {
        core.menu_ = true;
        return;
    }
    const auto found = std::find_if(core.games_.begin(), core.games_.end(),
                                    [&](const auto &info) { return info.id == game; });
    if (found == core.games_.end())
        throw std::runtime_error("benchmark game plugin missing: " + game);
    core.startGame(static_cast<std::size_t>(found - core.games_.begin()));
    core.game_->instance<IGame>().reset(seed);
}

} // namespace arcade::benchmark

namespace arcade::benchmark {

void CoreAccess::variant(Core &core, int index) {
    const std::vector<std::string> games{"",
                                         "",
                                         "",
                                         "pacman",
                                         "pacman",
                                         "pacman",
                                         "snake",
                                         "minesweeper",
                                         "minesweeper",
                                         "solarfox",
                                         "nibbler",
                                         "qix",
                                         "centipede"};
    prepare(core, games.at(static_cast<std::size_t>(index)), 42);
    core.player_ = "Bench_42";
    if (index == 2)
        core.selectedGame_ = core.games_.size() - 1;
    if (index == 4)
        core.game_->instance<IGame>().update(9.5, {{Action::Right}});
    if (index == 5)
        core.paused_ = true;
    if (index == 6)
        core.game_->instance<IGame>().update(3, {});
    if (index == 8)
        core.game_->instance<IGame>().update(0, {{Action::Primary}});
}

} // namespace arcade::benchmark
