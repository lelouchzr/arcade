#include "Centipede.hpp"
#include <algorithm>

namespace arcade::bonus {

void Centipede::spawnBonusBody(CentipedeType type) {
    CentipedeBody body;
    body.type = type;
    const int length = type == CentipedeType::Racer ? std::max(2, rules_.initialLength - 3)
                                                    : std::min(20, rules_.initialLength + 2);
    const int preferredRow = type == CentipedeType::Racer ? 1 : 2;
    const bool right = (++spawnSequence_ % 2) == 0;
    body.direction = right ? 1 : -1;
    bool available = false;
    for (int attempt = 0; attempt < 6 && !available; ++attempt) {
        const int row = (preferredRow + attempt) % 6;
        body.segments.clear();
        for (int index = 0; index < length; ++index)
            body.segments.push_back({right ? length - 1 - index : Width - length + index, row});
        available = std::none_of(bodies_.begin(), bodies_.end(), [&](const CentipedeBody &other) {
            return std::any_of(body.segments.begin(), body.segments.end(), [&](Point point) {
                return std::find(other.segments.begin(), other.segments.end(), point) !=
                       other.segments.end();
            });
        });
    }
    if (!available)
        return;
    // Generated scenery cannot materialize inside an arriving body.
    for (const auto point : body.segments)
        obstacles_[board_.index(point)] = 0;
    bodies_.push_back(std::move(body));
}

void Centipede::tickBonusBodies(double seconds) {
    spawnClock_ += seconds;
    if (spawnClock_ + 1e-9 >= 5) {
        spawnClock_ -= 5;
        if (bodies_.size() < 6)
            spawnBonusBody(spawnSequence_ % 2 == 0 ? CentipedeType::Racer : CentipedeType::Weaver);
    }
    for (auto &body : bodies_) {
        const double scale = body.type == CentipedeType::Racer    ? 0.6
                             : body.type == CentipedeType::Weaver ? 1.3
                                                                  : 1.0;
        const double period = rules_.movementPeriod * scale / (1.0 + 0.08 * (level_ - 1));
        body.clock += seconds;
        if (body.clock + 1e-9 >= period) {
            body.clock -= period;
            moveBody(body);
        }
    }
    resolveBodies();
}

void Centipede::moveSecondPlayer(Action action) {
    Point delta;
    switch (action) {
    case Action::PlayerTwoUp:
        delta = {0, -1};
        break;
    case Action::PlayerTwoDown:
        delta = {0, 1};
        break;
    case Action::PlayerTwoLeft:
        delta = {-1, 0};
        break;
    case Action::PlayerTwoRight:
        delta = {1, 0};
        break;
    case Action::PlayerTwoFire:
        if (!shot_) {
            shot_ = secondPlayer_;
            shotClock_ = 0;
        }
        return;
    default:
        return;
    }
    const auto next = add(secondPlayer_, delta);
    if (board_.contains(next) && next.y >= PlayerTop && next != player_ &&
        obstacles_[board_.index(next)] == 0)
        secondPlayer_ = next;
}

} // namespace arcade::bonus
