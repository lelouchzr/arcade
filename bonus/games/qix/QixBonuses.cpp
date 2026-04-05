#include "Qix.hpp"
#include <algorithm>

namespace arcade::bonus {

void Qix::initializeBonuses() {
    pickups_.clear();
    shield_ = slow_ = pickupClock_ = 0;
    originalArea_ = (Width - 2) * (Height - 2);
    if (!rules_.bonus)
        return;
    // Short islands leave corridors on every side and never block the initial actors.
    for (const Point point : {Point{6, 6}, Point{6, 7}, Point{18, 14}, Point{18, 15}}) {
        board_.at(point) = Cell::Obstacle;
        --originalArea_;
    }
    pickups_ = {{{3, 10}, QixPower::Shield, 8}, {{21, 10}, QixPower::Slow, 8}};
}

void Qix::collectBonuses() {
    if (!rules_.bonus)
        return;
    std::erase_if(pickups_, [&](const QixPickup &pickup) {
        const bool touched =
            pickup.position == player_ ||
            std::find(trail_.begin(), trail_.end(), pickup.position) != trail_.end();
        if (!touched)
            return false;
        if (pickup.effect == QixPower::Shield) {
            shield_ = 5;
            burning_ = false;
            idle_ = 0;
        } else {
            slow_ = 5;
        }
        score_ += 250;
        return true;
    });
}

void Qix::spawnPickup() {
    if (pickups_.size() >= 3)
        return;
    std::vector<Point> candidates;
    for (std::size_t index = 0; index < board_.cells().size(); ++index) {
        const auto point = board_.point(index);
        if (board_.at(point) != Cell::Empty ||
            std::find(monster_.begin(), monster_.end(), point) != monster_.end() ||
            std::any_of(pickups_.begin(), pickups_.end(),
                        [&](const QixPickup &pickup) { return pickup.position == point; }))
            continue;
        candidates.push_back(point);
    }
    if (candidates.empty())
        return;
    const auto index =
        std::uniform_int_distribution<std::size_t>(0, candidates.size() - 1)(random_);
    const auto effect =
        std::uniform_int_distribution<int>(0, 1)(random_) == 0 ? QixPower::Shield : QixPower::Slow;
    pickups_.push_back({candidates[index], effect, 8});
}

void Qix::tickBonuses(double seconds) {
    if (!rules_.bonus)
        return;
    shield_ = std::max(0.0, shield_ - seconds);
    slow_ = std::max(0.0, slow_ - seconds);
    for (auto &pickup : pickups_)
        pickup.remaining -= seconds;
    std::erase_if(pickups_, [](const QixPickup &pickup) { return pickup.remaining <= 1e-9; });
    collectBonuses();
    pickupClock_ += seconds;
    if (pickupClock_ + 1e-9 >= 4) {
        pickupClock_ -= 4;
        spawnPickup();
    }
}

} // namespace arcade::bonus
