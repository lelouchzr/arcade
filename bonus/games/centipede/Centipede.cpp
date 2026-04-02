#include "Centipede.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace arcade::bonus {

Centipede::Centipede(CentipedeRules rules) : rules_(rules) {
    if (!std::isfinite(rules.movementPeriod) || rules.movementPeriod <= 0 ||
        !std::isfinite(rules.shotPeriod) || rules.shotPeriod <= 0 || rules.initialLength < 2 ||
        rules.initialLength > 20 || rules.obstacles < 0 || rules.obstacles > 200)
        throw std::invalid_argument("invalid Centipede rules");
    reset(1);
}

void Centipede::reset(std::uint64_t seed) {
    random_.seed(seed);
    ++generation_;
    score_ = 0;
    survived_ = 0;
    level_ = 1;
    status_ = Status::Running;
    accumulated_ = movementClock_ = shotClock_ = 0;
    player_ = {12, 20};
    shot_.reset();
    buildField();
    spawnWave();
}

void Centipede::buildField() {
    obstacles_.assign(board_.cells().size(), 0);
    std::vector<Point> candidates;
    for (int y = 2; y < PlayerTop; ++y)
        for (int x = 0; x < Width; ++x)
            candidates.push_back({x, y});
    std::shuffle(candidates.begin(), candidates.end(), random_);
    for (int i = 0; i < rules_.obstacles; ++i)
        obstacles_[board_.index(candidates[static_cast<std::size_t>(i)])] = 5;
}

void Centipede::spawnWave() {
    CentipedeBody body;
    const bool right = (survived_ % 2) == 0;
    body.direction = right ? 1 : -1;
    for (int i = 0; i < rules_.initialLength; ++i)
        body.segments.push_back(
            {right ? rules_.initialLength - 1 - i : Width - rules_.initialLength + i, 0});
    bodies_ = {std::move(body)};
    movementClock_ = 0;
}

void Centipede::collisions() {
    for (const auto &body : bodies_)
        if (std::find(body.segments.begin(), body.segments.end(), player_) != body.segments.end()) {
            status_ = Status::Lost;
            return;
        }
}

void Centipede::finishWave() {
    if (!bodies_.empty() || status_ != Status::Running)
        return;
    ++survived_;
    if (survived_ == 20) {
        survived_ = 0;
        ++level_;
        buildField();
    }
    spawnWave();
}

void Centipede::tick(double seconds) {
    movementClock_ += seconds;
    shotClock_ += seconds;
    if (shotClock_ + 1e-9 >= rules_.shotPeriod) {
        shotClock_ -= rules_.shotPeriod;
        moveShot();
    }
    const double period = rules_.movementPeriod / (1.0 + 0.08 * (level_ - 1));
    if (movementClock_ + 1e-9 >= period) {
        movementClock_ -= period;
        moveBodies();
    }
    finishWave();
}

void Centipede::update(double seconds, const std::vector<Input> &inputs) {
    validateDelta(seconds);
    if (status_ != Status::Running)
        return;
    for (const auto &input : inputs) {
        const auto d = direction(input.action);
        if (d != Point{}) {
            const auto next = add(player_, d);
            if (board_.contains(next) && next.y >= PlayerTop && obstacles_[board_.index(next)] == 0)
                player_ = next;
        }
        if (input.action == Action::Primary && !shot_) {
            shot_ = player_;
            shotClock_ = 0;
        }
        collisions();
    }
    accumulated_ += seconds;
    while (accumulated_ + 1e-9 >= 0.005 && status_ == Status::Running) {
        accumulated_ -= 0.005;
        tick(0.005);
    }
}

} // namespace arcade::bonus

#ifndef ARCADE_NO_ENTRY
ARCADE_PLUGIN(arcade::bonus::Centipede, arcade::PluginKind::Game, "centipede", "CENTIPEDE")
#endif
