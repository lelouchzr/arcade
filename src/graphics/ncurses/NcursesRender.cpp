#include "Ncurses.hpp"
#include <algorithm>

namespace arcade {

void Ncurses::draw(const Frame &frame) {
    int rows = 0, cols = 0;
    getmaxyx(stdscr, rows, cols);
    erase();
    if (rows < 30 || cols < 90) {
        mvaddnstr(0, 0, "Please enlarge terminal to at least 90 columns x 30 rows. Esc exits.",
                  std::max(0, cols - 1));
        refresh();
        return;
    }
    if (frame.board) {
        drawBoard(frame, rows, cols);
        refresh();
        return;
    }
    for (const auto &quad : frame.quads) {
        if (quad.x == 0 && quad.y == 0 && quad.width == 1280 && quad.height == 800)
            continue;
        const int x = quad.x * cols / Frame::Width, y = quad.y * rows / Frame::Height;
        const int w = std::max(1, quad.width * cols / Frame::Width),
                  h = std::max(1, quad.height * rows / Frame::Height);
        attrset(COLOR_PAIR(pair(quad.color)));
        if (quad.outline) {
            for (int col = x; col < std::min(cols - 1, x + w); ++col) {
                if (y >= 0 && y < rows)
                    mvaddch(y, col, ACS_HLINE);
                if (y + h - 1 >= 0 && y + h - 1 < rows)
                    mvaddch(y + h - 1, col, ACS_HLINE);
            }
            for (int row = y; row < std::min(rows, y + h); ++row) {
                if (x >= 0 && x < cols)
                    mvaddch(row, x, ACS_VLINE);
                if (x + w - 1 >= 0 && x + w - 1 < cols)
                    mvaddch(row, x + w - 1, ACS_VLINE);
            }
        } else if (quad.width < 100 && quad.height < 100) {
            // Board tiles use colored block characters; panels keep the terminal background.
            for (int row = y; row < std::min(rows, y + h); ++row)
                for (int col = x; col < std::min(cols - 1, x + w); ++col)
                    if (row >= 0 && col >= 0)
                        mvaddch(row, col, ACS_CKBOARD);
        }
    }
    for (const auto &label : frame.labels) {
        const int x = label.x * cols / Frame::Width, y = label.y * rows / Frame::Height;
        if (y < 0 || y >= rows || x < 0 || x >= cols - 1)
            continue;
        attrset(COLOR_PAIR(pair(label.color)) | (label.scale >= 3 ? A_BOLD : 0));
        mvaddnstr(y, x, label.text.c_str(), cols - x - 1);
    }
    attrset(A_NORMAL);
    refresh();
}

} // namespace arcade
