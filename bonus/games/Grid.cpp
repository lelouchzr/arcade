#include "Grid.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace arcade::bonus {

Grid::Grid(int width, int height, Cell initial) : width_(width), height_(height) {
    if (width <= 0 || height <= 0 || width > 200 || height > 200)
        throw std::invalid_argument("invalid grid dimensions");
    cells_.assign(static_cast<std::size_t>(width * height), initial);
}

int Grid::width() const {
    return width_;
}

int Grid::height() const {
    return height_;
}

bool Grid::contains(Point p) const {
    return p.x >= 0 && p.y >= 0 && p.x < width_ && p.y < height_;
}

std::size_t Grid::index(Point p) const {
    if (!contains(p))
        throw std::out_of_range("grid coordinate outside board");
    return static_cast<std::size_t>(p.y * width_ + p.x);
}

Point Grid::point(std::size_t index) const {
    if (index >= cells_.size())
        throw std::out_of_range("grid index outside board");
    return {static_cast<int>(index) % width_, static_cast<int>(index) / width_};
}

Point Grid::wrap(Point p) const {
    return {(p.x % width_ + width_) % width_, (p.y % height_ + height_) % height_};
}

Cell &Grid::at(Point p) {
    return cells_.at(index(p));
}

Cell Grid::at(Point p) const {
    return cells_.at(index(p));
}

const std::vector<Cell> &Grid::cells() const {
    return cells_;
}

void Grid::fill(Cell value) {
    std::fill(cells_.begin(), cells_.end(), value);
}

Point add(Point a, Point b) {
    return {a.x + b.x, a.y + b.y};
}

Point direction(Action action) {
    switch (action) {
    case Action::Right:
        return {1, 0};
    case Action::Down:
        return {0, 1};
    case Action::Left:
        return {-1, 0};
    case Action::Up:
        return {0, -1};
    default:
        return {0, 0};
    }
}

int manhattan(Point a, Point b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

void validateDelta(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0 || seconds > 60)
        throw std::invalid_argument("invalid game delta time");
}

} // namespace arcade::bonus
