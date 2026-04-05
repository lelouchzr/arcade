#include "BonusFeatures.hpp"
#include "TestSupport.hpp"
#include <algorithm>
#include <stdexcept>

namespace arcade::test {

const std::vector<bonus::CentipedeBody> &
CentipedeBonusAccess::bodies(const bonus::Centipede &game) {
    return game.bodies_;
}

Point CentipedeBonusAccess::second(const bonus::Centipede &game) {
    return game.secondPlayer_;
}

std::optional<Point> CentipedeBonusAccess::shot(const bonus::Centipede &game) {
    return game.shot_;
}

void CentipedeBonusAccess::field(bonus::Centipede &game, std::vector<bonus::CentipedeBody> bodies) {
    game.bodies_ = std::move(bodies);
    game.obstacles_.assign(game.board_.cells().size(), 0);
    game.movementClock_ = game.shotClock_ = game.spawnClock_ = game.accumulated_ = 0;
    game.shot_.reset();
}

void CentipedeBonusAccess::projectile(bonus::Centipede &game, Point point) {
    game.shot_ = point;
}

void centipedeBonusTests() {
    using namespace bonus;
    const auto bonusHud = Centipede(CentipedeRules{.16, .035, 8, 45, true}).snapshot();
    require(bonusHud.information.size() <= 7, "Centipede bonus HUD exceeds seven lines");
    for (const auto &line : bonusHud.information)
        require(line.size() <= 23, "Centipede bonus HUD contains a truncated control line");
    const CentipedeRules rules{.16, .035, 8, 0, true};
    Centipede game(rules);
    const auto initial = CentipedeBonusAccess::bodies(game);
    require(initial.size() == 3 && initial[0].segments.size() != initial[1].segments.size() &&
                initial[1].segments.size() != initial[2].segments.size(),
            "Centipede bonus wave lacks simultaneous bodies of different lengths");
    game.update(.1, {});
    const auto &advanced = CentipedeBonusAccess::bodies(game);
    require(advanced[0].segments.front() == initial[0].segments.front() &&
                advanced[1].segments.front() != initial[1].segments.front() &&
                advanced[2].segments.front() == initial[2].segments.front(),
            "Centipede racer did not move faster than other types");
    game.reset(1);
    const auto firstPlayer = locate(game.snapshot(), Cell::Player);
    game.update(0, {{Action::PlayerTwoUp}, {Action::PlayerTwoRight}});
    require(CentipedeBonusAccess::second(game) == Point{7, 19} &&
                locate(game.snapshot(), Cell::Player) == firstPlayer,
            "second Centipede pilot is not controlled independently");
    for (int index = 0; index < 30; ++index)
        game.update(0, {{Action::PlayerTwoUp}, {Action::PlayerTwoLeft}});
    require(CentipedeBonusAccess::second(game) == Point{0, 17},
            "second Centipede pilot escaped movement limits");
    game.update(0, {{Action::PlayerTwoFire}, {Action::Primary}, {Action::PlayerTwoFire}});
    require(CentipedeBonusAccess::shot(game) == std::optional<Point>{{0, 17}},
            "two Centipede pilots bypassed shared one-projectile limit");
    game.reset(1);
    CentipedeBody weaver{{{12, 5}, {11, 5}, {10, 5}}, 1, false, false, CentipedeType::Weaver, 0, 6};
    CentipedeBonusAccess::field(game, {weaver});
    game.update(.21, {});
    require(CentipedeBonusAccess::bodies(game)[0].segments.front() == Point{12, 6} &&
                CentipedeBonusAccess::bodies(game)[0].direction == -1,
            "Centipede weaver pattern failed to descend and reverse in open space");
    game.reset(1);
    CentipedeBody racer{{{10, 6}, {9, 6}, {8, 6}, {7, 6}}, 1, false, false, CentipedeType::Racer};
    CentipedeBonusAccess::field(game, {racer});
    CentipedeBonusAccess::projectile(game, {9, 7});
    game.update(.035, {});
    require(CentipedeBonusAccess::bodies(game).size() == 2 &&
                CentipedeBonusAccess::bodies(game)[0].type == CentipedeType::Racer &&
                CentipedeBonusAccess::bodies(game)[1].type == CentipedeType::Racer,
            "Centipede split lost the parent body type");
    game.update(.065, {});
    require(CentipedeBonusAccess::bodies(game)[1].segments.front().y == 7,
            "split bonus racer did not descend at its faster movement period");
    game.reset(1);
    CentipedeBody pursuer{{{5, 20}, {4, 20}}, 1, false, false};
    CentipedeBonusAccess::field(game, {pursuer});
    game.update(.16, {});
    require(game.snapshot().status == Status::Lost,
            "Centipede contact with the second pilot did not end the game");
    Centipede arrivals(CentipedeRules{10000, .035, 8, 0, true});
    arrivals.update(5, {});
    require(CentipedeBonusAccess::bodies(arrivals).size() == 4,
            "new Centipede did not arrive while an existing wave was still alive");
    Centipede deterministic(rules), partitioned(rules);
    deterministic.reset(73);
    partitioned.reset(73);
    deterministic.update(.3, {{Action::PlayerTwoUp}});
    partitioned.update(.1, {{Action::PlayerTwoUp}});
    partitioned.update(.2, {});
    require(deterministic.snapshot().cells == partitioned.snapshot().cells,
            "Centipede bonus movement depends on update partition");
    arrivals.update(0, {{Action::Difficulty}});
    require(CentipedeBonusAccess::bodies(arrivals).size() == 1 &&
                count(arrivals.snapshot(), Cell::SecondPlayer) == 0,
            "Centipede classic mode retained multiplayer or extra bodies");
    arrivals.update(0, {{Action::PlayerTwoUp}, {Action::PlayerTwoFire}});
    require(!CentipedeBonusAccess::shot(arrivals), "classic Centipede accepted second pilot fire");
    bool rejected = false;
    try {
        game.update(-.01, {});
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    require(rejected, "Centipede bonus accepted negative delta");
    require(game.snapshot().information.size() <= 7, "Centipede bonus HUD exceeds seven lines");
    for (const auto &line : game.snapshot().information)
        require(line.size() <= 23, "Centipede HUD line exceeds visible width");
}

} // namespace arcade::test
