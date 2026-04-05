#include "Pacman.hpp"
#include <cmath>

namespace arcade::bonus {

Snapshot Pacman::snapshot() const {
    Snapshot view;
    view.width = Width;
    view.height = Height;
    view.cells = board_.cells();
    if (rules_.bonusEnabled && fruitActive_)
        view.cells[board_.index({12, 12})] = Cell::BonusFood;
    for (std::size_t i = 0; i < ghosts_.size(); ++i) {
        const auto &ghost = ghosts_[i];
        Cell cell = static_cast<Cell>(static_cast<int>(Cell::GhostRed) + static_cast<int>(i));
        if (ghost.mode == GhostMode::Eyes)
            cell = Cell::GhostEyes;
        else if (ghost.mode == GhostMode::Active && power_ > 0)
            cell = Cell::FrightenedGhost;
        view.cells[board_.index(ghost.position)] = cell;
        if (rules_.bonusEnabled && elevation(ghost.jump) > 0)
            view.sprites.push_back({ghost.position, Cell::JumpingGhost, elevation(ghost.jump)});
        else if (rules_.bonusEnabled && ghost.position == player_ && elevation(jump_) > 0)
            // Keep the ground ghost visible beneath the semantic player cell and its jump.
            view.sprites.push_back({ghost.position, cell, 0});
    }
    view.cells[board_.index(player_)] = Cell::Player;
    if (rules_.bonusEnabled && elevation(jump_) > 0)
        view.sprites.push_back({player_, Cell::JumpingPlayer, elevation(jump_)});
    if (rules_.bonusEnabled && cameraEnabled_)
        view.camera = Camera{player_, 17, 13, true};
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
                        "D / F7   PACMANIA MODE",
                        "TUNNELS WRAP THE BOARD"};
    if (rules_.bonusEnabled) {
        view.information = {
            "ARROWS TURN - Z JUMP",
            "D CLASSIC - V CAMERA",
            "LEVEL " + std::to_string(level_) + " PELLETS " + std::to_string(pellets_),
            "POWER " + std::to_string(static_cast<int>(std::ceil(power_))) + " JUMP CD " +
                std::to_string(static_cast<int>(std::ceil(jump_.cooldown))),
            std::string(fruitActive_ ? "FRUIT EXPIRES " : "FRUIT ARRIVES ") +
                std::to_string(static_cast<int>(std::ceil(fruitClock_))) + "S",
            "FRUIT +" + std::to_string(1000LL * level_) + " AT START",
            "SPEED X" + std::to_string(acceleration()).substr(0, 4)};
    }
    return view;
}

} // namespace arcade::bonus
