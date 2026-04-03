#pragma once
#include "arcade/Api.hpp"

namespace arcade::bonus {
class TimedPickup {
  public:
    explicit TimedPickup(double delay = 10, double lifetime = 5);
    void reset();
    void advance(double seconds);
    void place(Point point);
    void clear();
    void defer(double seconds = 1);
    bool consume(Point point);
    [[nodiscard]] bool active() const;
    [[nodiscard]] bool due() const;
    [[nodiscard]] Point position() const;
    [[nodiscard]] double remaining() const;
    [[nodiscard]] double nextEvent() const;

  private:
    double delay_, lifetime_, cooldown_ = 0, remaining_ = 0;
    Point position_{-1, -1};
};
} // namespace arcade::bonus
