#include "arcade/BoardViewport.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace arcade {

bool BoardViewport::valid(const Snapshot &board) {
    return board.width > 0 && board.height > 0 && board.width <= 200 && board.height <= 200 &&
           board.cells.size() == static_cast<std::size_t>(board.width * board.height);
}

BoardViewport::BoardViewport(const Snapshot &board) {
    if (!valid(board))
        throw std::invalid_argument("invalid board dimensions");
    columns = board.width;
    rows = board.height;
    if (board.camera) {
        columns = std::clamp(board.camera->columns, 1, std::min(30, board.width));
        rows = std::clamp(board.camera->rows, 1, std::min(21, board.height));
        const int centerX = std::clamp(board.camera->center.x, 0, board.width - 1);
        const int centerY = std::clamp(board.camera->center.y, 0, board.height - 1);
        origin = {std::clamp(centerX - columns / 2, 0, board.width - columns),
                  std::clamp(centerY - rows / 2, 0, board.height - rows)};
        isometric = board.camera->isometric;
    }
}

Cell BoardViewport::cellAt(const Snapshot &board, int column, int row) const {
    if (column < 0 || row < 0 || column >= columns || row >= rows)
        throw std::out_of_range("viewport cell outside board");
    const Point world{origin.x + column, origin.y + row};
    for (const auto &sprite : board.sprites)
        if (sprite.position == world && std::isfinite(sprite.elevation) && sprite.elevation > 0)
            return sprite.cell;
    return board.cells[static_cast<std::size_t>(world.y * board.width + world.x)];
}

} // namespace arcade
