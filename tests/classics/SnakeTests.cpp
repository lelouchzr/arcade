#include "Access.hpp"
#include "Cases.hpp"
#include <cmath>

namespace arcade::test {
void snakeBonusTests() {
    Snake game;
    game.update(0, {{Action::Difficulty}});
    SnakeBonusAccess::pickup(game, {13, 10});
    game.update(0.14, {});
    auto view = game.snapshot();
    requireHud(view);
    requireClassic(view.score == 250 && countClassic(view, Cell::Body) == 3,
                   "snake bonus points must not replace regular food/growth");
    requireClassic(countClassic(view, Cell::Food) == 1 && countClassic(view, Cell::BonusFood) == 0,
                   "snake food/bonus cells");
    game.reset(42);
    SnakeBonusAccess::longBody(game);
    requireClassic(SnakeBonusAccess::interval(game) < 0.14, "snake growth must increase speed");
    Snake single, split;
    single.update(0, {{Action::Difficulty}});
    split.update(0, {{Action::Difficulty}});
    single.update(0.7, {{Action::Primary}});
    split.update(0.01, {{Action::Primary}});
    for (int i = 1; i < 70; ++i)
        split.update(0.01, {});
    requireClassic(single.snapshot().cells == split.snapshot().cells,
                   "snake boost must be independent of delta partition");
    requireClassic(single.snapshot().status == Status::Running, "snake boost test hit wall");
    game.reset(42);
    requireClassic(std::abs(SnakeBonusAccess::interval(game) - 0.14) < 1e-12,
                   "snake reset must clear boost/growth");
    SnakeBonusAccess::pickup(game, {2, 2});
    game.update(0, {{Action::Difficulty}});
    requireClassic(countClassic(game.snapshot(), Cell::BonusFood) == 0,
                   "disabling snake bonuses must hide pickup");
}
} // namespace arcade::test
