#include "Qix.hpp"
#include <algorithm>

namespace arcade::bonus {

Snapshot Qix::snapshot() const {
    Snapshot view;
    view.width = Width;
    view.height = Height;
    view.cells = board_.cells();
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
    const int percent = 100 - remaining * 100 / ((Width - 2) * (Height - 2));
    view.information = {"ARROWS   MOVE / CUT",
                        "F        STOP",
                        "CLAIMED  " + std::to_string(percent) + "%",
                        "TARGET   MORE THAN 75%",
                        "AVOID QIX AND SPARKS",
                        "DO NOT CROSS YOUR TRAIL",
                        burning_ ? "YOUR TRAIL IS BURNING" : "KEEP MOVING WHEN CUTTING"};
    return view;
}

} // namespace arcade::bonus
