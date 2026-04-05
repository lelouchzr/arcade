#pragma once

#include "../Grid.hpp"
#include <optional>
#include <random>

namespace arcade::test {
struct CentipedeAccess;
struct CentipedeBonusAccess;
} // namespace arcade::test

namespace arcade::bonus {

enum class CentipedeType { Standard, Racer, Weaver };

struct CentipedeBody {
    std::vector<Point> segments;
    int direction = 1;
    bool bottomCharged = false;
    bool pendingSplitTurn = false;
    CentipedeType type = CentipedeType::Standard;
    double clock = 0;
    std::uint64_t steps = 0;
};

struct CentipedeRules {
    double movementPeriod = 0.16;
    double shotPeriod = 0.035;
    int initialLength = 8;
    int obstacles = 45;
    bool bonus = false;
};

class Centipede final : public IGame {
  public:
    explicit Centipede(CentipedeRules rules = {});
    void reset(std::uint64_t seed) override;
    void update(double seconds, const std::vector<Input> &inputs) override;
    [[nodiscard]] Snapshot snapshot() const override;

  private:
    friend struct arcade::test::CentipedeAccess;
    friend struct arcade::test::CentipedeBonusAccess;
    static constexpr int Width = 25;
    static constexpr int Height = 21;
    static constexpr int PlayerTop = 17;
    Grid board_{Width, Height};
    CentipedeRules rules_;
    std::vector<int> obstacles_;
    std::vector<CentipedeBody> bodies_;
    std::optional<Point> shot_;
    Point player_{12, 20};
    Point secondPlayer_{6, 20};
    std::uint64_t seed_ = 1;
    double spawnClock_ = 0;
    int spawnSequence_ = 0;
    std::mt19937_64 random_;
    std::uint64_t generation_ = 0;
    std::int64_t score_ = 0;
    int survived_ = 0;
    int level_ = 1;
    double accumulated_ = 0;
    double movementClock_ = 0;
    double shotClock_ = 0;
    Status status_ = Status::Running;
    void buildField();
    void spawnWave();
    void tick(double seconds);
    void moveBodies();
    void moveBody(CentipedeBody &body);
    void resolveBodies();
    void spawnBonusBody(CentipedeType type);
    void tickBonusBodies(double seconds);
    void moveSecondPlayer(Action action);
    void moveShot();
    void hitShot();
    void split(std::size_t body, std::size_t segment);
    void collisions();
    void finishWave();
};

} // namespace arcade::bonus
