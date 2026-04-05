#include "Pacman.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace arcade::bonus {

Pacman::Pacman(PacmanRules rules) : rules_(rules) {
    for (double value :
         {rules.releaseDelay, rules.powerDuration, rules.healingDuration, rules.playerPeriod,
          rules.ghostPeriod, rules.fruitDelay, rules.fruitDuration, rules.fruitInterval,
          rules.jumpDuration, rules.jumpCooldown, rules.ghostJumpInterval})
        if (!std::isfinite(value) || value <= 0)
            throw std::invalid_argument("invalid Pacman rules");
    reset(1);
}

void Pacman::reset(std::uint64_t seed) {
    seed_ = seed;
    ++generation_;
    score_ = 0;
    bonusElapsed_ = 0;
    level_ = 1;
    status_ = Status::Running;
    buildLevel();
}

void Pacman::buildLevel() {
    board_.fill(Cell::Pellet);
    for (int y = 0; y < Height; ++y)
        for (int x = 0; x < Width; ++x) {
            const bool edge = x == 0 || y == 0 || x == Width - 1 || y == Height - 1;
            if (edge && x != 12 && y != 10)
                board_.at({x, y}) = Cell::Wall;
            if ((x == 4 || x == 8 || x == 16 || x == 20) && y >= 2 && y <= 18 && y != 4 &&
                y != 10 && y != 16)
                board_.at({x, y}) = Cell::Wall;
            if (x >= 10 && x <= 14 && y >= 8 && y <= 11)
                board_.at({x, y}) = Cell::Home;
        }
    for (Point p : std::array<Point, 4>{{{1, 1}, {23, 1}, {1, 19}, {23, 19}}})
        board_.at(p) = Cell::PowerPellet;
    player_ = {12, 12};
    board_.at(player_) = Cell::Empty;
    heading_ = desired_ = {-1, 0};
    ghosts_ = {Ghost{{10, 9}}, Ghost{{11, 9}}, Ghost{{13, 9}}, Ghost{{14, 9}}};
    pellets_ = static_cast<int>(
        std::count(board_.cells().begin(), board_.cells().end(), Cell::Pellet) +
        std::count(board_.cells().begin(), board_.cells().end(), Cell::PowerPellet));
    elapsed_ = power_ = accumulated_ = playerClock_ = 0;
    combo_ = 0;
    jump_ = {};
    fruitActive_ = false;
    fruitClock_ = rules_.fruitDelay;
}

bool Pacman::walkable(Point p, bool homeAllowed) const {
    const auto cell = board_.at(board_.wrap(p));
    return cell != Cell::Wall && (homeAllowed || cell != Cell::Home);
}

void Pacman::consume() {
    collectFruit();
    auto &cell = board_.at(player_);
    if (cell != Cell::Pellet && cell != Cell::PowerPellet)
        return;
    if (cell == Cell::PowerPellet) {
        power_ = rules_.powerDuration;
        combo_ = 0;
        score_ += 50;
    } else
        score_ += 10;
    cell = Cell::Empty;
    --pellets_;
}

void Pacman::movePlayer() {
    if (walkable(add(player_, desired_), false))
        heading_ = desired_;
    if (walkable(add(player_, heading_), false))
        player_ = board_.wrap(add(player_, heading_));
    consume();
    collisions();
    if (pellets_ == 0 && status_ == Status::Running) {
        ++level_;
        buildLevel();
    }
}

void Pacman::collisions() {
    for (auto &ghost : ghosts_) {
        if (ghost.position != player_ || ghost.mode != GhostMode::Active || !sameHeight(ghost))
            continue;
        if (power_ > 0) {
            ghost.mode = GhostMode::Eyes;
            ghost.clock = 0;
            ghost.jump = {};
            score_ += 200LL * (1LL << std::min(combo_++, 3));
        } else
            status_ = Status::Lost;
    }
}

void Pacman::tick(double seconds) {
    elapsed_ += seconds;
    if (rules_.bonusEnabled)
        bonusTick(seconds);
    power_ = std::max(0.0, power_ - seconds);
    playerClock_ += seconds;
    const double speed = acceleration();
    if (playerClock_ + 1e-9 >= rules_.playerPeriod / speed) {
        playerClock_ -= rules_.playerPeriod / speed;
        movePlayer();
    }
    if (status_ != Status::Running)
        return;
    for (std::size_t i = 0; i < ghosts_.size(); ++i) {
        auto &ghost = ghosts_[i];
        if (ghost.mode == GhostMode::Caged) {
            if (elapsed_ + 1e-9 < rules_.releaseDelay)
                continue;
            ghost.mode = GhostMode::Active;
        }
        if (ghost.mode == GhostMode::Healing) {
            ghost.healing -= seconds;
            if (ghost.healing > 0)
                continue;
            ghost.mode = GhostMode::Active;
        }
        ghost.clock += seconds;
        const double period = ghost.mode == GhostMode::Eyes
                                  ? rules_.ghostPeriod * 0.35
                                  : rules_.ghostPeriod * (power_ > 0 ? 1.6 : 1.0) / speed;
        if (ghost.clock + 1e-9 >= period) {
            ghost.clock -= period;
            moveGhost(i);
        }
    }
    collisions();
}

void Pacman::update(double seconds, const std::vector<Input> &inputs) {
    validateDelta(seconds);
    if (status_ != Status::Running)
        return;
    for (const auto &input : inputs) {
        bonusInput(input.action);
        if (direction(input.action) != Point{})
            desired_ = direction(input.action);
    }
    accumulated_ += seconds;
    while (accumulated_ + 1e-9 >= 0.01 && status_ == Status::Running) {
        accumulated_ -= 0.01;
        tick(0.01);
    }
}

} // namespace arcade::bonus

#ifndef ARCADE_NO_ENTRY
ARCADE_PLUGIN(arcade::bonus::Pacman, arcade::PluginKind::Game, "pacman", "PACMAN")
#endif
