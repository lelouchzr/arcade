#include "Solarfox.hpp"
#include <algorithm>

namespace arcade::bonus {

void Solarfox::intercept() {
    for (auto &shot : shots_)
        for (auto &enemy : enemyShots_)
            if (shot.remaining >= 0 && enemy.remaining >= 0 && shot.position == enemy.position) {
                shot.remaining = enemy.remaining = -1;
                score_ += 25;
                break;
            }
}

void Solarfox::moveShots(bool enemy) {
    auto &shots = enemy ? enemyShots_ : shots_;
    for (auto &shot : shots) {
        if (shot.remaining < 0)
            continue;
        shot.position = add(shot.position, shot.direction);
        if (!walkable(shot.position)) {
            shot.remaining = -1;
            continue;
        }
        --shot.remaining;
        // Keep the last cell alive through interception and collection below.
    }
    if (!enemy) {
        for (auto &shot : shots_) {
            if (shot.remaining < 0 || !walkable(shot.position))
                continue;
            const bool blocked =
                std::any_of(enemyShots_.begin(), enemyShots_.end(), [&](const auto &other) {
                    return other.remaining >= 0 && other.position == shot.position;
                });
            if (blocked) {
                for (auto &other : enemyShots_)
                    if (other.position == shot.position && other.remaining >= 0) {
                        other.remaining = -1;
                        score_ += 25;
                        break;
                    }
                shot.remaining = -1;
                continue;
            }
            auto &cell = board_.at(shot.position);
            if (cell == Cell::Powerup) {
                cell = Cell::Empty;
                --powerups_;
                score_ += 100;
                shot.remaining = -1;
            } else if (cell == Cell::BadPowerup) {
                cell = Cell::Empty;
                shot.remaining = -1;
            }
        }
    }
    // Enemy shots can move onto stationary player shots, including their second cell.
    intercept();
    if (enemy)
        collisions();
}

} // namespace arcade::bonus
