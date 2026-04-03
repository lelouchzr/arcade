#include "Nibbler.hpp"
#include "original/OriginalLevels.hpp"
#include <algorithm>
#include <array>
#include <cmath>

namespace arcade::bonus {
double Nibbler::baseStep() const {
    return originalLevels_ ? originalLevel().period : std::max(0.06, 0.16 - 0.003 * level_);
}

double Nibbler::stepInterval() const {
    return baseStep() * (boostRemaining_ > 1e-9 ? 0.5 : 1.0);
}

void Nibbler::spawnBonus() {
    // An imported maze may contain decorative floor enclosed by walls. Only reachable
    // cells can receive a pickup; the moving body does not permanently block a passage.
    std::vector<Point> reachable{body_.front()};
    std::vector<bool> visited(static_cast<std::size_t>(width_ * height_), false);
    visited[static_cast<std::size_t>(body_.front().y * width_ + body_.front().x)] = true;
    for (std::size_t i = 0; i < reachable.size(); ++i) {
        const Point point = reachable[i];
        const std::array<Point, 4> neighbours{{{point.x + 1, point.y},
                                               {point.x - 1, point.y},
                                               {point.x, point.y + 1},
                                               {point.x, point.y - 1}}};
        for (const Point next : neighbours) {
            if (wall(next))
                continue;
            const auto cell = static_cast<std::size_t>(next.y * width_ + next.x);
            if (!visited[cell]) {
                visited[cell] = true;
                reachable.push_back(next);
            }
        }
    }
    std::vector<Point> free;
    for (int y = 1; y < height_ - 1; ++y)
        for (int x = 1; x < width_ - 1; ++x) {
            const Point point{x, y};
            const auto cell = static_cast<std::size_t>(y * width_ + x);
            if (visited[cell] && map_[cell] == Cell::Empty &&
                std::find(body_.begin(), body_.end(), point) == body_.end())
                free.push_back(point);
        }
    if (free.empty())
        pickup_.defer();
    else
        pickup_.place(
            free[std::uniform_int_distribution<std::size_t>(0, free.size() - 1)(random_)]);
}

void Nibbler::updateBonus(double seconds, const std::vector<Input> &inputs) {
    for (const auto &input : inputs)
        if (input.action == Action::Primary)
            boostRemaining_ = 0.6;
    double left = seconds;
    while (left > 1e-12 && status_ == Status::Running) {
        if (pickup_.due())
            spawnBonus();
        const double base = baseStep();
        const double rate = base / stepInterval();
        const double toMove = std::max(0.0, (base - accumulated_) / rate);
        double slice = std::min({left, toMove, pickup_.nextEvent(), remaining_});
        if (boostRemaining_ > 1e-9)
            slice = std::min(slice, boostRemaining_);
        accumulated_ += slice * rate;
        pickup_.advance(slice);
        boostRemaining_ = std::max(0.0, boostRemaining_ - slice);
        remaining_ = std::max(0.0, remaining_ - slice);
        left -= slice;
        if (remaining_ <= 1e-9) {
            remaining_ = 0;
            status_ = Status::Lost;
        } else if (accumulated_ + 1e-9 >= base) {
            accumulated_ = std::max(0.0, accumulated_ - base);
            move();
        }
    }
}
} // namespace arcade::bonus
