#include "Libcaca.hpp"
#include "../common/NativeKeys.hpp"
#include "../common/TerminalLayout.hpp"
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace arcade::bonus {
Libcaca::~Libcaca() {
    close();
}

void Libcaca::open() {
    if (display_)
        return;
    signals_.capture();
    try {
        canvas_ = caca_create_canvas(128, 50);
        if (!canvas_)
            throw std::runtime_error("libcaca canvas allocation failed");
        const char *driver = std::getenv("CACA_DRIVER");
        // Slang avoids mixing libcaca's wide ncurses with AA-lib/NDK++'s narrow ncurses.
        if (!driver && isatty(STDIN_FILENO)) {
            for (auto entry = caca_get_display_driver_list(); *entry; entry += 2)
                if (std::string(*entry) == "slang")
                    driver = "slang";
        }
        display_ = caca_create_display_with_driver(canvas_, driver);
        if (!display_)
            throw std::runtime_error("libcaca: no usable display driver");
        caca_set_display_title(display_, "ARCADE / 400 - libcaca");
        caca_set_display_time(display_, 0);
        dither_ = caca_create_dither(32, Frame::Width, Frame::Height, Frame::Width * 4, 0xff0000,
                                     0xff00, 0xff, 0);
        if (!dither_)
            throw std::runtime_error("libcaca dither allocation failed");
        caca_set_dither_algorithm(dither_, "none");
    } catch (...) {
        close();
        throw;
    }
}

void Libcaca::close() noexcept {
    if (dither_)
        caca_free_dither(dither_);
    if (display_)
        caca_free_display(display_);
    if (canvas_)
        caca_free_canvas(canvas_);
    dither_ = nullptr;
    display_ = nullptr;
    canvas_ = nullptr;
    signals_.restore();
}

std::vector<Input> Libcaca::poll() {
    std::vector<Input> inputs;
    caca_event_t event;
    while (display_ && caca_get_event(display_, CACA_EVENT_ANY, &event, 0)) {
        auto type = caca_get_event_type(&event);
        if (type == CACA_EVENT_QUIT)
            inputs.push_back({Action::Quit});
        if (type != CACA_EVENT_KEY_PRESS && type != CACA_EVENT_KEY_RELEASE)
            continue;
        const int key = caca_get_event_key_ch(&event);
        unsigned symbol = static_cast<unsigned>(key);
        switch (key) {
        case CACA_KEY_LEFT:
            symbol = 0xff51;
            break;
        case CACA_KEY_UP:
            symbol = 0xff52;
            break;
        case CACA_KEY_RIGHT:
            symbol = 0xff53;
            break;
        case CACA_KEY_DOWN:
            symbol = 0xff54;
            break;
        default:
            if (key >= CACA_KEY_F2 && key <= CACA_KEY_F7)
                symbol = 0xffbfU + static_cast<unsigned>(key - CACA_KEY_F2);
        }
        auto input = nativeKey(symbol, type == CACA_EVENT_KEY_PRESS);
        if (input.action != Action::None)
            inputs.push_back(input);
    }
    return inputs;
}

void Libcaca::draw(const Frame &frame) {
    if (!display_)
        throw std::runtime_error("libcaca display is closed");
    pixels_.render(frame);
    const int width = caca_get_canvas_width(canvas_), height = caca_get_canvas_height(canvas_);
    caca_clear_canvas(canvas_);
    caca_dither_bitmap(canvas_, 0, 0, width, height, dither_, pixels_.pixels().data());
    if (frame.board)
        caca_clear_canvas(canvas_);
    for (const auto &label : terminalLayout(frame, width, height)) {
        caca_set_color_argb(canvas_,
                            static_cast<std::uint16_t>(0xf000U | ((label.color.r >> 4U) << 8U) |
                                                       ((label.color.g >> 4U) << 4U) |
                                                       (label.color.b >> 4U)),
                            0xf000);
        caca_put_str(canvas_, label.x, label.y, label.text.c_str());
    }
    caca_refresh_display(display_);
}
} // namespace arcade::bonus
ARCADE_PLUGIN(arcade::bonus::Libcaca, arcade::PluginKind::Graphics, "libcaca",
              "LIBCACA / COLOUR ASCII")
