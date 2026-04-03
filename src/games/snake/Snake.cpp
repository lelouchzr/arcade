#include "Snake.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace arcade {

Snake::Snake() {
    reset(1);
}

void Snake::reset(std::uint64_t seed) {
    random_.seed(seed);
    body_ = {{12, 10}, {11, 10}, {10, 10}, {9, 10}};
    direction_ = pending_ = {1, 0};
    turnQueued_ = false;
    accumulated_ = 0;
    score_ = 0;
    status_ = Status::Running;
#ifdef ARCADE_BONUS
    pickup_.reset();
    boostRemaining_ = 0;
#endif
    spawnFood();
}

void Snake::spawnFood() {
    std::vector<Point> free;
    for (int y = 1; y < Height - 1; ++y)
        for (int x = 1; x < Width - 1; ++x)
            if (std::find(body_.begin(), body_.end(), Point{x, y}) == body_.end())
                free.push_back({x, y});
    if (free.empty()) {
        status_ = Status::Won;
        food_ = {-1, -1};
        return;
    }
    food_ = free[std::uniform_int_distribution<std::size_t>(0, free.size() - 1)(random_)];
#ifdef ARCADE_BONUS
    if (pickup_.active() && pickup_.position() == food_)
        pickup_.clear();
#endif
}

void Snake::step() {
    direction_ = pending_;
    turnQueued_ = false;
    const Point head{body_.front().x + direction_.x, body_.front().y + direction_.y};
    const bool grows = head == food_;
    // The old tail vacates its cell on an ordinary move; entering it is legal.
    auto collisionEnd = grows ? body_.end() : std::prev(body_.end());
    if (head.x <= 0 || head.y <= 0 || head.x >= Width - 1 || head.y >= Height - 1 ||
        std::find(body_.begin(), collisionEnd, head) != collisionEnd) {
        status_ = Status::Lost;
        return;
    }
    body_.push_front(head);
    if (grows) {
        score_ += 100;
        spawnFood();
    } else
        body_.pop_back();
#ifdef ARCADE_BONUS
    if (bonusEnabled_ && pickup_.consume(head))
        score_ += 250;
#endif
}

void Snake::update(double seconds, const std::vector<Input> &inputs) {
    if (!std::isfinite(seconds) || seconds < 0 || seconds > 60)
        throw std::invalid_argument("invalid game delta time");
#ifdef ARCADE_BONUS
    for (const auto &input : inputs)
        if (input.action == Action::Difficulty) {
            bonusEnabled_ = !bonusEnabled_;
            pickup_.reset();
            boostRemaining_ = 0;
        }
#endif
    if (status_ != Status::Running)
        return;
    queueTurns(inputs);
#ifdef ARCADE_BONUS
    if (bonusEnabled_) {
        updateBonus(seconds, inputs);
        return;
    }
#endif
    accumulated_ += seconds;
    while (accumulated_ + 1e-9 >= Step && status_ == Status::Running) {
        accumulated_ -= Step;
        step();
    }
}

void Snake::queueTurns(const std::vector<Input> &inputs) {
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
        // One queued turn per tick prevents a rapid two-key 180-degree reversal.
        if (!turnQueued_ && candidate != direction_ && candidate.x != -direction_.x &&
            candidate.y != -direction_.y) {
            pending_ = candidate;
            turnQueued_ = true;
        }
    }
}

Snapshot Snake::snapshot() const {
    Snapshot view;
    view.width = Width;
    view.height = Height;
    view.cells.assign(Width * Height, Cell::Empty);
    for (int y = 0; y < Height; ++y)
        for (int x = 0; x < Width; ++x)
            if (x == 0 || y == 0 || x == Width - 1 || y == Height - 1)
                view.cells[static_cast<std::size_t>(y * Width + x)] = Cell::Wall;
    if (food_.x >= 0)
        view.cells[static_cast<std::size_t>(food_.y * Width + food_.x)] = Cell::Food;
#ifdef ARCADE_BONUS
    if (bonusEnabled_ && pickup_.active()) {
        const Point p = pickup_.position();
        view.cells[static_cast<std::size_t>(p.y * Width + p.x)] = Cell::BonusFood;
    }
#endif
    for (auto point : body_)
        view.cells[static_cast<std::size_t>(point.y * Width + point.x)] = Cell::Body;
    view.cells[static_cast<std::size_t>(body_.front().y * Width + body_.front().x)] = Cell::Head;
    view.score = score_;
    view.status = status_;
    view.information = {"ARROWS  STEER", "EAT TO GROW", "AVOID WALLS AND BODY",
                        "LENGTH  " + std::to_string(body_.size()), "SPEED   7.14 CELLS/S"};
#ifdef ARCADE_BONUS
    view.information.push_back(bonusEnabled_ ? "D BONUS ON" : "D BONUS OFF");
    if (bonusEnabled_) {
        view.information.back() += " SPACE BOOST";
        view.information[4] =
            "SPEED   " + std::to_string(1 / stepInterval()).substr(0, 4) + " CELLS/S";
        view.information.push_back(
            pickup_.active()
                ? "BONUS +250  " +
                      std::to_string(static_cast<int>(std::ceil(pickup_.remaining()))) + "S"
                : "BONUS +250 COOLDOWN 10S");
    }
#endif
    return view;
}
} // namespace arcade
#ifndef ARCADE_NO_ENTRY
ARCADE_PLUGIN(arcade::Snake, arcade::PluginKind::Game, "snake", "SNAKE")
#endif
