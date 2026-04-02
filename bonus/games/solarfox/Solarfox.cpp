#include "Solarfox.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace arcade::bonus {

Solarfox::Solarfox(SolarfoxRules rules) : rules_(rules) {
    for (double value : {rules.movementPeriod, rules.enemyShotPeriod, rules.gunPeriod})
        if (!std::isfinite(value) || value <= 0)
            throw std::invalid_argument("invalid Solarfox rules");
    reset(1);
}

void Solarfox::reset(std::uint64_t seed) {
    (void)seed;
    ++generation_;
    score_ = 0;
    level_ = 1;
    status_ = Status::Running;
    buildLevel();
}

bool Solarfox::walkable(Point p) const {
    return p.x >= Margin && p.x < Width - Margin && p.y >= Margin && p.y < Height - Margin;
}

void Solarfox::buildLevel() {
    board_.fill(Cell::Wall);
    for (int y = Margin; y < Height - Margin; ++y)
        for (int x = Margin; x < Width - Margin; ++x)
            board_.at({x, y}) = Cell::Empty;
    for (int y = 5; y <= 17; y += 4)
        for (int x = 5; x <= 21; x += 4)
            board_.at({x, y}) = Cell::Powerup;
    board_.at({7, 7}) = Cell::BadPowerup;
    board_.at({17, 13}) = Cell::BadPowerup;
    powerups_ =
        static_cast<int>(std::count(board_.cells().begin(), board_.cells().end(), Cell::Powerup));
    guns_ = {EnemyGun{{2, 5}, {1, 0}, 0}, EnemyGun{{22, 15}, {-1, 0}, 0.25},
             EnemyGun{{8, 2}, {0, 1}, 0.5}, EnemyGun{{16, 18}, {0, -1}, 0.75}};
    for (const auto &gun : guns_)
        board_.at(gun.position) = Cell::EnemyGun;
    player_ = {12, 10};
    heading_ = desired_ = {1, 0};
    shots_.clear();
    enemyShots_.clear();
    accumulated_ = movementClock_ = shotClock_ = enemyShotClock_ = boost_ = 0;
    turnQueued_ = false;
}

void Solarfox::collisions() {
    if (!walkable(player_) || board_.at(player_) == Cell::BadPowerup) {
        status_ = Status::Lost;
        return;
    }
    for (const auto &shot : enemyShots_)
        if (shot.remaining >= 0 && shot.position == player_) {
            status_ = Status::Lost;
            return;
        }
}

void Solarfox::movePlayer() {
    heading_ = desired_;
    turnQueued_ = false;
    player_ = add(player_, heading_);
    collisions();
}

void Solarfox::tick(double seconds) {
    boost_ = std::max(0.0, boost_ - seconds);
    const double acceleration = (1.0 + 0.08 * (level_ - 1)) * (boost_ > 0 ? 2.0 : 1.0);
    const double period = rules_.movementPeriod / acceleration;
    movementClock_ += seconds;
    shotClock_ += seconds;
    enemyShotClock_ += seconds;
    for (auto &gun : guns_) {
        gun.clock += seconds;
        if (gun.clock + 1e-9 >= rules_.gunPeriod / (1.0 + 0.06 * (level_ - 1))) {
            gun.clock = 0;
            enemyShots_.push_back({gun.position, gun.direction, Width + Height});
        }
    }
    if (shotClock_ + 1e-9 >= period / 4.0) {
        shotClock_ -= period / 4.0;
        moveShots(false);
    }
    if (enemyShotClock_ + 1e-9 >= rules_.enemyShotPeriod) {
        enemyShotClock_ -= rules_.enemyShotPeriod;
        moveShots(true);
    }
    std::erase_if(shots_, [](const SolarShot &shot) { return shot.remaining <= 0; });
    std::erase_if(enemyShots_, [](const SolarShot &shot) { return shot.remaining <= 0; });
    if (movementClock_ + 1e-9 >= period) {
        movementClock_ -= period;
        movePlayer();
    }
    if (status_ == Status::Running && powerups_ == 0) {
        ++level_;
        buildLevel();
    }
}

void Solarfox::update(double seconds, const std::vector<Input> &inputs) {
    validateDelta(seconds);
    if (status_ != Status::Running)
        return;
    for (const auto &input : inputs) {
        const auto d = direction(input.action);
        if (!turnQueued_ && d != Point{} && d != heading_ && d != Point{-heading_.x, -heading_.y}) {
            desired_ = d;
            turnQueued_ = true;
        }
        if (input.action == Action::Primary && shots_.size() < 32)
            shots_.push_back({player_, heading_, 2});
        if (input.action == Action::Secondary)
            boost_ = 0.6;
    }
    accumulated_ += seconds;
    while (accumulated_ + 1e-9 >= 0.005 && status_ == Status::Running) {
        accumulated_ -= 0.005;
        tick(0.005);
    }
}

} // namespace arcade::bonus

#ifndef ARCADE_NO_ENTRY
ARCADE_PLUGIN(arcade::bonus::Solarfox, arcade::PluginKind::Game, "solarfox", "SOLAR FOX")
#endif
