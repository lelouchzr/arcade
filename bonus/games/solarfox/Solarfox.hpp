#pragma once

#include "../Grid.hpp"
#include <random>

namespace arcade::test {
struct SolarfoxAccess;
struct SolarfoxBonusAccess;
} // namespace arcade::test

namespace arcade::bonus {

enum class SolarShotType { Standard, Slow, Fast, Spread };

struct SolarShot {
    Point position;
    Point direction;
    int remaining = 2;
    SolarShotType type = SolarShotType::Standard;
    double clock = 0;
};

struct EnemyGun {
    Point position;
    Point direction;
    double clock = 0;
    SolarShotType type = SolarShotType::Standard;
    int patrol = 1;
};

struct SolarfoxRules {
    double movementPeriod = 0.18;
    double enemyShotPeriod = 0.09;
    double gunPeriod = 1.1;
    bool bonus = false;
};

class Solarfox final : public IGame {
  public:
    explicit Solarfox(SolarfoxRules rules = {});
    void reset(std::uint64_t seed) override;
    void update(double seconds, const std::vector<Input> &inputs) override;
    [[nodiscard]] Snapshot snapshot() const override;

  private:
    friend struct arcade::test::SolarfoxAccess;
    friend struct arcade::test::SolarfoxBonusAccess;
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
    std::uint64_t seed_ = 1;
    double patrolClock_ = 0;
    Status status_ = Status::Running;
    void buildLevel();
    void tick(double seconds);
    void movePlayer();
    void moveShots(bool enemy);
    void advanceShot(SolarShot &shot);
    void collectShots();
    void initializeBonuses();
    void fireGun(const EnemyGun &gun);
    void moveGuns();
    void moveBonusShots(double seconds);
    void intercept();
    void collisions();
    [[nodiscard]] bool walkable(Point point) const;
};

} // namespace arcade::bonus
