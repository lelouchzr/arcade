#include "Qix.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace arcade::bonus {

Qix::Qix(QixRules rules) : rules_(rules) {
    for (double value : {rules.playerPeriod, rules.monsterPeriod, rules.sparkPeriod,
                         rules.ignitionDelay, rules.firePeriod})
        if (!std::isfinite(value) || value <= 0)
            throw std::invalid_argument("invalid Qix rules");
    reset(1);
}

void Qix::reset(std::uint64_t seed) {
    seed_ = seed;
    random_.seed(seed);
    ++generation_;
    board_.fill(Cell::Empty);
    tracks_.assign(board_.cells().size(), false);
    for (int y = 0; y < Height; ++y)
        for (int x = 0; x < Width; ++x)
            if (x == 0 || y == 0 || x == Width - 1 || y == Height - 1) {
                board_.at({x, y}) = Cell::Boundary;
                tracks_[board_.index({x, y})] = true;
            }
    monster_ = {{12, 10}, {12, 11}, {12, 12}};
    sparks_ = {Spark{{0, 0}}, Spark{{24, 20}}};
    player_ = anchor_ = {0, 10};
    heading_ = {};
    trail_.clear();
    score_ = 0;
    accumulated_ = playerClock_ = monsterClock_ = sparkClock_ = idle_ = fireClock_ = 0;
    fireIndex_ = 0;
    burning_ = false;
    status_ = Status::Running;
    initializeBonuses();
}

void Qix::lose() {
    for (auto point : trail_)
        board_.at(point) = Cell::Empty;
    trail_.clear();
    burning_ = false;
    player_ = anchor_;
    status_ = Status::Lost;
}

void Qix::collisions() {
    if (rules_.bonus && shield_ > 0)
        return;
    for (auto point : monster_)
        if (point == player_ || board_.at(point) == Cell::Trail) {
            lose();
            return;
        }
    for (const auto &spark : sparks_)
        if (spark.position == player_) {
            lose();
            return;
        }
}

void Qix::movePlayer() {
    if (heading_ == Point{})
        return;
    const auto next = board_.wrap(add(player_, heading_));
    const auto cell = board_.at(next);
    if (cell == Cell::Claimed || cell == Cell::Trail || cell == Cell::Obstacle) {
        heading_ = {};
        return;
    }
    if (cell == Cell::Empty) {
        if (trail_.empty())
            anchor_ = player_;
        board_.at(next) = Cell::Trail;
        tracks_[board_.index(next)] = true;
        trail_.push_back(next);
    }
    player_ = next;
    collectBonuses();
    idle_ = 0;
    collisions();
    if (status_ == Status::Running && cell == Cell::Boundary && !trail_.empty())
        capture();
}

void Qix::moveMonster() {
    std::vector<Point> candidates;
    for (auto d : Directions) {
        const auto next = add(monster_.front(), d);
        if (!board_.contains(next))
            continue;
        const auto cell = board_.at(next);
        if ((cell == Cell::Empty || cell == Cell::Trail) &&
            std::find(monster_.begin(), monster_.end(), next) == monster_.end())
            candidates.push_back(next);
    }
    if (!candidates.empty()) {
        const auto next = candidates[std::uniform_int_distribution<std::size_t>(
            0, candidates.size() - 1)(random_)];
        monster_.push_front(next);
        monster_.pop_back();
    }
    collisions();
}

void Qix::tick(double seconds) {
    tickBonuses(seconds);
    idle_ += seconds;
    playerClock_ += seconds;
    const double enemyTime = slow_ > 0 ? seconds * 0.4 : seconds;
    monsterClock_ += enemyTime;
    sparkClock_ += enemyTime;
    if (playerClock_ + 1e-9 >= rules_.playerPeriod) {
        playerClock_ -= rules_.playerPeriod;
        movePlayer();
    }
    if (status_ != Status::Running)
        return;
    if (monsterClock_ + 1e-9 >= rules_.monsterPeriod) {
        monsterClock_ -= rules_.monsterPeriod;
        moveMonster();
    }
    if (status_ != Status::Running)
        return;
    if (sparkClock_ + 1e-9 >= rules_.sparkPeriod) {
        sparkClock_ -= rules_.sparkPeriod;
        moveSparks();
    }
    if (status_ != Status::Running || trail_.empty() || shield_ > 0)
        return;
    if (!burning_ && idle_ + 1e-9 >= rules_.ignitionDelay) {
        burning_ = true;
        fireIndex_ = 0;
        fireClock_ = 0;
    }
    if (burning_) {
        fireClock_ += seconds;
        if (fireClock_ + 1e-9 >= rules_.firePeriod) {
            fireClock_ -= rules_.firePeriod;
            ++fireIndex_;
        }
        if (fireIndex_ + 1 >= trail_.size())
            lose();
    }
}

void Qix::update(double seconds, const std::vector<Input> &inputs) {
    validateDelta(seconds);
    for (const auto &input : inputs)
        if (input.action == Action::Difficulty) {
            rules_.bonus = !rules_.bonus;
            reset(seed_);
            return;
        }
    if (status_ != Status::Running)
        return;
    for (const auto &input : inputs) {
        if (direction(input.action) != Point{})
            heading_ = direction(input.action);
        if (input.action == Action::Secondary || input.action == Action::Stop)
            heading_ = {};
    }
    accumulated_ += seconds;
    while (accumulated_ + 1e-9 >= 0.01 && status_ == Status::Running) {
        accumulated_ -= 0.01;
        tick(0.01);
    }
}

} // namespace arcade::bonus

#ifndef ARCADE_NO_ENTRY
ARCADE_PLUGIN(arcade::bonus::Qix, arcade::PluginKind::Game, "qix", "QIX")
#endif
