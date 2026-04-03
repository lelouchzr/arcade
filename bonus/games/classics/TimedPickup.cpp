#include "TimedPickup.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace arcade::bonus {
TimedPickup::TimedPickup(double delay, double lifetime) : delay_(delay), lifetime_(lifetime) {
    if (!std::isfinite(delay) || !std::isfinite(lifetime) || delay <= 0 || lifetime <= 0)
        throw std::invalid_argument("pickup timing must be finite and positive");
    reset();
}

void TimedPickup::reset() {
    clear();
}

void TimedPickup::advance(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::invalid_argument("invalid pickup delta time");
    if (active()) {
        const double overflow = std::max(0., seconds - remaining_);
        remaining_ = std::max(0., remaining_ - seconds);
        if (remaining_ <= 1e-9) {
            clear();
            cooldown_ = std::max(0., delay_ - overflow);
        }
    } else
        cooldown_ = std::max(0., cooldown_ - seconds);
}

void TimedPickup::place(Point point) {
    if (point.x < 0 || point.y < 0)
        throw std::invalid_argument("pickup position must be on the board");
    position_ = point;
    remaining_ = lifetime_;
    cooldown_ = 0;
}

void TimedPickup::clear() {
    position_ = {-1, -1};
    remaining_ = 0;
    cooldown_ = delay_;
}

void TimedPickup::defer(double seconds) {
    if (!std::isfinite(seconds) || seconds <= 0)
        throw std::invalid_argument("pickup retry delay must be positive");
    clear();
    cooldown_ = seconds;
}

bool TimedPickup::consume(Point point) {
    if (!active() || position_ != point)
        return false;
    clear();
    return true;
}

bool TimedPickup::active() const {
    return position_.x >= 0;
}

bool TimedPickup::due() const {
    return !active() && cooldown_ <= 1e-9;
}

Point TimedPickup::position() const {
    return position_;
}

double TimedPickup::remaining() const {
    return remaining_;
}

double TimedPickup::nextEvent() const {
    return active() ? remaining_ : cooldown_;
}
} // namespace arcade::bonus
