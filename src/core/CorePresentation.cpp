#include "BoardRenderer.hpp"
#include "Core.hpp"
#include "arcade/CellStyle.hpp"
#include <algorithm>
#include <cmath>

namespace arcade {
namespace {
constexpr Color Background{10, 15, 23}, Panel{17, 25, 36}, Border{43, 58, 71};
constexpr Color Ink{234, 239, 225}, Muted{133, 151, 159}, Green{190, 244, 109};
constexpr Color Orange{255, 164, 96}, Blue{111, 195, 228};

void rectangle(Frame &f, int x, int y, int w, int h, Color c, bool outline = false) {
    f.quads.push_back({x, y, w, h, c, outline});
}

void text(Frame &f, int x, int y, std::string value, Color color = Ink, int size = 2) {
    f.labels.push_back({x, y, size, std::move(value), color});
}

void panel(Frame &f, int x, int y, int w, int h, const std::string &name) {
    rectangle(f, x, y, w, h, Panel);
    rectangle(f, x, y, w, h, Border, true);
    text(f, x + 22, y + 20, name, Muted);
    rectangle(f, x + 22, y + 47, w - 44, 1, Border);
}

std::string clipped(const std::string &value, std::size_t length) {
    return value.substr(0, length);
}
} // namespace

Frame Core::frame() const {
    Frame f;
    // The fixed chrome fits in one allocation; larger plugin boards still grow normally.
    f.quads.reserve(24);
    f.labels.reserve(32);
    f.player = player_.empty() ? "PLAYER" : player_;
    rectangle(f, 0, 0, 1280, 800, Background);
    rectangle(f, 0, 0, 1280, 6, Green);
    text(f, 48, 35, "EPITECH / INTERACTIVE SYSTEMS", Muted);
    text(f, 48, 78, "ARCADE", Ink, 10);
    text(f, 945, 89, "400", Green, 8);
    text(f, 48, 174, menu_ ? "CHOOSE YOUR NEXT HIGH SCORE." : "ONE PLATFORM. EVERY PLAYFIELD.",
         Muted);
    text(f, 945, 174, clipped(graphics_[selectedGraphics_].name, 18), Blue);
    if (menu_) {
        panel(f, 48, 220, 550, 222, "01 / GAME LIBRARY");
        const auto gameStart = selectedGame_ < 4 ? 0 : selectedGame_ - 3;
        for (std::size_t i = gameStart; i < games_.size() && i < gameStart + 4; ++i) {
            int y = 286 + static_cast<int>(i - gameStart) * 36;
            if (i == selectedGame_)
                rectangle(f, 66, y - 7, 514, 31, Green);
            text(f, 80, y, (i == selectedGame_ ? "> " : "  ") + clipped(games_[i].name, 34),
                 i == selectedGame_ ? Background : Ink);
        }
        panel(f, 622, 220, 610, 222, "02 / DISPLAY SYSTEM");
        const auto graphicsStart = selectedGraphics_ < 4 ? 0 : selectedGraphics_ - 3;
        for (std::size_t i = graphicsStart; i < graphics_.size() && i < graphicsStart + 4; ++i) {
            int y = 286 + static_cast<int>(i - graphicsStart) * 36;
            text(f, 646, y,
                 (i == selectedGraphics_ ? "[ON] " : "[  ] ") + clipped(graphics_[i].name, 35),
                 i == selectedGraphics_ ? Blue : Ink);
        }
        panel(f, 48, 466, 738, 234, "03 / LEADERBOARD - " + games_[selectedGame_].name);
        auto records = scores_.top(games_[selectedGame_].id, 4);
        if (records.empty())
            text(f, 74, 540, "A CLEAN SLATE. MAKE IT YOURS.", Muted);
        for (std::size_t i = 0; i < records.size(); ++i) {
            const int y = 532 + static_cast<int>(i) * 34;
            std::string rank{"0"};
            rank.append(std::to_string(i + 1));
            text(f, 74, y, std::move(rank), Green);
            text(f, 134, y, clipped(records[i].player, 16));
            text(f, 584, y, std::to_string(records[i].points), Orange);
        }
        panel(f, 810, 466, 422, 234, "04 / PLAYER NAME");
        text(f, 834, 536, player_.empty() ? "_" : player_ + "_", Green, 3);
        text(f, 834, 591, "TYPE TO EDIT / BACKSPACE", Muted);
        text(f, 834, 641, "ENTER  START GAME", Orange);
        text(f, 48, 736, "UP/DOWN GAME   LEFT/RIGHT DISPLAY   ENTER PLAY   ESC EXIT", Muted);
    } else if (game_) {
        const auto view = game_->instance<IGame>().snapshot();
        f.board = view;
        f.title = game_->info().name;
        panel(f, 48, 220, 828, 486, "01 / " + game_->info().name);
        panel(f, 900, 220, 332, 486, "02 / SESSION");
        text(f, 924, 289, "SCORE", Muted);
        text(f, 924, 319, std::to_string(view.score), Green, 4);
        text(f, 924, 377, "PLAYER / " + clipped(player_.empty() ? "PLAYER" : player_, 14), Ink, 2);
        if (view.secondsLeft >= 0)
            text(f, 924, 410,
                 "TIME  " + std::to_string(static_cast<int>(std::ceil(view.secondsLeft))), Orange);
        for (std::size_t i = 0; i < view.information.size() && i < 7; ++i)
            text(f, 924, 465 + static_cast<int>(i) * 27, clipped(view.information[i], 23), Muted,
                 1);
        BoardRenderer::draw(f, view);
        if (paused_ || view.status != Status::Running) {
            rectangle(f, 234, 426, 456, 106, Background);
            rectangle(f, 234, 426, 456, 106, Green, true);
            text(f, 270, 446,
                 paused_                      ? "PAUSED"
                 : view.status == Status::Won ? "YOU WIN"
                                              : "GAME OVER",
                 Green, 4);
            text(f, 270, 498, paused_ ? "F6 / P TO RESUME" : "F4 / R TO TRY AGAIN", Muted);
        }
        text(f, 48, 736, "D/F7 BONUS  F2 DISPLAY  F3 GAME  F4 RESTART  F5 MENU  F6 PAUSE  ESC EXIT",
             Muted);
    }
    return f;
}

} // namespace arcade
