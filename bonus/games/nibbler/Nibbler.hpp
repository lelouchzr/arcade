#pragma once
#include "arcade/Api.hpp"
#include <deque>

namespace arcade::bonus {
class Nibbler final : public IGame {
  public:
    Nibbler();
    void reset(std::uint64_t seed) override;
    void update(double seconds, const std::vector<Input> &inputs) override;
    Snapshot snapshot() const override;

  private:
    static constexpr int Width = 25, Height = 21;
    std::uint64_t seed_ = 1;
    std::vector<Cell> map_;
    std::deque<Point> body_;
    Point direction_{1, 0}, desired_{1, 0};
    int level_ = 1, food_ = 0;
    std::int64_t score_ = 0;
    double remaining_ = 90, accumulated_ = 0;
    Status status_ = Status::Running;
    bool wall(Point point) const;
    Point ahead(Point direction) const;
    void buildLevel();
    void move();
};
} // namespace arcade::bonus
