#include "BoardRenderer.hpp"
#include <algorithm>
#include <cmath>

namespace arcade {

BoardRenderer::BoardRenderer(Frame &frame, const Snapshot &board)
    : frame_(frame), board_(board), viewport_(board) {
    if (viewport_.isometric) {
        const int span = viewport_.columns + viewport_.rows;
        tile_ = std::max(2, std::min({32, 772 / span, 778 / (span + 2)}));
        const int halfHeight = std::max(1, tile_ / 2);
        const int height = tile_ + span * halfHeight;
        screenOrigin_ = {462 + (viewport_.rows - viewport_.columns) * tile_ / 2,
                         282 + (405 - height) / 2 + tile_ + halfHeight};
    } else {
        tile_ = std::min(772 / viewport_.columns, 405 / viewport_.rows);
        screenOrigin_ = {462 - viewport_.columns * tile_ / 2, 485 - viewport_.rows * tile_ / 2};
    }
}

Point BoardRenderer::screen(int column, int row) const {
    if (viewport_.isometric)
        return {screenOrigin_.x + (column - row) * tile_,
                screenOrigin_.y + (column + row) * std::max(1, tile_ / 2)};
    return {screenOrigin_.x + column * tile_, screenOrigin_.y + row * tile_};
}

void BoardRenderer::rectangle(int x, int y, int width, int height, Color color, bool outline) {
    const int right = std::min(848, x + width), bottom = std::min(687, y + height);
    x = std::max(76, x);
    y = std::max(282, y);
    if (right > x && bottom > y)
        frame_.quads.push_back({x, y, right - x, bottom - y, color, outline});
}

void BoardRenderer::label(Point position, const CellStyle &style, int scale) {
    if (style.glyph != 0 && position.x - 3 * scale >= 76 && position.x + 3 * scale <= 848 &&
        position.y - 4 * scale >= 282 && position.y + 4 * scale <= 687)
        frame_.labels.push_back({position.x - 3 * scale, position.y - 4 * scale, scale,
                                 std::string(1, style.glyph), style.ink});
}

void BoardRenderer::diamond(Point position, Color color) {
    const int halfHeight = std::max(1, tile_ / 2);
    for (int row = -halfHeight; row <= halfHeight; ++row) {
        const int halfWidth = tile_ * (halfHeight - std::abs(row)) / halfHeight;
        rectangle(position.x - halfWidth, position.y + row, halfWidth * 2 + 1, 1, color);
    }
}

bool BoardRenderer::elevated(Point point, Cell occupant) const {
    return std::any_of(board_.sprites.begin(), board_.sprites.end(), [&](const auto &sprite) {
        const bool player = occupant == Cell::Player && sprite.cell == Cell::JumpingPlayer;
        const bool ghost = sprite.cell == Cell::JumpingGhost &&
                           ((occupant >= Cell::GhostRed && occupant <= Cell::GhostOrange) ||
                            occupant == Cell::FrightenedGhost);
        return (player || ghost) && sprite.position == point && std::isfinite(sprite.elevation) &&
               sprite.elevation > 0;
    });
}

void BoardRenderer::tile(int column, int row, Cell cell) {
    const Point world{viewport_.origin.x + column, viewport_.origin.y + row};
    const bool airborne = elevated(world, cell);
    const auto style = cellStyle(airborne ? Cell::Empty : cell);
    const auto position = screen(column, row);
    if (viewport_.isometric) {
        if (cell == Cell::Wall || cell == Cell::Obstacle) {
            rectangle(position.x - tile_, position.y - tile_, tile_ * 2, tile_, {32, 46, 58});
            diamond({position.x, position.y - tile_}, style.fill);
        } else {
            diamond(position, style.fill);
            label(position, style, tile_ >= 24 ? 2 : 1);
        }
        if (board_.cursor == world)
            label(position, CellStyle{style.fill, {234, 239, 225}, '+'}, 1);
    } else {
        rectangle(position.x, position.y, tile_ - 2, tile_ - 2, style.fill);
        label({position.x + tile_ / 2, position.y + tile_ / 2}, style, tile_ >= 24 ? 2 : 1);
        if (board_.cursor == world)
            rectangle(position.x - 1, position.y - 1, tile_, tile_, {234, 239, 225}, true);
    }
}

void BoardRenderer::sprites() {
    std::vector<const Sprite *> visible;
    for (const auto &sprite : board_.sprites) {
        if (sprite.position.x < viewport_.origin.x || sprite.position.y < viewport_.origin.y ||
            sprite.position.x >= viewport_.origin.x + viewport_.columns ||
            sprite.position.y >= viewport_.origin.y + viewport_.rows ||
            !std::isfinite(sprite.elevation) || sprite.elevation < 0)
            continue;
        visible.push_back(&sprite);
    }
    std::stable_sort(visible.begin(), visible.end(),
                     [](const auto *a, const auto *b) { return a->elevation < b->elevation; });
    for (const auto *entry : visible) {
        const auto &sprite = *entry;
        const int column = sprite.position.x - viewport_.origin.x;
        const int row = sprite.position.y - viewport_.origin.y;
        auto position = screen(column, row);
        const auto style = cellStyle(sprite.cell);
        const int rise = static_cast<int>(std::clamp(sprite.elevation, 0., 2.) * tile_);
        if (viewport_.isometric) {
            diamond(position, {28, 38, 46});
            position.y -= rise;
            diamond(position, style.fill);
            label(position, style, tile_ >= 24 ? 2 : 1);
        } else {
            rectangle(position.x + tile_ / 4, position.y + tile_ / 2, tile_ / 2,
                      std::max(1, tile_ / 5), {14, 21, 28});
            rectangle(position.x, position.y - rise, tile_ - 2, tile_ - 2, style.fill);
            label({position.x + tile_ / 2, position.y + tile_ / 2 - rise}, style,
                  tile_ >= 24 ? 2 : 1);
        }
    }
}

void BoardRenderer::draw(Frame &frame, const Snapshot &board) {
    if (!BoardViewport::valid(board))
        return;
    BoardRenderer renderer(frame, board);
    const auto drawCell = [&](int column, int row) {
        const int worldX = renderer.viewport_.origin.x + column;
        const int worldY = renderer.viewport_.origin.y + row;
        renderer.tile(column, row,
                      board.cells[static_cast<std::size_t>(worldY * board.width + worldX)]);
    };
    if (renderer.viewport_.isometric) {
        // Equal-depth diagonals preserve the painter order for raised wall tiles.
        for (int depth = 0; depth < renderer.viewport_.columns + renderer.viewport_.rows - 1;
             ++depth)
            for (int row = 0; row < renderer.viewport_.rows; ++row) {
                const int column = depth - row;
                if (column >= 0 && column < renderer.viewport_.columns)
                    drawCell(column, row);
            }
    } else {
        for (int row = 0; row < renderer.viewport_.rows; ++row)
            for (int column = 0; column < renderer.viewport_.columns; ++column)
                drawCell(column, row);
    }
    renderer.sprites();
}

} // namespace arcade
