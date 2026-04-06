#include "Ndk.hpp"
#include "../common/NativeKeys.hpp"
#include "../common/TerminalLayout.hpp"
#include "arcade/BoardViewport.hpp"
#include "arcade/CellStyle.hpp"
#include "arcade/Keys.hpp"
#include <algorithm>
#include <ncurses/color.hh>
#include <ncurses/pen.hh>
#include <stdexcept>

namespace arcade::bonus {
Ndk::~Ndk() {
    close();
}

void Ndk::open() {
    if (panel_)
        return;
    signals_.capture();
    auto &application = ndk::application::instance();
    application.resume();
    screen_ = application.screen();
    set_term(screen_);
    reset_prog_mode();
    try {
        panel_ = std::make_unique<ndk::panel>();
        panel_->resize(static_cast<unsigned>(application.width()),
                       static_cast<unsigned>(application.height()));
        panel_->show();
        nodelay(panel_->win(), TRUE);
        keypad(panel_->win(), TRUE);
        curs_set(0);
    } catch (...) {
        close();
        throw;
    }
}

void Ndk::close() noexcept {
    if (screen_) {
        set_term(screen_);
        panel_.reset();
        ndk::application::instance().suspend();
    }
    screen_ = nullptr;
    signals_.restore();
}

std::vector<Input> Ndk::poll() {
    std::vector<Input> inputs;
    for (int key = panel_ ? wgetch(panel_->win()) : ERR; key != ERR; key = wgetch(panel_->win())) {
        if (key == KEY_RESIZE) {
            panel_->resize(static_cast<unsigned>(COLS), static_cast<unsigned>(LINES));
            continue;
        }
        unsigned symbol = static_cast<unsigned>(key);
        switch (key) {
        case KEY_LEFT:
            symbol = 0xff51;
            break;
        case KEY_UP:
            symbol = 0xff52;
            break;
        case KEY_RIGHT:
            symbol = 0xff53;
            break;
        case KEY_DOWN:
            symbol = 0xff54;
            break;
        case KEY_BACKSPACE:
            symbol = 0xff08;
            break;
        default:
            if (key >= KEY_F(2) && key <= KEY_F(7))
                symbol = 0xffbfU + static_cast<unsigned>(key - KEY_F(2));
        }
        auto input = nativeKey(symbol);
        if (input.action != Action::None)
            inputs.push_back(input);
    }
    return inputs;
}

void Ndk::draw(const Frame &frame) {
    if (!panel_)
        throw std::runtime_error("NDK++ display is closed");
    ncurses::pen pen(*panel_);
    pen.fill(ncurses::color(COLOR_BLACK, COLOR_WHITE));
    pen.clear();
    const int cols = static_cast<int>(panel_->width()), rows = static_cast<int>(panel_->height());
    for (const auto &label : terminalLayout(frame, cols, rows)) {
        pen.move(static_cast<unsigned>(label.x), static_cast<unsigned>(label.y));
        pen.write(label.text);
    }
    pen.commit();
    update_panels();
    doupdate();
}
} // namespace arcade::bonus
ARCADE_PLUGIN(arcade::bonus::Ndk, arcade::PluginKind::Graphics, "ndk", "NDK++ / NATIVE PANEL")
