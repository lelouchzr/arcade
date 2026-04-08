#include "LegacyGame.hpp"
#include <stdexcept>

namespace arcade::legacy {

Cell LegacyGame::translate(Arcade::Types::SpriteType cell) {
    using S = Arcade::Types::SpriteType;
    switch (cell) {
    case S::EMPTY:
        return Cell::Empty;
    case S::WALL:
        return Cell::Wall;
    case S::FRUIT:
        return Cell::Food;
    case S::S_HEAD:
    case S::N_HEAD:
        return Cell::Head;
    case S::S_BODY:
    case S::N_BODY:
        return Cell::Body;
    default:
        return Cell::Empty;
    }
}

Snapshot LegacyGame::snapshot() const {
    const auto map = game_->getMap();
    if (map.empty() || map.size() > 200 || map.front().empty() || map.front().size() > 200)
        throw std::runtime_error("invalid historical game map dimensions");
    Snapshot view;
    view.width = static_cast<int>(map.front().size());
    view.height = static_cast<int>(map.size());
    for (const auto &row : map) {
        if (row.size() != map.front().size())
            throw std::runtime_error("historical game returned a ragged map");
        for (auto cell : row)
            view.cells.push_back(translate(cell));
    }
    view.score = game_->getScore();
    view.generation = generation_;
    if (game_->getLives() <= 0)
        view.status = Status::Lost;
    else if (game_->isGameRunning())
        view.status = Status::Won;
    const auto time = game_->getTimeLeft();
    view.secondsLeft = time > 0 ? static_cast<double>(time) : -1;
    view.information = {"HISTORICAL SDK ADAPTER",  game_->getGameName(),  "ARROWS   STEER",
                        "FIXED LEGACY TICK 100MS", "ORIGINAL GAME RULES", "SEED NOT IN OLD API"};
    return view;
}

} // namespace arcade::legacy
