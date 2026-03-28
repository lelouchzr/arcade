#pragma once

#include "Api.hpp"

namespace arcade {

class BoardViewport {
  public:
    explicit BoardViewport(const Snapshot &board);
    [[nodiscard]] static bool valid(const Snapshot &board);
    [[nodiscard]] Cell cellAt(const Snapshot &board, int column, int row) const;
    Point origin;
    int columns = 0;
    int rows = 0;
    bool isometric = false;
};

} // namespace arcade
