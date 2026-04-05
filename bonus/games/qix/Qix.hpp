#pragma once

#include "../Grid.hpp"
#include <deque>
#include <random>

namespace arcade::test {
struct QixAccess;
struct QixBonusAccess;
} // namespace arcade::test

namespace arcade::bonus {

struct Spark {
    Point position;
    Point previous{-1, -1};
};

enum class QixPower { Shield, Slow };

struct QixPickup {
    Point position;
    QixPower effect = QixPower::Shield;
    double remaining = 8;
};

struct QixRules {
    double playerPeriod = 0.08;
    double monsterPeriod = 0.14;
    double sparkPeriod = 0.18;
    double ignitionDelay = 0.6;
    double firePeriod = 0.07;
    bool bonus = false;
};

class Qix final : public IGame {
  public:
    explicit Qix(QixRules rules = {});
    void reset(std::uint64_t seed) override;
    void update(double seconds, const std::vector<Input> &inputs) override;
    [[nodiscard]] Snapshot snapshot() const override;

  private:
    friend struct arcade::test::QixAccess;
    friend struct arcade::test::QixBonusAccess;
    static constexpr int Width = 25;
    static constexpr int Height = 21;
    Grid board_{Width, Height};
    QixRules rules_;
    std::vector<bool> tracks_;
    std::vector<Point> trail_;
    std::deque<Point> monster_;
    std::vector<Spark> sparks_;
    std::mt19937_64 random_;
    std::vector<QixPickup> pickups_;
    std::uint64_t seed_ = 1;
    double shield_ = 0;
    double slow_ = 0;
    double pickupClock_ = 0;
    int originalArea_ = (Width - 2) * (Height - 2);
    Point player_{0, 10};
    Point anchor_{0, 10};
    Point heading_;
    std::uint64_t generation_ = 0;
    std::int64_t score_ = 0;
    double accumulated_ = 0;
    double playerClock_ = 0;
    double monsterClock_ = 0;
    double sparkClock_ = 0;
    double idle_ = 0;
    double fireClock_ = 0;
    std::size_t fireIndex_ = 0;
    bool burning_ = false;
    Status status_ = Status::Running;
    void initializeBonuses();
    void tickBonuses(double seconds);
    void collectBonuses();
    void spawnPickup();
    void tick(double seconds);
    void movePlayer();
    void moveMonster();
    void moveSparks();
    void capture();
    void collisions();
    void lose();
    [[nodiscard]] std::vector<int> trackDistances() const;
};

} // namespace arcade::bonus
