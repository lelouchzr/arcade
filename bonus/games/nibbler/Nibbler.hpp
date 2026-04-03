#pragma once
#include "../classics/TimedPickup.hpp"
#include "arcade/Api.hpp"
#include "original/OriginalLevels.hpp"
#include <deque>
#include <random>

namespace arcade::bonus {
namespace test {
struct NibblerBonusAccess;
}

class Nibbler final : public IGame {
  public:
    Nibbler();
    void reset(std::uint64_t seed) override;
    void update(double seconds, const std::vector<Input> &inputs) override;
    Snapshot snapshot() const override;

  private:
    friend struct test::NibblerBonusAccess;
    static constexpr int Width = 25, Height = 21;
    int width_ = Width, height_ = Height;
    std::uint64_t seed_ = 1, generation_ = 0;
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
    bool bonusEnabled_ = false, originalLevels_ = false;
    double boostRemaining_ = 0;
    TimedPickup pickup_;
    std::mt19937_64 random_;
    void updateClassic(double seconds);
    void updateBonus(double seconds, const std::vector<Input> &inputs);
    void spawnBonus();
    void buildOriginalLevel();
    [[nodiscard]] const OriginalLevel &originalLevel() const;
    [[nodiscard]] double baseStep() const;
    [[nodiscard]] double stepInterval() const;
};
} // namespace arcade::bonus
