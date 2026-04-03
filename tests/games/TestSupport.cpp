#include "TestSupport.hpp"
#include <algorithm>
#include <stdexcept>

namespace arcade::test {

int checks = 0;

void require(bool condition, const std::string &message) {
    ++checks;
    if (!condition)
        throw std::runtime_error(message);
}

int count(const Snapshot &view, Cell cell) {
    return static_cast<int>(std::count(view.cells.begin(), view.cells.end(), cell));
}

Point locate(const Snapshot &view, Cell cell) {
    const auto found = std::find(view.cells.begin(), view.cells.end(), cell);
    if (found == view.cells.end())
        return {-1, -1};
    const int index = static_cast<int>(found - view.cells.begin());
    return {index % view.width, index / view.width};
}

Action action(Point d) {
    if (d.x > 0)
        return Action::Right;
    if (d.x < 0)
        return Action::Left;
    if (d.y > 0)
        return Action::Down;
    return Action::Up;
}

} // namespace arcade::test
