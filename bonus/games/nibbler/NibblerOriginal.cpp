#include "Nibbler.hpp"
#include "original/OriginalLevels.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace arcade::bonus {
const OriginalLevel &Nibbler::originalLevel() const {
    return OriginalLevels::level(level_);
}

void Nibbler::buildOriginalLevel() {
    const auto &level = originalLevel();
    width_ = level.width;
    height_ = level.height;
    map_.assign(static_cast<std::size_t>(width_ * height_), Cell::Wall);
    std::vector<Point> segments;
    body_.clear();
    for (int y = 0; y < level.height; ++y)
        for (int x = 0; x < level.width; ++x) {
            const char cell = level.rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            const Point point{x, y};
            auto &target = map_[static_cast<std::size_t>(point.y * width_ + point.x)];
            target = cell == '#' ? Cell::Wall : cell == '.' ? Cell::Food : Cell::Empty;
            if (cell == 'H')
                body_.push_front(point);
            else if (cell == 'o')
                segments.push_back(point);
        }
    if (body_.size() != 1 || segments.size() != 3)
        throw std::runtime_error("Nibbler reconstruction must start with four snake cells");
    while (!segments.empty()) {
        const Point tail = body_.back();
        const auto next = std::find_if(segments.begin(), segments.end(), [&](Point point) {
            return std::abs(point.x - tail.x) + std::abs(point.y - tail.y) == 1;
        });
        if (next == segments.end())
            throw std::runtime_error("disconnected Nibbler reconstruction snake");
        body_.push_back(*next);
        segments.erase(next);
    }
    direction_ = desired_ = {body_[0].x - body_[1].x, body_[0].y - body_[1].y};
    remaining_ = 90;
    accumulated_ = 0;
    status_ = Status::Running;
    food_ = static_cast<int>(std::count(map_.begin(), map_.end(), Cell::Food));
}
} // namespace arcade::bonus
