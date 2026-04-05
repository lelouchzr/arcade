#include "Qix.hpp"
#include <algorithm>
#include <cmath>

namespace arcade::bonus {

Snapshot Qix::snapshot() const {
    Snapshot view;
    view.width = Width;
    view.height = Height;
    view.cells = board_.cells();
    for (const auto &pickup : pickups_)
        if (board_.at(pickup.position) == Cell::Empty)
            view.cells[board_.index(pickup.position)] =
                pickup.effect == QixPower::Shield ? Cell::Powerup : Cell::BonusFood;
    for (auto point : monster_)
        view.cells[board_.index(point)] = Cell::Qix;
    for (const auto &spark : sparks_)
        view.cells[board_.index(spark.position)] = Cell::Spark;
    if (burning_ && fireIndex_ < trail_.size())
        view.cells[board_.index(trail_[fireIndex_])] = Cell::Fire;
    view.cells[board_.index(player_)] = Cell::Player;
    view.score = score_;
    view.generation = generation_;
    view.status = status_;
    const int remaining =
        static_cast<int>(std::count(board_.cells().begin(), board_.cells().end(), Cell::Empty));
    const int percent = 100 - remaining * 100 / originalArea_;
    view.information = {"ARROWS   MOVE / CUT",
                        "F        STOP",
                        "CLAIMED  " + std::to_string(percent) + "%",
                        "TARGET   MORE THAN 75%",
                        "AVOID QIX AND SPARKS",
                        "DO NOT CROSS YOUR TRAIL",
                        burning_ ? "YOUR TRAIL IS BURNING" : "KEEP MOVING WHILE CUT"};
    if (rules_.bonus) {
        view.information[4] = "SHIELD " + std::to_string(static_cast<int>(std::ceil(shield_))) +
                              "S / SLOW " + std::to_string(static_cast<int>(std::ceil(slow_))) +
                              "S";
        view.information[5] = "CUT THROUGH ITEMS";
    }
    view.information[1] = rules_.bonus ? "D/F7 CLASSIC / F STOP" : "D/F7 BONUSES / F STOP";
    return view;
}

} // namespace arcade::bonus
