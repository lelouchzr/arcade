#pragma once

#include "../Grid.hpp"
#include <array>

namespace arcade::test {
struct PacmanAccess;
struct PacmanBonusAccess;
} // namespace arcade::test

namespace arcade::bonus {

enum class GhostMode { Caged, Active, Eyes, Healing };

struct PacmanJump {
    double elapsed = -1;
    double cooldown = 0;
};

struct Ghost {
    Point position;
    Point direction{0, 0};
    GhostMode mode = GhostMode::Caged;
    double clock = 0;
    double healing = 0;
    PacmanJump jump{};
    double jumpClock = 0;
};

struct PacmanRules {
    double releaseDelay = 10;
    double powerDuration = 10;
    double healingDuration = 2;
    double playerPeriod = 0.14;
    double ghostPeriod = 0.20;
    bool bonusEnabled = false;
    double fruitDelay = 12;
    double fruitDuration = 8;
    double fruitInterval = 20;
    double jumpDuration = 0.8;
    double jumpCooldown = 1.4;
    double ghostJumpInterval = 3;
};

class Pacman final : public IGame {
  public:
    explicit Pacman(PacmanRules rules = {});
    void reset(std::uint64_t seed) override;
    void update(double seconds, const std::vector<Input> &inputs) override;
    [[nodiscard]] Snapshot snapshot() const override;

  private:
    friend struct arcade::test::PacmanAccess;
    friend struct arcade::test::PacmanBonusAccess;
    static constexpr int Width = 25;
    static constexpr int Height = 21;
    Grid board_{Width, Height};
    PacmanRules rules_;
    std::array<Ghost, 4> ghosts_;
    Point player_{12, 12};
    Point heading_{-1, 0};
    Point desired_{-1, 0};
    std::uint64_t generation_ = 0;
    std::uint64_t seed_ = 0;
    std::int64_t score_ = 0;
    int level_ = 1;
    int pellets_ = 0;
    int combo_ = 0;
    double elapsed_ = 0;
    double power_ = 0;
    double accumulated_ = 0;
    double playerClock_ = 0;
    Status status_ = Status::Running;
    PacmanJump jump_;
    double bonusElapsed_ = 0;
    double fruitClock_ = 0;
    bool fruitActive_ = false;
    bool cameraEnabled_ = true;
    void buildLevel();
    void bonusTick(double seconds);
    void bonusInput(Action action);
    void collectFruit();
    void advanceJump(PacmanJump &jump, double seconds) const;
    bool startJump(PacmanJump &jump) const;
    [[nodiscard]] double elevation(const PacmanJump &jump) const;
    [[nodiscard]] double acceleration() const;
    [[nodiscard]] bool sameHeight(const Ghost &ghost) const;
    void tick(double seconds);
    void movePlayer();
    void consume();
    void collisions();
    void moveGhost(std::size_t index);
    [[nodiscard]] bool walkable(Point point, bool homeAllowed) const;
    [[nodiscard]] std::vector<int> distances(Point target, bool homeAllowed) const;
    [[nodiscard]] Point ghostTarget(std::size_t index) const;
};

} // namespace arcade::bonus
