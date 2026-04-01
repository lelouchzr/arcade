#include "Qix.hpp"
#include <algorithm>
#include <limits>
#include <queue>

namespace arcade::bonus {

void Qix::capture() {
    for (auto point : trail_)
        board_.at(point) = Cell::Boundary;
    trail_.clear();
    burning_ = false;
    std::vector<bool> reachable(board_.cells().size(), false);
    std::queue<Point> pending;
    // Every segment is a seed; a cut must never swallow part of the monster.
    for (auto point : monster_) {
        reachable[board_.index(point)] = true;
        pending.push(point);
    }
    while (!pending.empty()) {
        const auto point = pending.front();
        pending.pop();
        for (auto d : Directions) {
            const auto next = add(point, d);
            if (!board_.contains(next) || reachable[board_.index(next)] ||
                board_.at(next) != Cell::Empty)
                continue;
            reachable[board_.index(next)] = true;
            pending.push(next);
        }
    }
    int captured = 0;
    for (std::size_t i = 0; i < reachable.size(); ++i)
        if (board_.at(board_.point(i)) == Cell::Empty && !reachable[i]) {
            board_.at(board_.point(i)) = Cell::Claimed;
            ++captured;
        }
    const auto previous = board_.cells();
    for (std::size_t i = 0; i < previous.size(); ++i) {
        if (previous[i] == Cell::Empty)
            continue;
        const auto point = board_.point(i);
        bool adjacent = false;
        for (auto d : Directions) {
            const auto neighbour = add(point, d);
            if (board_.contains(neighbour) && previous[board_.index(neighbour)] == Cell::Empty)
                adjacent = true;
        }
        board_.at(point) = adjacent ? Cell::Boundary : Cell::Claimed;
        if (adjacent)
            tracks_[i] = true;
    }
    score_ += captured * 100;
    if (board_.at(player_) != Cell::Boundary) {
        int best = std::numeric_limits<int>::max();
        Point closest = player_;
        for (std::size_t i = 0; i < previous.size(); ++i)
            if (board_.at(board_.point(i)) == Cell::Boundary &&
                manhattan(player_, board_.point(i)) < best) {
                closest = board_.point(i);
                best = manhattan(player_, closest);
            }
        player_ = closest;
    }
    anchor_ = player_;
    const int remaining =
        static_cast<int>(std::count(board_.cells().begin(), board_.cells().end(), Cell::Empty));
    if (remaining * 4 < (Width - 2) * (Height - 2))
        status_ = Status::Won;
}

std::vector<int> Qix::trackDistances() const {
    std::vector<int> distance(tracks_.size(), -1);
    std::queue<Point> pending;
    distance[board_.index(player_)] = 0;
    pending.push(player_);
    while (!pending.empty()) {
        const auto point = pending.front();
        pending.pop();
        for (auto d : Directions) {
            const auto next = add(point, d);
            if (!board_.contains(next) || !tracks_[board_.index(next)] ||
                distance[board_.index(next)] >= 0)
                continue;
            distance[board_.index(next)] = distance[board_.index(point)] + 1;
            pending.push(next);
        }
    }
    return distance;
}

void Qix::moveSparks() {
    const auto distance = trackDistances();
    for (auto &spark : sparks_) {
        Point best = spark.position;
        int bestDistance = std::numeric_limits<int>::max();
        for (auto d : Directions) {
            const auto next = add(spark.position, d);
            if (!board_.contains(next) || !tracks_[board_.index(next)] || next == spark.previous)
                continue;
            const int value =
                distance[board_.index(next)] < 0 ? 100000 : distance[board_.index(next)];
            if (value < bestDistance) {
                best = next;
                bestDistance = value;
            }
        }
        if (best != spark.position) {
            spark.previous = spark.position;
            spark.position = best;
        }
    }
    collisions();
}

} // namespace arcade::bonus
