#include "LegacyGraphics.hpp"
#include "arcade/BoardViewport.hpp"
#include "arcade/CellStyle.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

namespace arcade::legacy {

std::string LegacyGraphics::escapeFormat(const std::string &value) {
    std::string escaped;
    for (char character : value) {
        escaped += character;
        if (character == '%')
            escaped += '%';
    }
    return escaped;
}

Arcade::Types::ForeColor LegacyGraphics::color(Color c) {
    using C = Arcade::Types::ForeColor;
    if (c.r > 180 && c.g > 180 && c.b > 170)
        return C::WHITE;
    if (c.r > 180 && c.g > 180)
        return C::GREEN;
    if (c.r > 150 && c.b > 150 && c.g < 170)
        return C::MAGENTA;
    if (c.r > 190 && c.g < 140)
        return C::RED;
    if (c.r > 190)
        return C::YELLOW;
    if (c.b > 150 && c.b > c.g * 1.5)
        return C::BLUE;
    if (c.b > 150)
        return C::CYAN;
    if (c.g > 100 && c.g > c.r)
        return C::GREEN;
    return C::WHITE;
}

void LegacyGraphics::text(const std::string &value, int x, int y) {
    int column = x * 90 / Frame::Width;
    int row = y * 30 / Frame::Height;
    if (profile_ == LegacyProfile::Sdl) {
        column = static_cast<int>((x - 100) / 17.5);
        row = (y - 200) / 40;
    }
    if (profile_ == LegacyProfile::Sfml) {
        column = (x - 600) / 20;
        row = (y - 500) / 40;
    }
    display_->drawText(value, column, row);
}

void LegacyGraphics::footer(const std::string &message) {
    if (profile_ == LegacyProfile::Terminal) {
        winsize size{};
        const int row = ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_row > 0
                            ? static_cast<int>(size.ws_row) - 1
                            : 29;
        display_->drawText(message, 1, row);
    } else
        text(message, 20, 780);
}

void LegacyGraphics::panels(const Frame &frame) {
    if (profile_ != LegacyProfile::Terminal && profile_ != LegacyProfile::RawText)
        return;
    for (const auto &quad : frame.quads) {
        if (!quad.outline || quad.width < 100 || quad.height < 100)
            continue;
        const int x = quad.x * 90 / Frame::Width, y = quad.y * 30 / Frame::Height;
        const int width = std::max(2, quad.width * 90 / Frame::Width),
                  height = std::max(2, quad.height * 30 / Frame::Height);
        std::string edge{"+"};
        edge.append(static_cast<std::size_t>(width - 2), '-');
        edge.push_back('+');
        display_->drawText(edge, x, y);
        display_->drawText(edge, x, y + height - 1);
        for (int row = y + 1; row < y + height - 1; ++row) {
            display_->drawText("|", x, row);
            display_->drawText("|", x + width - 1, row);
        }
    }
}

void LegacyGraphics::board(const Snapshot &view) {
    if (view.width <= 0 || view.height <= 0 || view.width > 200 || view.height > 200 ||
        view.cells.size() != static_cast<std::size_t>(view.width * view.height))
        throw std::runtime_error("invalid board for historical graphics");
    const BoardViewport viewport(view);
    for (int y = 0; y < viewport.rows; ++y)
        for (int x = 0; x < viewport.columns; ++x) {
            const auto cell = viewport.cellAt(view, x, y);
            const auto style = cellStyle(cell);
            const bool transparent = style.fill.r == 22 && style.fill.g == 32 && style.fill.b == 43;
            const bool selected =
                view.cursor == Point{x + viewport.origin.x, y + viewport.origin.y};
            const auto foreground = selected ? Arcade::Types::ForeColor::GREEN
                                             : color(transparent ? style.ink : style.fill);
            // The original ABI defines exactly eight usable sprite slots. Share each by color,
            // varying its character per draw without changing an already-used ncurses color pair.
            const auto slot = static_cast<Arcade::Types::SpriteType>(foreground);
            char glyph = style.glyph;
            if (!glyph)
                glyph = cell == Cell::Empty || cell == Cell::Zero ? ' '
                        : cell == Cell::Claimed                   ? '.'
                                                                  : '#';
            if (selected)
                glyph = '+';
            if (display_->createSprite(slot, glyph, Arcade::Types::BackColor::BLACK, foreground))
                display_->drawTile(slot, x, y + 4, viewport.rows, viewport.columns);
            else
                text(std::string(1, glyph), 220 + x * 28, 220 + y * 24);
        }
    const auto maximum = static_cast<std::int64_t>(std::numeric_limits<Arcade::Types::i32>::max());
    display_->drawScore(
        static_cast<Arcade::Types::i32>(std::clamp<std::int64_t>(view.score, 0, maximum)));
    display_->drawLives(view.status == Status::Lost ? 0 : 1);
    if (view.secondsLeft >= 0)
        display_->drawTimeLeft(static_cast<Arcade::Types::i32>(
            std::clamp(std::ceil(view.secondsLeft), 0.0, static_cast<double>(maximum))));
    for (std::size_t i = 0; i < view.information.size() && i < 7; ++i)
        text(view.information[i], 20, 110 + static_cast<int>(i) * 55);
    if (view.status != Status::Running)
        text(view.status == Status::Won ? "YOU WIN / K RESTART" : "GAME OVER / K RESTART", 30, 740);
}

void LegacyGraphics::draw(const Frame &frame) {
    if (!display_)
        throw std::runtime_error("historical graphics is not open");
    playing_ = frame.board.has_value();
    player_ = frame.player.empty() ? "PLAYER" : frame.player;
    display_->clear();
    if (frame.board) {
        const bool printfHud =
            profile_ == LegacyProfile::Terminal || profile_ == LegacyProfile::RawText;
        display_->drawGameName(printfHud ? escapeFormat(frame.title) : frame.title);
        display_->drawPlayerName(printfHud ? escapeFormat(player_) : player_);
        board(*frame.board);
        footer("H ACTION  J PAUSE  K RESTART  L LEVEL  G DISPLAY  B GAME  M MENU  ESC EXIT");
        for (const auto &label : frame.labels)
            if (label.text == "PAUSED")
                text("PAUSED / J RESUME", 500, 400);
    } else {
        panels(frame);
        for (const auto &label : frame.labels)
            text(label.text, label.x, label.y);
        footer("H/J CHANGE LETTER  K/L CURSOR  SPACE PLAY  G DISPLAY  ESC EXIT");
    }
    display_->display();
}

} // namespace arcade::legacy
