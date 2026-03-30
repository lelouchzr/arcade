#include "Ncurses.hpp"
#include "arcade/BoardViewport.hpp"
#include "arcade/CellStyle.hpp"
#include <algorithm>
#include <cmath>

namespace arcade {

void Ncurses::drawBoard(const Frame &frame, int rows, int cols) {
    const auto &board = *frame.board;
    if (!BoardViewport::valid(board)) {
        mvaddstr(2, 2, "Invalid board");
        return;
    }
    const BoardViewport viewport(board);
    if (viewport.columns > 30 || viewport.rows > 21) {
        mvaddstr(2, 2, "Board too large for compact terminal renderer");
        return;
    }
    attrset(COLOR_PAIR(COLOR_GREEN + 1) | A_BOLD);
    mvaddnstr(1, 3, frame.title.c_str(), cols - 5);
    attrset(A_NORMAL);
    mvaddstr(2, 3, "ARCADE / 400");
    for (int y = 0; y < viewport.rows; ++y)
        for (int x = 0; x < viewport.columns; ++x) {
            const auto cell = viewport.cellAt(board, x, y);
            const auto style = cellStyle(cell);
            chtype symbol = style.glyph == 0
                                ? (cell == Cell::Empty || cell == Cell::Zero ? ' ' : ACS_CKBOARD)
                                : static_cast<chtype>(style.glyph);
            const bool transparent = style.fill.r == 22 && style.fill.g == 32 && style.fill.b == 43;
            const int colorPair = pair(transparent ? style.ink : style.fill);
            if (cell == Cell::Claimed)
                symbol = '.';
            if (cell == Cell::Boundary)
                symbol = '#';
            if (cell == Cell::Trail)
                symbol = '+';
            const bool cursor = board.cursor == Point{x + viewport.origin.x, y + viewport.origin.y};
            attrset(COLOR_PAIR(colorPair) | (cursor ? A_REVERSE : A_NORMAL));
            mvaddch(4 + y, 3 + x * 2, symbol);
            mvaddch(4 + y, 4 + x * 2, ' ');
        }
    const int side = std::min(cols - 24, viewport.columns * 2 + 7);
    attrset(COLOR_PAIR(COLOR_GREEN + 1) | A_BOLD);
    mvprintw(5, side, "SCORE %lld", static_cast<long long>(board.score));
    attrset(COLOR_PAIR(COLOR_YELLOW + 1));
    if (board.secondsLeft >= 0)
        mvprintw(8, side, "TIME %d", static_cast<int>(std::ceil(board.secondsLeft)));
    attrset(A_NORMAL);
    for (std::size_t i = 0; i < board.information.size() && i < 7; ++i)
        mvaddnstr(12 + static_cast<int>(i) * 2, side, board.information[i].c_str(),
                  cols - side - 1);
    bool paused = false;
    for (const auto &label : frame.labels)
        if (label.text == "PAUSED")
            paused = true;
    if (paused || board.status != Status::Running) {
        attrset(COLOR_PAIR(COLOR_GREEN + 1) | A_BOLD);
        mvaddstr(rows - 3, 3,
                 paused                        ? "PAUSED / F6 RESUME"
                 : board.status == Status::Won ? "YOU WIN / F4 RESTART"
                                               : "GAME OVER / F4 RESTART");
    }
    attrset(A_NORMAL);
    mvaddnstr(rows - 1, 1,
              "D/F7 BONUS  F2 DISPLAY  F3 GAME  F4 RESTART  F5 MENU  F6 PAUSE  ESC EXIT", cols - 2);
}

int Ncurses::pair(Color c) {
    if (c.r > 180 && c.g > 180 && c.b > 170)
        return COLOR_WHITE + 1;
    if (c.r > 180 && c.g > 180)
        return COLOR_GREEN + 1;
    if (c.r > 150 && c.b > 150 && c.g < 170)
        return COLOR_MAGENTA + 1;
    if (c.r > 190 && c.g < 140)
        return COLOR_RED + 1;
    if (c.r > 190)
        return COLOR_YELLOW + 1;
    if (c.b > 150 && c.b > c.g * 1.5 && c.b > c.r * 1.5)
        return COLOR_BLUE + 1;
    if (c.b > 150)
        return COLOR_CYAN + 1;
    if (c.g > 100 && c.g > c.r)
        return COLOR_GREEN + 1;
    return COLOR_BLUE + 1;
}

} // namespace arcade
