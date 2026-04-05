#include "Centipede.hpp"
#include <algorithm>

namespace arcade::bonus {

void Centipede::moveBody(CentipedeBody &body) {
    auto next = add(body.segments.front(), {body.direction, 0});
    const bool weave =
        rules_.bonus && body.type == CentipedeType::Weaver && body.steps > 0 && body.steps % 6 == 0;
    if (weave || body.pendingSplitTurn || !board_.contains(next) ||
        obstacles_[board_.index(next)] > 0) {
        const int reversed = -body.direction;
        next = {body.segments.front().x, body.segments.front().y + 1};
        if (board_.contains(next) && obstacles_[board_.index(next)] > 0) {
            next.x += reversed;
            if (!board_.contains(next) || obstacles_[board_.index(next)] > 0)
                return;
        }
        body.direction = reversed;
        body.pendingSplitTurn = false;
    }
    for (std::size_t i = body.segments.size() - 1; i > 0; --i)
        body.segments[i] = body.segments[i - 1];
    body.segments[0] = next;
    ++body.steps;
    if (next.y >= Height - 1 && !body.bottomCharged) {
        score_ = std::max<std::int64_t>(0, score_ - 100);
        body.bottomCharged = true;
    }
}

void Centipede::resolveBodies() {
    collisions();
    std::erase_if(bodies_,
                  [](const CentipedeBody &body) { return body.segments.front().y >= Height; });
    hitShot();
}

void Centipede::moveBodies() {
    for (auto &body : bodies_)
        moveBody(body);
    resolveBodies();
}

void Centipede::split(std::size_t bodyIndex, std::size_t segmentIndex) {
    const auto original = bodies_.at(bodyIndex);
    const auto hit = original.segments.at(segmentIndex);
    obstacles_[board_.index(hit)] = 5;
    std::vector<CentipedeBody> replacements;
    if (segmentIndex > 0) {
        CentipedeBody head = original;
        head.segments.resize(segmentIndex);
        replacements.push_back(std::move(head));
    }
    if (segmentIndex + 1 < original.segments.size()) {
        CentipedeBody tail;
        tail.type = original.type;
        tail.clock = original.clock;
        tail.steps = original.steps;
        tail.segments.assign(original.segments.begin() +
                                 static_cast<std::ptrdiff_t>(segmentIndex + 1),
                             original.segments.end());
        const int horizontal = hit.x - tail.segments.front().x;
        tail.direction = horizontal == 0 ? original.direction : horizontal > 0 ? 1 : -1;
        tail.bottomCharged = tail.segments.front().y >= Height - 1;
        tail.pendingSplitTurn = true;
        replacements.push_back(std::move(tail));
    }
    bodies_.erase(bodies_.begin() + static_cast<std::ptrdiff_t>(bodyIndex));
    bodies_.insert(bodies_.end(), replacements.begin(), replacements.end());
    score_ += 100;
}

void Centipede::hitShot() {
    if (!shot_ || !board_.contains(*shot_))
        return;
    auto &obstacle = obstacles_[board_.index(*shot_)];
    if (obstacle > 0) {
        --obstacle;
        if (obstacle == 0)
            score_ += 5;
        shot_.reset();
        return;
    }
    for (std::size_t b = 0; b < bodies_.size(); ++b) {
        const auto &segments = bodies_[b].segments;
        const auto found = std::find(segments.begin(), segments.end(), *shot_);
        if (found != segments.end()) {
            const auto segment = static_cast<std::size_t>(found - segments.begin());
            split(b, segment);
            shot_.reset();
            return;
        }
    }
}

void Centipede::moveShot() {
    if (!shot_)
        return;
    --shot_->y;
    if (shot_->y < 0) {
        shot_.reset();
        return;
    }
    hitShot();
}

} // namespace arcade::bonus
