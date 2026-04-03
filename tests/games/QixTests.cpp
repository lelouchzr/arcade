#include "Fixtures.hpp"
#include "TestSupport.hpp"
#include <algorithm>

namespace arcade::test {

void QixAccess::trail(bonus::Qix &g, const std::vector<Point> &trail, Point anchor) {
    g.trail_ = trail;
    g.anchor_ = anchor;
    g.player_ = trail.back();
    g.heading_ = {};
    for (auto p : trail) {
        g.board_.at(p) = Cell::Trail;
        g.tracks_[g.board_.index(p)] = true;
    }
    g.idle_ = g.fireClock_ = 0;
    g.burning_ = false;
}

void QixAccess::monster(bonus::Qix &g, const std::deque<Point> &points) {
    g.monster_ = points;
}

void QixAccess::sparkTrack(bonus::Qix &g, const std::vector<Point> &track, bonus::Spark spark,
                           Point player) {
    g.board_.fill(Cell::Claimed);
    g.tracks_.assign(g.board_.cells().size(), false);
    for (auto p : track)
        g.tracks_[g.board_.index(p)] = true;
    g.sparks_ = {spark};
    g.player_ = g.anchor_ = player;
    g.heading_ = {};
}

const bonus::Grid &QixAccess::board(const bonus::Qix &g) {
    return g.board_;
}

const std::vector<bonus::Spark> &QixAccess::sparks(const bonus::Qix &g) {
    return g.sparks_;
}

void qixTests() {
    using namespace bonus;
    const QixRules controlled{.08, 10000, 10000, .6, .07};
    Qix game(controlled);
    require(count(game.snapshot(), Cell::Qix) == 3, "Qix must occupy several cells");
    const auto move = [&](Action a, int cells) {
        game.update(.08 * cells, {{a}});
        require(game.snapshot().status != Status::Lost, "Qix lost while cutting safe area");
    };
    move(Action::Up, 5);
    move(Action::Right, 24);
    require(game.snapshot().score > 0, "Qix did not claim cut-off area");
    require(QixAccess::board(game).at({12, 2}) == Cell::Claimed,
            "flood fill kept area away from Qix");
    require(QixAccess::board(game).at({12, 10}) == Cell::Empty,
            "flood fill swallowed monster area");
    move(Action::Down, 10);
    move(Action::Left, 24);
    move(Action::Right, 18);
    move(Action::Up, 10);
    move(Action::Left, 12);
    move(Action::Down, 10);
    require(game.snapshot().status == Status::Won, "claiming more than 75 percent did not win");
    require(count(game.snapshot(), Cell::Qix) == 3, "capture destroyed a Qix segment");
    game.reset(1);
    game.update(.08, {{Action::Left}});
    require(locate(game.snapshot(), Cell::Player) == Point{24, 10}, "Qix border did not wrap");
    game.reset(1);
    QixAccess::trail(game, {{1, 10}, {2, 10}, {3, 10}, {4, 10}}, {0, 10});
    game.update(.08, {{Action::Left}});
    require(locate(game.snapshot(), Cell::Player) == Point{4, 10}, "Qix crossed its own trail");
    game.update(.52, {{Action::Secondary}});
    require(count(game.snapshot(), Cell::Fire) == 1, "stopping did not ignite trail");
    game.update(.3, {});
    require(game.snapshot().status == Status::Lost, "fire reaching player did not lose");
    require(locate(game.snapshot(), Cell::Player) == Point{0, 10}, "loss did not return to anchor");
    Qix moving(QixRules{.08, .14, 10000, .6, .07});
    const auto original = moving.snapshot();
    moving.update(.28, {});
    require(moving.snapshot().cells != original.cells, "Qix monster did not move");
    moving.reset(1);
    QixAccess::trail(moving, {{1, 10}, {2, 10}, {3, 10}}, {0, 10});
    QixAccess::monster(moving, {{2, 10}, {2, 11}, {2, 12}});
    moving.update(.14, {});
    require(moving.snapshot().status == Status::Lost, "Qix touching trail did not lose");
    Qix sparks(QixRules{.08, 10000, .18, .6, .07});
    QixAccess::sparkTrack(sparks, {{4, 5}, {5, 5}, {6, 5}, {7, 5}}, Spark{{5, 5}, {4, 5}}, {7, 5});
    sparks.update(.18, {});
    require(QixAccess::sparks(sparks)[0].position == Point{6, 5},
            "Spark did not use old internal border");
    sparks.update(.18, {});
    require(sparks.snapshot().status == Status::Lost, "Spark contact did not lose");
    sparks.reset(1);
    QixAccess::sparkTrack(sparks, {{4, 5}, {5, 5}}, Spark{{5, 5}, {4, 5}}, {7, 5});
    sparks.update(.18, {});
    require(QixAccess::sparks(sparks)[0].position == Point{5, 5}, "Spark turned back at dead end");
}

} // namespace arcade::test
