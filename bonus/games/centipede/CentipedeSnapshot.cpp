#include "Centipede.hpp"

namespace arcade::bonus {

Snapshot Centipede::snapshot() const {
    Snapshot view;
    view.width = Width;
    view.height = Height;
    view.cells.assign(board_.cells().size(), Cell::Empty);
    for (std::size_t i = 0; i < obstacles_.size(); ++i)
        if (obstacles_[i] > 0)
            view.cells[i] = Cell::Obstacle;
    for (const auto &body : bodies_)
        for (std::size_t i = 0; i < body.segments.size(); ++i)
            if (board_.contains(body.segments[i]))
                view.cells[board_.index(body.segments[i])] =
                    i == 0 ? Cell::CentipedeHead : Cell::CentipedeBody;
    if (shot_ && board_.contains(*shot_))
        view.cells[board_.index(*shot_)] = Cell::Projectile;
    view.cells[board_.index(player_)] = Cell::Player;
    view.score = score_;
    view.generation = generation_;
    view.status = status_;
    view.information = {"ARROWS   MOVE",
                        "SPACE    FIRE",
                        "ONE SHOT AT A TIME",
                        "MUSHROOMS NEED 5 HITS",
                        "SURVIVED " + std::to_string(survived_) + " / 20",
                        "LEVEL    " + std::to_string(level_),
                        "BOTTOM 20% IS YOUR AREA"};
    return view;
}

} // namespace arcade::bonus
