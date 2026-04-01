#include "Pacman.hpp"
#include <cmath>

namespace arcade::bonus {

Snapshot Pacman::snapshot() const {
    Snapshot view;
    view.width = Width;
    view.height = Height;
    view.cells = board_.cells();
    for (std::size_t i = 0; i < ghosts_.size(); ++i) {
        const auto &ghost = ghosts_[i];
        Cell cell = static_cast<Cell>(static_cast<int>(Cell::GhostRed) + static_cast<int>(i));
        if (ghost.mode == GhostMode::Eyes)
            cell = Cell::GhostEyes;
        else if (ghost.mode == GhostMode::Active && power_ > 0)
            cell = Cell::FrightenedGhost;
        view.cells[board_.index(ghost.position)] = cell;
    }
    view.cells[board_.index(player_)] = Cell::Player;
    view.score = score_;
    view.generation = generation_;
    view.status = status_;
    view.information = {"ARROWS   TURN",
                        "PELLETS  " + std::to_string(pellets_),
                        "LEVEL    " + std::to_string(level_),
                        "POWER    " + std::to_string(static_cast<int>(std::ceil(power_))),
                        elapsed_ < rules_.releaseDelay
                            ? "GHOSTS RELEASE IN " + std::to_string(static_cast<int>(
                                                         std::ceil(rules_.releaseDelay - elapsed_)))
                            : "GHOSTS ARE HUNTING",
                        "FOUR POWER PELLETS",
                        "TUNNELS WRAP THE BOARD"};
    return view;
}

} // namespace arcade::bonus
