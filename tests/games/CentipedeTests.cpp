#include "Fixtures.hpp"
#include "TestSupport.hpp"
#include <algorithm>

namespace arcade::test {

void CentipedeAccess::scenario(bonus::Centipede &g, std::vector<bonus::CentipedeBody> bodies,
                               Point player) {
    g.bodies_ = std::move(bodies);
    g.player_ = player;
    g.shot_.reset();
    g.obstacles_.assign(g.board_.cells().size(), 0);
    g.movementClock_ = g.shotClock_ = g.accumulated_ = 0;
}

void CentipedeAccess::obstacle(bonus::Centipede &g, Point p, int health) {
    g.obstacles_[g.board_.index(p)] = health;
}

int CentipedeAccess::health(const bonus::Centipede &g, Point p) {
    return g.obstacles_[g.board_.index(p)];
}

void CentipedeAccess::shot(bonus::Centipede &g, Point p) {
    g.shot_ = p;
    g.shotClock_ = 0;
}

const std::vector<bonus::CentipedeBody> &CentipedeAccess::bodies(const bonus::Centipede &g) {
    return g.bodies_;
}

void CentipedeAccess::score(bonus::Centipede &g, std::int64_t value) {
    g.score_ = value;
}

void CentipedeAccess::survived(bonus::Centipede &g, int value) {
    g.survived_ = value;
}

int CentipedeAccess::level(const bonus::Centipede &g) {
    return g.level_;
}

void centipedeTests() {
    using namespace bonus;
    Centipede game;
    require(count(game.snapshot(), Cell::Obstacle) == 45, "random obstacles missing");
    for (int i = 0; i < 30; ++i)
        game.update(0, {{Action::Up}});
    require(locate(game.snapshot(), Cell::Player).y == 17, "player escaped upper 20 percent limit");
    for (int i = 0; i < 30; ++i)
        game.update(0, {{Action::Left}});
    require(locate(game.snapshot(), Cell::Player).x == 0, "player cannot move full width");
    game.update(0, {{Action::Primary}, {Action::Primary}, {Action::Primary}});
    require(count(game.snapshot(), Cell::Projectile) <= 1, "more than one projectile spawned");
    Centipede target(CentipedeRules{10000, .035, 8, 0});
    CentipedeAccess::obstacle(target, {12, 19}, 5);
    for (int hit = 0; hit < 5; ++hit) {
        target.update(.035, {{Action::Primary}});
        require(CentipedeAccess::health(target, {12, 19}) == 4 - hit,
                "mushroom durability is not five hits");
    }
    require(target.snapshot().score == 5, "destroyed obstacle score missing");
    game.reset(1);
    CentipedeAccess::scenario(
        game, {CentipedeBody{{{10, 6}, {9, 6}, {8, 6}, {7, 6}, {6, 6}}, 1, false, false}}, {8, 17});
    CentipedeAccess::shot(game, {8, 7});
    game.update(.035, {});
    require(CentipedeAccess::bodies(game).size() == 2, "body hit did not split centipede");
    require(CentipedeAccess::health(game, {8, 6}) == 5, "hit segment did not become obstacle");
    require(game.snapshot().score == 100, "segment score missing");
    game.update(.125, {});
    const auto &parts = CentipedeAccess::bodies(game);
    require(parts[0].segments.front() == Point{11, 6}, "head fragment did not keep moving");
    require(parts[1].segments.front() == Point{7, 7} && parts[1].direction == -1,
            "tail did not descend and reverse at new obstacle");
    game.reset(1);
    CentipedeAccess::scenario(
        game, {CentipedeBody{{{10, 6}, {9, 6}, {9, 5}, {9, 4}}, 1, false, false}}, {12, 17});
    CentipedeAccess::shot(game, {9, 7});
    game.update(.035, {});
    game.update(.125, {});
    const auto &vertical = CentipedeAccess::bodies(game);
    require(vertical.size() == 2 && vertical[1].segments.front().y == 6 &&
                vertical[1].direction == -1,
            "vertical split tail did not descend and reverse");
    require(CentipedeAccess::health(game, vertical[1].segments.front()) == 0,
            "vertical split entered its new obstacle");
    game.reset(1);
    CentipedeAccess::scenario(game, {CentipedeBody{{{24, 19}, {23, 19}}, 1, false, false}},
                              {0, 17});
    CentipedeAccess::score(game, 250);
    game.update(.16, {});
    require(game.snapshot().score == 150, "bottom escape did not deduct score");
    game.reset(1);
    CentipedeAccess::scenario(game, {CentipedeBody{{{9, 17}, {8, 17}}, 1, false, false}}, {10, 17});
    game.update(.16, {});
    require(game.snapshot().status == Status::Lost, "centipede contact did not lose");
    game.reset(1);
    for (int wave = 0; wave < 20; ++wave) {
        // Arrange the end of a genuine wave, then let its last fragment leave the field.
        CentipedeAccess::scenario(game, {CentipedeBody{{{24, 20}}, 1, true, false}}, {0, 17});
        game.update(.16, {});
        if (wave < 19)
            require(CentipedeAccess::level(game) == 1, "fragments counted as extra waves");
    }
    require(CentipedeAccess::level(game) == 2, "surviving twenty waves did not start next map");
    require(count(game.snapshot(), Cell::Obstacle) == 45, "new level did not regenerate obstacles");
    Centipede endurance(CentipedeRules{.16, .035, 8, 0});
    for (int step = 0; step < 14000 && CentipedeAccess::level(endurance) == 1; ++step) {
        const auto view = endurance.snapshot();
        const auto head = locate(view, Cell::CentipedeHead);
        const auto player = locate(view, Cell::Player);
        const int targetRow = head.y >= 20 ? 17 : 20;
        std::vector<Input> inputs;
        if (player.y != targetRow)
            inputs.push_back({player.y < targetRow ? Action::Down : Action::Up});
        endurance.update(.16, inputs);
        require(endurance.snapshot().status == Status::Running, "public-control survival lost");
    }
    require(CentipedeAccess::level(endurance) == 2,
            "public-control player did not survive twenty full waves");
}

} // namespace arcade::test
