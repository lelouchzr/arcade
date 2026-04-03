#include "Snake.hpp"
#include <algorithm>
#include <cmath>

namespace arcade {
double Snake::stepInterval() const {
    const double interval = std::max(.06, Step - .004 * static_cast<double>(body_.size() - 4));
    return boostRemaining_ > 1e-9 ? interval * .5 : interval;
}

void Snake::spawnBonus() {
    std::vector<Point> candidates;
    for (int y = 1; y < Height - 1; ++y)
        for (int x = 1; x < Width - 1; ++x) {
            Point point{x, y};
            if (point != food_ && std::find(body_.begin(), body_.end(), point) == body_.end())
                candidates.push_back(point);
        }
    if (candidates.empty())
        pickup_.defer();
    else
        pickup_.place(candidates[std::uniform_int_distribution<std::size_t>(0, candidates.size() -
                                                                                   1)(random_)]);
}

void Snake::updateBonus(double seconds, const std::vector<Input> &inputs) {
    for (const auto &input : inputs)
        if (input.action == Action::Primary)
            boostRemaining_ = .6;
    double left = seconds;
    while (left > 1e-12 && status_ == Status::Running) {
        if (pickup_.due())
            spawnBonus();
        const double rate = Step / stepInterval();
        const double toMove = std::max(0., (Step - accumulated_) / rate);
        double slice = std::min({left, toMove, pickup_.nextEvent()});
        if (boostRemaining_ > 1e-9)
            slice = std::min(slice, boostRemaining_);
        accumulated_ += slice * rate;
        pickup_.advance(slice);
        boostRemaining_ = std::max(0., boostRemaining_ - slice);
        left = std::max(0., left - slice);
        if (accumulated_ + 1e-9 >= Step) {
            accumulated_ = std::max(0., accumulated_ - Step);
            step();
        }
    }
}
} // namespace arcade
