#pragma once
#include "arcade/Api.hpp"
#include <random>

namespace arcade {
class Minesweeper final : public IGame {
  public:
    Minesweeper();
    void reset(std::uint64_t seed) override;
    void update(double seconds, const std::vector<Input> &inputs) override;
    [[nodiscard]] Snapshot snapshot() const override;

  private:
    int width_ = 9, height_ = 9, mineCount_ = 10;
    // Difficulty selection is compiled only in the bonus configuration.
    [[maybe_unused]] int difficulty_ = 0;
    std::vector<bool> mines_, revealed_, flags_;
    std::vector<int> adjacent_;
    Point cursor_;
    std::mt19937_64 random_;
    std::uint64_t seed_ = 1, generation_ = 0;
    bool planted_ = false;
    double seconds_ = 180;
    std::int64_t score_ = 0;
    Status status_ = Status::Running;
    [[nodiscard]] int index(Point point) const;
    [[nodiscard]] std::vector<int> neighbours(int cell) const;
    void plant(int safeCell);
    void reveal(int cell);
};
} // namespace arcade
