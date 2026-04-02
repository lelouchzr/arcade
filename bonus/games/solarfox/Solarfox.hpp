#pragma once

#include "../Grid.hpp"
#include <random>

namespace arcade::test {
struct SolarfoxAccess;
}

namespace arcade::bonus {

struct SolarShot {
    Point position;
    Point direction;
    int remaining = 2;
};

struct EnemyGun {
    Point position;
    Point direction;
    double clock = 0;
};

struct SolarfoxRules {
    double movementPeriod = 0.18;
    double enemyShotPeriod = 0.09;
    double gunPeriod = 1.1;
};

class Solarfox final : public IGame {
  public:
    explicit Solarfox(SolarfoxRules rules = {});
    void reset(std::uint64_t seed) override;
    void update(double seconds, const std::vector<Input> &inputs) override;
    [[nodiscard]] Snapshot snapshot() const override;

  private:
    friend struct arcade::test::SolarfoxAccess;
    static constexpr int Width = 25;
    static constexpr int Height = 21;
    static constexpr int Margin = 3;
    Grid board_{Width, Height};
    SolarfoxRules rules_;
    std::vector<SolarShot> shots_;
    std::vector<SolarShot> enemyShots_;
    std::vector<EnemyGun> guns_;
    Point player_{12, 10};
    Point heading_{1, 0};
    Point desired_{1, 0};
    std::uint64_t generation_ = 0;
    std::int64_t score_ = 0;
    int level_ = 1;
    int powerups_ = 0;
    double accumulated_ = 0;
    double movementClock_ = 0;
    double shotClock_ = 0;
    double enemyShotClock_ = 0;
    double boost_ = 0;
    bool turnQueued_ = false;
    Status status_ = Status::Running;
    void buildLevel();
    void tick(double seconds);
    void movePlayer();
    void moveShots(bool enemy);
    void intercept();
    void collisions();
    [[nodiscard]] bool walkable(Point point) const;
};

} // namespace arcade::bonus
