#include "PacmanBonusAccess.hpp"
#include "TestSupport.hpp"
#include <algorithm>
#include <cmath>

namespace arcade::test {

void PacmanBonusAccess::park(bonus::Pacman &game, Point point, Point heading) {
    game.player_ = point;
    game.heading_ = game.desired_ = heading;
    game.playerClock_ = 0;
}

void PacmanBonusAccess::ghost(bonus::Pacman &game, std::size_t index, Point point, double jump) {
    auto &ghost = game.ghosts_.at(index);
    ghost = {};
    ghost.position = point;
    ghost.mode = bonus::GhostMode::Active;
    ghost.jump.elapsed = jump;
}

double PacmanBonusAccess::playerHeight(const bonus::Pacman &game) {
    return game.elevation(game.jump_);
}

double PacmanBonusAccess::ghostHeight(const bonus::Pacman &game, std::size_t index) {
    return game.elevation(game.ghosts_.at(index).jump);
}

double PacmanBonusAccess::acceleration(const bonus::Pacman &game) {
    return game.acceleration();
}

namespace {

bonus::PacmanRules safeRules() {
    bonus::PacmanRules rules;
    rules.releaseDelay = 10000;
    rules.ghostPeriod = 10000;
    rules.bonusEnabled = true;
    rules.fruitDelay = 10000;
    return rules;
}

void fruitTests() {
    auto rules = safeRules();
    rules.fruitDelay = .3;
    rules.fruitDuration = .4;
    rules.fruitInterval = .5;
    bonus::Pacman game(rules);
    PacmanBonusAccess::park(game);
    game.update(.29, {});
    require(count(game.snapshot(), Cell::BonusFood) == 0, "fruit appeared before delay");
    game.update(.01, {});
    require(locate(game.snapshot(), Cell::BonusFood) == Point{12, 12},
            "fruit must appear at Pacman's starting position");
    game.update(.39, {});
    require(count(game.snapshot(), Cell::BonusFood) == 1, "fruit expired too early");
    game.update(.01, {});
    require(count(game.snapshot(), Cell::BonusFood) == 0, "fruit did not expire");
    game.update(.5, {});
    require(count(game.snapshot(), Cell::BonusFood) == 1, "fruit did not respawn");
    const auto generation = game.snapshot().generation;
    const auto score = game.snapshot().score;
    PacmanBonusAccess::park(game, {12, 12}, {0, -1});
    game.update(.01, {});
    require(game.snapshot().score == score + 1000, "fruit bonus must award 1000 points");
    require(game.snapshot().generation == generation, "fruit started a new score round");
    game.update(.1, {});
    require(game.snapshot().score == score + 1000, "fruit can be collected twice");
    game.reset(42);
    PacmanBonusAccess::park(game);
    require(count(game.snapshot(), Cell::BonusFood) == 0, "reset retained an active fruit");
    game.update(.29, {});
    require(count(game.snapshot(), Cell::BonusFood) == 0, "reset did not restore fruit timer");

    rules.fruitDelay = .01;
    rules.fruitDuration = 2;
    bonus::Pacman airborne(rules);
    PacmanBonusAccess::park(airborne);
    airborne.update(.4, {{Action::Jump}});
    const auto beforeLanding = airborne.snapshot().score;
    PacmanBonusAccess::park(airborne, {12, 12}, {0, -1});
    airborne.update(.01, {});
    require(airborne.snapshot().score == beforeLanding,
            "airborne player collected fruit above the ground");
    airborne.update(.39, {});
    require(airborne.snapshot().score == beforeLanding + 1000,
            "ground fruit was not collected when landing");
}

void jumpTests() {
    bonus::Pacman game(safeRules());
    PacmanBonusAccess::park(game);
    game.update(.4, {{Action::Jump}});
    require(std::abs(PacmanBonusAccess::playerHeight(game) - 1.4) < 1e-8,
            "jump did not reach its 1.4-cell apex");
    const auto view = game.snapshot();
    require(view.sprites.size() == 1 && view.sprites.front().cell == Cell::JumpingPlayer &&
                view.sprites.front().position == Point{2, 1},
            "jump elevation was not exposed as a player sprite");
    require(locate(view, Cell::Player) == Point{2, 1}, "jump removed semantic player cell");
    game.update(.1, {{Action::Primary}});
    require(PacmanBonusAccess::playerHeight(game) < 1.4, "jump input restarted an airborne jump");
    game.update(.3, {});
    require(PacmanBonusAccess::playerHeight(game) == 0 && game.snapshot().sprites.empty(),
            "jump did not land at configured duration");
    game.update(.1, {{Action::Primary}});
    require(PacmanBonusAccess::playerHeight(game) == 0, "jump bypassed cooldown");
    game.update(.5, {});
    game.update(.4, {{Action::Primary}});
    require(PacmanBonusAccess::playerHeight(game) > 1.3, "jump unavailable after cooldown");
    game.reset(42);
    require(game.snapshot().sprites.empty(), "reset retained airborne state");
}

void heightCollisionTests() {
    bonus::Pacman over(safeRules());
    PacmanBonusAccess::park(over);
    over.update(.4, {{Action::Jump}});
    PacmanBonusAccess::ghost(over, 0, {2, 1});
    over.update(.01, {});
    require(over.snapshot().status == Status::Running,
            "jumping player collided with ground ghost at the apex");
    const auto crossingView = over.snapshot();
    require(std::any_of(crossingView.sprites.begin(), crossingView.sprites.end(),
                        [](const Sprite &sprite) {
                            return sprite.cell == Cell::GhostRed && sprite.elevation == 0;
                        }),
            "ground ghost beneath the jumping player was removed from the presentation");
    over.update(.4, {});
    require(over.snapshot().status == Status::Lost, "landing on a ghost did not lose");

    bonus::Pacman same(safeRules());
    PacmanBonusAccess::park(same);
    same.update(.4, {{Action::Jump}});
    PacmanBonusAccess::ghost(same, 0, {2, 1}, .4);
    same.update(.01, {});
    require(same.snapshot().status == Status::Lost,
            "actors jumping at the same height must still collide");

    bonus::Pacman below(safeRules());
    PacmanBonusAccess::park(below);
    PacmanBonusAccess::ghost(below, 0, {2, 1}, .4);
    below.update(.01, {});
    require(below.snapshot().status == Status::Running,
            "ground player collided with a ghost above its head");
    below.update(.4, {});
    require(below.snapshot().status == Status::Lost, "landing ghost collision was missed");
}

void ghostJumpTests() {
    auto rules = safeRules();
    rules.ghostJumpInterval = .3;
    bonus::Pacman game(rules);
    PacmanBonusAccess::park(game);
    PacmanBonusAccess::ghost(game, 0, {3, 1});
    PacmanBonusAccess::ghost(game, 1, {5, 1});
    game.update(.3, {});
    game.update(.4, {});
    require(PacmanBonusAccess::ghostHeight(game, 0) == 0, "non-jumping ghost unexpectedly jumped");
    require(PacmanBonusAccess::ghostHeight(game, 1) > 1.3,
            "jumping ghost did not jump near Pacman after its timer");
    const auto view = game.snapshot();
    require(std::count_if(view.sprites.begin(), view.sprites.end(),
                          [](const Sprite &sprite) { return sprite.cell == Cell::JumpingGhost; }) ==
                1,
            "jumping ghost sprite missing");
    game.update(.4, {});
    require(PacmanBonusAccess::ghostHeight(game, 1) == 0, "ghost jump failed to land");
    game.update(.1, {});
    require(PacmanBonusAccess::ghostHeight(game, 1) == 0, "ghost jump bypassed cooldown");
}

void cameraAndSpeedTests() {
    bonus::Pacman classic;
    require(!classic.snapshot().camera && classic.snapshot().sprites.empty(),
            "classic Pacman must remain the default");
    const auto generation = classic.snapshot().generation;
    classic.update(0, {{Action::Difficulty}});
    require(classic.snapshot().generation == generation + 1 && classic.snapshot().camera,
            "bonus toggle must create a fresh score round and camera");
    classic.update(0, {{Action::Camera}});
    require(!classic.snapshot().camera, "camera toggle did not restore full-board view");
    classic.update(0, {{Action::Camera}});
    require(classic.snapshot().camera && classic.snapshot().camera->isometric,
            "camera toggle did not restore Pacmania projection");
    require(classic.snapshot().information.size() <= 7, "Pacmania controls exceed HUD capacity");
    classic.update(0, {{Action::Difficulty}});
    require(!classic.snapshot().camera, "returning to classic retained Pacmania camera");

    bonus::Pacman faster(safeRules());
    PacmanBonusAccess::park(faster);
    faster.update(60, {});
    faster.update(60, {});
    require(std::abs(PacmanBonusAccess::acceleration(faster) - 1.6) < 1e-8,
            "timed speed progression must cap at 1.6x");
    PacmanBonusAccess::park(faster, {10, 12}, {1, 0});
    faster.update(.1, {});
    require(locate(faster.snapshot(), Cell::Player) == Point{11, 12},
            "timed speed progression did not affect actual movement");
    require(faster.snapshot().camera && faster.snapshot().camera->center == Point{11, 12} &&
                faster.snapshot().camera->columns == 17 && faster.snapshot().camera->rows == 13,
            "Pacmania camera did not follow the moved player");
    faster.reset(42);
    require(PacmanBonusAccess::acceleration(faster) == 1, "reset retained timed acceleration");
}

} // namespace

void pacmanBonusTests() {
    fruitTests();
    jumpTests();
    heightCollisionTests();
    ghostJumpTests();
    cameraAndSpeedTests();
}

} // namespace arcade::test
