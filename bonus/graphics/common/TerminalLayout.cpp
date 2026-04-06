#include "TerminalLayout.hpp"
#include "arcade/BoardViewport.hpp"
#include "arcade/CellStyle.hpp"
#include <algorithm>
#include <cmath>

namespace arcade::bonus {
std::vector<TerminalText> terminalLayout(const Frame &frame, int columns, int rows) {
    std::vector<TerminalText> result;
    if (columns < 4 || rows < 6)
        return result;
    auto text = [&](int x, int y, std::string value, Color color = {230, 237, 225}) {
        if (x >= 0 && y >= 0 && x < columns - 1 && y < rows)
            result.push_back(
                {x, y, value.substr(0, static_cast<std::size_t>(columns - x - 1)), color});
    };
    if (!frame.board || !BoardViewport::valid(*frame.board)) {
        for (const auto &label : frame.labels)
            text(label.x * columns / Frame::Width, label.y * rows / Frame::Height, label.text,
                 label.color);
        return result;
    }
    const auto &board = *frame.board;
    const BoardViewport viewport(board);
    const int stride = viewport.columns * 2 + 5 < columns ? 2 : 1;
    text(2, 0, frame.title, {190, 244, 109});
    std::string session = "SCORE " + std::to_string(board.score);
    if (board.secondsLeft >= 0)
        session += "  TIME " + std::to_string(static_cast<int>(std::ceil(board.secondsLeft)));
    text(2, 1, session);
    for (int y = 0; y < viewport.rows && y + 3 < rows - 3; ++y)
        for (int x = 0; x < viewport.columns && x * stride + 3 < columns; ++x) {
            const auto cell = viewport.cellAt(board, x, y);
            const auto style = cellStyle(cell);
            char glyph =
                style.glyph ? style.glyph : (cell == Cell::Empty || cell == Cell::Zero ? ' ' : '#');
            if (board.cursor == Point{x + viewport.origin.x, y + viewport.origin.y})
                glyph = '@';
            text(2 + x * stride, 3 + y, std::string(1, glyph),
                 style.glyph ? style.ink : style.fill);
        }
    const int side = viewport.columns * stride + 5;
    for (std::size_t index = 0; index < board.information.size() && index < 12; ++index)
        text(side, 3 + static_cast<int>(index), board.information[index]);
    if (viewport.rows > rows - 6 || viewport.columns * stride > columns - 4)
        text(2, rows - 3, "ENLARGE TERMINAL TO SEE THE FULL BOARD", {255, 190, 90});
    for (const auto &label : frame.labels)
        if (label.text == "PAUSED" || label.text == "YOU WIN" || label.text == "GAME OVER")
            text(2, rows - 2, label.text, {190, 244, 109});
    if (board.status != Status::Running)
        text(2, rows - 2, board.status == Status::Won ? "YOU WIN" : "GAME OVER", {190, 244, 109});
    text(1, rows - 1, "TAB DISPLAY  R RESTART  M MENU  P PAUSE  D BONUS  ESC EXIT");
    return result;
}
} // namespace arcade::bonus
