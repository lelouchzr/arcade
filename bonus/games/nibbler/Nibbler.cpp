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
    level_ = 1;
    score_ = 0;
    buildLevel();
}

void Nibbler::update(double seconds, const std::vector<Input> &inputs) {
    if (!std::isfinite(seconds) || seconds < 0 || seconds > 60)
        throw std::invalid_argument("invalid game delta time");
    if (status_ != Status::Running)
        return;
    remaining_ = std::max(0.0, remaining_ - seconds);
    if (remaining_ <= 0) {
        status_ = Status::Lost;
        return;
    }
    for (const auto &input : inputs) {
        Point candidate = direction_;
        if (input.action == Action::Up)
            candidate = {0, -1};
        if (input.action == Action::Down)
            candidate = {0, 1};
        if (input.action == Action::Left)
            candidate = {-1, 0};
        if (input.action == Action::Right)
            candidate = {1, 0};
        if (candidate.x != -direction_.x && candidate.y != -direction_.y)
            desired_ = candidate;
    }
    accumulated_ += seconds;
    const double step = std::max(0.06, 0.16 - 0.003 * level_);
    while (accumulated_ + 1e-9 >= step && status_ == Status::Running) {
        accumulated_ -= step;
        move();
    }
}

Snapshot Nibbler::snapshot() const {
    Snapshot view;
    view.width = Width;
    view.height = Height;
    view.cells = map_;
    for (const auto point : body_)
        view.cells[static_cast<std::size_t>(point.y * Width + point.x)] = Cell::Body;
    view.cells[static_cast<std::size_t>(body_.front().y * Width + body_.front().x)] = Cell::Head;
    view.score = score_;
    view.secondsLeft = remaining_;
    view.status = status_;
    view.information = {"ARROWS  STEER",
                        "AUTO TURN AT WALLS",
                        "T-JUNCTION  CHOOSE",
                        "EAT ALL FOOD TO ADVANCE",
                        "LEVEL  " + std::to_string(level_) + " / 32",
                        "FOOD   " + std::to_string(food_)};
    return view;
}

bool Nibbler::wall(Point p) const {
    return p.x < 0 || p.y < 0 || p.x >= Width || p.y >= Height ||
           map_[static_cast<std::size_t>(p.y * Width + p.x)] == Cell::Wall;
}

Point Nibbler::ahead(Point direction) const {
    return {body_.front().x + direction.x, body_.front().y + direction.y};
}

void Nibbler::buildLevel() {
    map_.assign(Width * Height, Cell::Empty);
    for (int y = 0; y < Height; ++y)
        for (int x = 0; x < Width; ++x) {
            if (x == 0 || y == 0 || x == Width - 1 || y == Height - 1)
                map_[static_cast<std::size_t>(y * Width + x)] = Cell::Wall;
            // Encode 4 x 4 x 2 distinct passage configurations across 32 stages.
            if (x % 6 == 0 && x > 0 && x < Width - 1 && y >= 2 && y <= 17) {
                const int baffle = x / 6 - 1;
                const int mask = baffle == 2 ? 1 : 3;
                const int variation = ((level_ - 1) >> (baffle * 2)) & mask;
                const int gap = (variation + 1 + static_cast<int>(seed_ % 5)) % 5;
                if (y % 5 != gap)
                    map_[static_cast<std::size_t>(y * Width + x)] = Cell::Wall;
            }
            if (x > 0 && y > 0 && x < Width - 1 && y < Height - 1 && x % 4 == 0 && y % 4 == 2 &&
                map_[static_cast<std::size_t>(y * Width + x)] != Cell::Wall)
                map_[static_cast<std::size_t>(y * Width + x)] = Cell::Food;
        }
    body_ = {{12, 19}, {11, 19}, {10, 19}, {9, 19}};
    for (auto p : body_)
        map_[static_cast<std::size_t>(p.y * Width + p.x)] = Cell::Empty;
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
    const bool grows = map_[static_cast<std::size_t>(next.y * Width + next.x)] == Cell::Food;
    const auto end = grows ? body_.end() : std::prev(body_.end());
    if (std::find(body_.begin(), end, next) != end) {
        status_ = Status::Lost;
        return;
    }
    body_.push_front(next);
    if (grows) {
        map_[static_cast<std::size_t>(next.y * Width + next.x)] = Cell::Empty;
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

ARCADE_PLUGIN(arcade::bonus::Nibbler, arcade::PluginKind::Game, "nibbler", "NIBBLER / 32")
