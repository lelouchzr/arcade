#include "Pacman.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace arcade::bonus {

double Pacman::elevation(const PacmanJump &jump) const {
    if (jump.elapsed < 0)
        return 0;
    const double progress = std::clamp(jump.elapsed / rules_.jumpDuration, 0.0, 1.0);
    return 1.4 * std::sin(std::numbers::pi * progress);
}

void Pacman::advanceJump(PacmanJump &jump, double seconds) const {
    jump.cooldown = std::max(0.0, jump.cooldown - seconds);
    if (jump.elapsed >= 0) {
        jump.elapsed += seconds;
        if (jump.elapsed + 1e-9 >= rules_.jumpDuration)
            jump.elapsed = -1;
    }
}

bool Pacman::startJump(PacmanJump &jump) const {
    if (jump.elapsed >= 0 || jump.cooldown > 1e-9)
        return false;
    jump.elapsed = 0;
    jump.cooldown = rules_.jumpCooldown;
    return true;
}

bool Pacman::sameHeight(const Ghost &ghost) const {
    return !rules_.bonusEnabled || std::abs(elevation(jump_) - elevation(ghost.jump)) <= 0.4;
}

} // namespace arcade::bonus
