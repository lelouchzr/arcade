#include "Pacman.hpp"
#include <algorithm>
#include <limits>

namespace arcade::bonus {

std::vector<int> Pacman::distances(Point target, bool homeAllowed) const {
    const auto &cells = board_.cells();
    const auto width = static_cast<std::size_t>(board_.width());
    const auto size = cells.size();
    std::vector<int> result(size, -1);
    target = board_.wrap(target);
    if (!walkable(target, homeAllowed))
        target = player_;
    const auto start = board_.index(target);
    std::vector<std::size_t> pending;
    pending.reserve(size);
    pending.push_back(start);
    result[start] = 0;
    for (std::size_t head = 0; head < pending.size(); ++head) {
        const auto index = pending[head];
        const auto column = index % width;
        const std::array<std::size_t, 4> neighbours{
            column + 1 < width ? index + 1 : index + 1 - width,
            index + width < size ? index + width : index + width - size,
            column > 0 ? index - 1 : index + width - 1,
            index >= width ? index - width : index + size - width};
        for (const auto next : neighbours) {
            // Geometry wraps by index, but eligibility always reads the current board.
            if (result[next] >= 0 || cells[next] == Cell::Wall ||
                (!homeAllowed && cells[next] == Cell::Home))
                continue;
            result[next] = result[index] + 1;
            pending.push_back(next);
        }
    }
    return result;
}

Point Pacman::ghostTarget(std::size_t index) const {
    if (index == 1) {
        const Point ahead{player_.x + heading_.x * 4, player_.y + heading_.y * 4};
        return walkable(ahead, false) ? board_.wrap(ahead) : player_;
    }
    if (index == 2) {
        const Point mirror{player_.x * 2 - ghosts_[0].position.x,
                           player_.y * 2 - ghosts_[0].position.y};
        return walkable(mirror, false) ? board_.wrap(mirror) : player_;
    }
    if (index == 3 && manhattan(ghosts_[index].position, player_) < 6)
        return {1, 19};
    return player_;
}

void Pacman::moveGhost(std::size_t index) {
    auto &ghost = ghosts_[index];
    const bool eyes = ghost.mode == GhostMode::Eyes;
    const bool inside = board_.at(ghost.position) == Cell::Home;
    if (eyes && ghost.position == Point{12, 9}) {
        ghost.mode = GhostMode::Healing;
        ghost.healing = rules_.healingDuration;
        return;
    }
    const bool homeAllowed = eyes || inside;
    const Point target = eyes ? Point{12, 9} : inside ? Point{12, 7} : ghostTarget(index);
    const bool fleeing = power_ > 0 && !eyes && !inside;
    const auto distance = distances(fleeing ? player_ : target, homeAllowed);
    std::vector<Point> options;
    for (const auto d : Directions)
        if (walkable(add(ghost.position, d), homeAllowed))
            options.push_back(d);
    if (options.size() > 1)
        std::erase(options, Point{-ghost.direction.x, -ghost.direction.y});
    if (options.empty())
        return;
    Point best = options.front();
    int bestValue = fleeing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    // A seeded ordering breaks equal-distance choices without frame-rate dependence.
    std::rotate(options.begin(),
                options.begin() + static_cast<std::ptrdiff_t>((seed_ + index) % options.size()),
                options.end());
    for (const auto d : options) {
        const auto next = board_.wrap(add(ghost.position, d));
        const int value = distance[board_.index(next)];
        if (value < 0)
            continue;
        if ((fleeing && value > bestValue) || (!fleeing && value < bestValue)) {
            bestValue = value;
            best = d;
        }
    }
    ghost.direction = best;
    ghost.position = board_.wrap(add(ghost.position, best));
    if (eyes && ghost.position == Point{12, 9}) {
        ghost.mode = GhostMode::Healing;
        ghost.healing = rules_.healingDuration;
    }
}

} // namespace arcade::bonus
