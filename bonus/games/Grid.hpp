#pragma once

#include "arcade/Api.hpp"
#include <array>

namespace arcade::bonus {

inline constexpr std::array<Point, 4> Directions{{{1, 0}, {0, 1}, {-1, 0}, {0, -1}}};

class Grid {
  public:
    Grid(int width, int height, Cell initial = Cell::Empty);
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    [[nodiscard]] bool contains(Point point) const;
    [[nodiscard]] std::size_t index(Point point) const;
    [[nodiscard]] Point point(std::size_t index) const;
    [[nodiscard]] Point wrap(Point point) const;
    Cell &at(Point point);
    [[nodiscard]] Cell at(Point point) const;
    [[nodiscard]] const std::vector<Cell> &cells() const;
    void fill(Cell value);

  private:
    int width_;
    int height_;
    std::vector<Cell> cells_;
};

Point add(Point a, Point b);
Point direction(Action action);
int manhattan(Point a, Point b);
void validateDelta(double seconds);

} // namespace arcade::bonus
