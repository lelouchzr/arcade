#pragma once

#include "arcade/BoardViewport.hpp"
#include "arcade/CellStyle.hpp"

namespace arcade {

class BoardRenderer {
  public:
    static void draw(Frame &frame, const Snapshot &board);

  private:
    BoardRenderer(Frame &frame, const Snapshot &board);
    Frame &frame_;
    const Snapshot &board_;
    BoardViewport viewport_;
    int tile_ = 0;
    Point screenOrigin_;
    [[nodiscard]] Point screen(int column, int row) const;
    void rectangle(int x, int y, int width, int height, Color color, bool outline = false);
    void label(Point position, const CellStyle &style, int scale);
    void diamond(Point position, Color color);
    void tile(int column, int row, Cell cell);
    void sprites();
    [[nodiscard]] bool elevated(Point point, Cell occupant) const;
};

} // namespace arcade
