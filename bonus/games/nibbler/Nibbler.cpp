#include "Nibbler.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace arcade::bonus {

Nibbler::Nibbler() {
    reset(1);
}

void Nibbler::reset(std::uint64_t seed) {
    seed_ = seed;
    ++generation_;
    random_.seed(seed);
    level_ = 1;
    score_ = 0;
    buildLevel();
}

void Nibbler::update(double seconds, const std::vector<Input> &inputs) {
    if (!std::isfinite(seconds) || seconds < 0 || seconds > 60)
        throw std::invalid_argument("invalid game delta time");
    for (const auto &input : inputs)
        if (input.action == Action::Difficulty) {
            if (!bonusEnabled_)
                bonusEnabled_ = true;
            else if (!originalLevels_)
                originalLevels_ = true;
            else
                bonusEnabled_ = originalLevels_ = false;
            reset(seed_);
            return;
        }
    if (status_ != Status::Running)
        return;
    for (const auto &input : inputs) {
        Point candidate = direction_;
        switch (input.action) {
        case Action::Up:
            candidate = {0, -1};
            break;
        case Action::Down:
            candidate = {0, 1};
            break;
        case Action::Left:
            candidate = {-1, 0};
            break;
        case Action::Right:
            candidate = {1, 0};
            break;
        default:
            continue;
        }
        if (candidate.x != -direction_.x && candidate.y != -direction_.y)
            desired_ = candidate;
    }
    if (bonusEnabled_) {
        updateBonus(seconds, inputs);
        return;
    }
    updateClassic(seconds);
}

void Nibbler::updateClassic(double seconds) {
    // Consume time up to each event so a level transition retains the frame remainder.
    double left = seconds;
    while (left > 1e-12 && status_ == Status::Running) {
        const double step = baseStep();
        const double slice = std::min({left, std::max(0.0, step - accumulated_), remaining_});
        accumulated_ += slice;
        remaining_ = std::max(0.0, remaining_ - slice);
        left -= slice;
        if (remaining_ <= 1e-9) {
            remaining_ = 0;
            status_ = Status::Lost;
        } else if (accumulated_ + 1e-9 >= step) {
            accumulated_ = std::max(0.0, accumulated_ - step);
            move();
        }
    }
}

Snapshot Nibbler::snapshot() const {
    Snapshot view;
    view.width = width_;
    view.height = height_;
    view.cells = map_;
    if (originalLevels_)
        view.camera = Camera{body_.front(), std::min(width_, 30), std::min(height_, 21), false};
    if (bonusEnabled_ && pickup_.active()) {
        const Point p = pickup_.position();
        view.cells[static_cast<std::size_t>(p.y * width_ + p.x)] = Cell::BonusFood;
    }
    for (const auto point : body_)
        view.cells[static_cast<std::size_t>(point.y * width_ + point.x)] = Cell::Body;
    view.cells[static_cast<std::size_t>(body_.front().y * width_ + body_.front().x)] = Cell::Head;
    view.score = score_;
    view.generation = generation_;
    view.secondsLeft = remaining_;
    view.status = status_;
    view.information = {"ARROWS  STEER",
                        "AUTO TURN AT WALLS",
                        "T-JUNCTION  CHOOSE",
                        "EAT ALL FOOD TO ADVANCE",
                        "LEVEL  " + std::to_string(level_) + " / 32",
                        "FOOD   " + std::to_string(food_)};
    std::string mode = bonusEnabled_ ? "D BONUS GENERATED" : "D CLASSIC GENERATED";
    if (originalLevels_)
        mode = originalLevel().certified ? "D ORIGINAL 1982" : "D PACK UNVERIFIED";
    view.information.push_back(mode);
    if (bonusEnabled_) {
        view.information[0] = "ARROWS SPACE BOOST";
        view.information[3] =
            pickup_.active()
                ? "BONUS +500  " +
                      std::to_string(static_cast<int>(std::ceil(pickup_.remaining()))) + "S"
                : "BONUS +500 COOLDOWN 10S";
        view.information[5] += " SPEED " + std::to_string(1 / stepInterval()).substr(0, 4);
    }
    return view;
}

bool Nibbler::wall(Point p) const {
    return p.x < 0 || p.y < 0 || p.x >= width_ || p.y >= height_ ||
           map_[static_cast<std::size_t>(p.y * width_ + p.x)] == Cell::Wall;
}

Point Nibbler::ahead(Point direction) const {
    return {body_.front().x + direction.x, body_.front().y + direction.y};
}

void Nibbler::buildLevel() {
    pickup_.reset();
    boostRemaining_ = 0;
    if (originalLevels_) {
        buildOriginalLevel();
        return;
    }
    width_ = Width;
    height_ = Height;
    map_.assign(static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_), Cell::Empty);
    for (int y = 0; y < height_; ++y)
        for (int x = 0; x < width_; ++x) {
            if (x == 0 || y == 0 || x == width_ - 1 || y == height_ - 1)
                map_[static_cast<std::size_t>(y * width_ + x)] = Cell::Wall;
            // Encode 4 x 4 x 2 distinct passage configurations across 32 stages.
            if (x % 6 == 0 && x > 0 && x < width_ - 1 && y >= 2 && y <= 17) {
                const int baffle = x / 6 - 1;
                const int mask = baffle == 2 ? 1 : 3;
                const int variation = ((level_ - 1) >> (baffle * 2)) & mask;
                const int gap = (variation + 1 + static_cast<int>(seed_ % 5)) % 5;
                if (y % 5 != gap)
                    map_[static_cast<std::size_t>(y * width_ + x)] = Cell::Wall;
            }
            if (x > 0 && y > 0 && x < width_ - 1 && y < height_ - 1 && x % 4 == 0 && y % 4 == 2 &&
                map_[static_cast<std::size_t>(y * width_ + x)] != Cell::Wall)
                map_[static_cast<std::size_t>(y * width_ + x)] = Cell::Food;
        }
    body_ = {{12, 19}, {11, 19}, {10, 19}, {9, 19}};
    for (auto p : body_)
        map_[static_cast<std::size_t>(p.y * width_ + p.x)] = Cell::Empty;
    direction_ = desired_ = {1, 0};
    remaining_ = 90;
    accumulated_ = 0;
    status_ = Status::Running;
    food_ = static_cast<int>(std::count(map_.begin(), map_.end(), Cell::Food));
}

void Nibbler::move() {
    if (!wall(ahead(desired_)))
        direction_ = desired_;
    if (wall(ahead(direction_))) {
        const Point left{direction_.y, -direction_.x}, right{-direction_.y, direction_.x};
        const bool leftOpen = !wall(ahead(left)), rightOpen = !wall(ahead(right));
        if (leftOpen == rightOpen)
            return; // A T-junction waits for a deliberate choice.
        direction_ = desired_ = leftOpen ? left : right;
    }
    const auto next = ahead(direction_);
    const bool grows = map_[static_cast<std::size_t>(next.y * width_ + next.x)] == Cell::Food;
    const auto end = grows ? body_.end() : std::prev(body_.end());
    if (std::find(body_.begin(), end, next) != end) {
        status_ = Status::Lost;
        return;
    }
    body_.push_front(next);
    if (bonusEnabled_ && pickup_.consume(next))
        score_ += 500;
    if (grows) {
        map_[static_cast<std::size_t>(next.y * width_ + next.x)] = Cell::Empty;
        --food_;
        score_ += 100;
        if (food_ == 0) {
            score_ += static_cast<std::int64_t>(remaining_) * 10;
            if (level_ == 32)
                status_ = Status::Won;
            else {
                ++level_;
                buildLevel();
            }
        }
    } else
        body_.pop_back();
}
} // namespace arcade::bonus

#ifndef ARCADE_NO_ENTRY
ARCADE_PLUGIN(arcade::bonus::Nibbler, arcade::PluginKind::Game, "nibbler", "NIBBLER / 32")
#endif
