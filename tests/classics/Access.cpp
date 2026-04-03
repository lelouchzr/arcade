#include "Access.hpp"
#include <stdexcept>

namespace arcade::test {
void SnakeBonusAccess::pickup(Snake &game, Point point) {
    game.food_ = {1, 1};
    game.pickup_.place(point);
}

void SnakeBonusAccess::longBody(Snake &game) {
    game.body_.clear();
    for (int x = 12; x >= 1; --x)
        game.body_.push_back({x, 10});
    game.food_ = {1, 1};
}

double SnakeBonusAccess::interval(const Snake &game) {
    return game.stepInterval();
}

void MinesBonusAccess::finish(Minesweeper &game, Status status) {
    game.status_ = status;
}
} // namespace arcade::test

namespace arcade::bonus::test {
void NibblerBonusAccess::pickup(Nibbler &game, Point point) {
    game.pickup_.place(point);
}

int NibblerBonusAccess::food(const Nibbler &game) {
    return game.food_;
}

void NibblerBonusAccess::originalLevel(Nibbler &game, int level) {
    game.bonusEnabled_ = game.originalLevels_ = true;
    game.level_ = level;
    game.buildLevel();
}

void NibblerBonusAccess::lastFood(Nibbler &game) {
    for (auto &cell : game.map_)
        if (cell == Cell::Food)
            cell = Cell::Empty;
    const Point point = game.ahead(game.direction_);
    if (game.wall(point))
        throw std::runtime_error("original wave must start facing an open passage");
    game.map_[static_cast<std::size_t>(point.y * game.width_ + point.x)] = Cell::Food;
    game.food_ = 1;
}

void NibblerBonusAccess::junction(Nibbler &game) {
    game.width_ = Nibbler::Width;
    game.height_ = Nibbler::Height;
    game.originalLevels_ = false;
    game.map_.assign(static_cast<std::size_t>(game.width_ * game.height_), Cell::Empty);
    for (int y = 0; y < game.height_; ++y)
        for (int x = 0; x < game.width_; ++x)
            if (x == 0 || y == 0 || x == game.width_ - 1 || y == game.height_ - 1)
                game.map_[static_cast<std::size_t>(y * game.width_ + x)] = Cell::Wall;
    game.map_[static_cast<std::size_t>(10 * game.width_ + 11)] = Cell::Wall;
    game.map_[static_cast<std::size_t>(game.width_ + 1)] = Cell::Food;
    game.food_ = 1;
    game.body_ = {{12, 10}, {13, 10}, {14, 10}, {15, 10}};
    game.direction_ = game.desired_ = {-1, 0};
}

void NibblerBonusAccess::isolatedFloor(Nibbler &game) {
    junction(game);
    game.map_.assign(static_cast<std::size_t>(game.width_ * game.height_), Cell::Wall);
    for (Point point : game.body_)
        game.map_[static_cast<std::size_t>(point.y * game.width_ + point.x)] = Cell::Empty;
    game.map_[static_cast<std::size_t>(9 * game.width_ + 12)] = Cell::Food;
    game.map_[static_cast<std::size_t>(11 * game.width_ + 12)] = Cell::Food;
    game.map_[static_cast<std::size_t>(11 * game.width_ + 13)] = Cell::Empty;
    game.map_[static_cast<std::size_t>(game.width_ + 1)] = Cell::Empty;
    game.food_ = 2;
}

int NibblerBonusAccess::level(const Nibbler &game) {
    return game.level_;
}
} // namespace arcade::bonus::test
