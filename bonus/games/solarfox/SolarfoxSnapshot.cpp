#include "Solarfox.hpp"

namespace arcade::bonus {

Snapshot Solarfox::snapshot() const {
    Snapshot view;
    view.width = Width;
    view.height = Height;
    view.cells = board_.cells();
    for (const auto &shot : shots_)
        if (board_.contains(shot.position))
            view.cells[board_.index(shot.position)] = Cell::Projectile;
    for (const auto &shot : enemyShots_)
        if (board_.contains(shot.position))
            view.cells[board_.index(shot.position)] =
                shot.type == SolarShotType::Slow   ? Cell::SlowProjectile
                : shot.type == SolarShotType::Fast ? Cell::FastProjectile
                                                   : Cell::EnemyProjectile;
    if (board_.contains(player_))
        view.cells[board_.index(player_)] = Cell::Ship;
    view.score = score_;
    view.generation = generation_;
    view.status = status_;
    view.information = {"ARROWS   TURN",
                        "SPACE    SHOOT",
                        "F        BOOST",
                        "POWERUPS " + std::to_string(powerups_),
                        "LEVEL    " + std::to_string(level_),
                        "SHOT RANGE 2 CELLS",
                        "SHOOT TO COLLECT"};
    view.information[6] = rules_.bonus ? "D CLASSIC / MOVING GUNS" : "D/F7 BONUS / SHOOT";
    if (rules_.bonus)
        view.information[5] = "SLOW FAST SPREAD SHOTS";
    return view;
}

} // namespace arcade::bonus
