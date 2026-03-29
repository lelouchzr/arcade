#pragma once
#include "arcade/Api.hpp"
#include <deque>
#include <random>

namespace arcade {
class Snake final : public IGame {
  public:
    Snake();
    void reset(std::uint64_t seed) override;
    void update(double seconds, const std::vector<Input> &inputs) override;
    [[nodiscard]] Snapshot snapshot() const override;

  private:
    static constexpr int Width = 25, Height = 21;
    static constexpr double Step = 0.14;
    std::deque<Point> body_;
    Point direction_{1, 0}, pending_{1, 0}, food_;
    bool turnQueued_ = false;
    double accumulated_ = 0;
    std::mt19937_64 random_;
    Status status_ = Status::Running;
    std::int64_t score_ = 0;
    void spawnFood();
    void step();
};
} // namespace arcade
