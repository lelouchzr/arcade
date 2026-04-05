#include "Pacman.hpp"
#include <algorithm>

namespace arcade::bonus {

void Pacman::bonusInput(Action action) {
    if (action == Action::Difficulty) {
        rules_.bonusEnabled = !rules_.bonusEnabled;
        reset(seed_);
    } else if (rules_.bonusEnabled) {
        if (action == Action::Primary || action == Action::Jump)
            startJump(jump_);
        if (action == Action::Camera)
            cameraEnabled_ = !cameraEnabled_;
    }
}

double Pacman::acceleration() const {
    const double timed = rules_.bonusEnabled ? std::min(0.6, bonusElapsed_ / 120.0) : 0;
    return 1.0 + 0.08 * (level_ - 1) + timed;
}

void Pacman::collectFruit() {
    if (!rules_.bonusEnabled || !fruitActive_ || player_ != Point{12, 12} || elevation(jump_) > 0.4)
        return;
    score_ += 1000LL * level_;
    fruitActive_ = false;
    fruitClock_ = rules_.fruitInterval;
}

void Pacman::bonusTick(double seconds) {
    bonusElapsed_ += seconds;
    advanceJump(jump_, seconds);
    for (std::size_t i = 0; i < ghosts_.size(); ++i) {
        auto &ghost = ghosts_[i];
        if (ghost.mode != GhostMode::Active) {
            ghost.jump = {};
            ghost.jumpClock = 0;
            continue;
        }
        advanceJump(ghost.jump, seconds);
        if (i % 2 == 0)
            continue;
        ghost.jumpClock += seconds;
        if (ghost.jumpClock + 1e-9 >= rules_.ghostJumpInterval &&
            manhattan(ghost.position, player_) <= 5 && startJump(ghost.jump))
            ghost.jumpClock = 0;
    }
    fruitClock_ -= seconds;
    if (fruitClock_ <= 1e-9) {
        fruitActive_ = !fruitActive_;
        fruitClock_ = fruitActive_ ? rules_.fruitDuration : rules_.fruitInterval;
    }
    collectFruit();
}

} // namespace arcade::bonus
