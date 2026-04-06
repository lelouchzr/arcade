#include "Aalib.hpp"
#include "../common/NativeKeys.hpp"
#include "../common/TerminalLayout.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace arcade::bonus {
Aalib::~Aalib() {
    close();
}

void Aalib::open() {
    if (context_)
        return;
    signals_.capture();
    auto params = aa_defparams;
    params.recwidth = 128;
    params.recheight = 50;
    if (!aa_parseoptions(&params, &aa_defrenderparams, nullptr, nullptr))
        throw std::runtime_error("aa-lib: invalid AAOPTS");
    const char *requested = std::getenv("ARCADE_AA_DRIVER");
    std::string driver = requested ? requested : (isatty(STDIN_FILENO) ? "slang" : "auto");
    if (driver != "auto" && driver != "curses" && driver != "slang")
        throw std::runtime_error("aa-lib: ARCADE_AA_DRIVER must be auto, slang or curses");
    const aa_driver *displayDriver = nullptr;
    const aa_kbddriver *keyboardDriver = nullptr;
    if (driver != "auto") {
        for (auto entry = aa_drivers; *entry; ++entry)
            if (std::string((*entry)->shortname) == driver)
                displayDriver = *entry;
        for (auto entry = aa_kbddrivers; *entry; ++entry)
            if (std::string((*entry)->shortname) == driver)
                keyboardDriver = *entry;
        if (!displayDriver || !keyboardDriver)
            throw std::runtime_error("aa-lib was built without the requested terminal drivers");
    }
    context_ = displayDriver ? aa_init(displayDriver, &params, nullptr) : aa_autoinit(&params);
    if (!context_)
        throw std::runtime_error("aa-lib: no usable display driver");
    if (!(keyboardDriver ? aa_initkbd(context_, keyboardDriver, 0) : aa_autoinitkbd(context_, 0))) {
        close();
        throw std::runtime_error("aa-lib: no usable keyboard driver");
    }
    aa_hidecursor(context_);
}

void Aalib::close() noexcept {
    if (context_)
        aa_close(context_);
    context_ = nullptr;
    signals_.restore();
}

std::vector<Input> Aalib::poll() {
    std::vector<Input> inputs;
    for (int key = context_ ? aa_getevent(context_, 0) : 0; key; key = aa_getevent(context_, 0)) {
        if (key == AA_RESIZE) {
            aa_resize(context_);
            continue;
        }
        unsigned symbol = static_cast<unsigned>(key);
        switch (key) {
        case AA_LEFT:
            symbol = 0xff51;
            break;
        case AA_UP:
            symbol = 0xff52;
            break;
        case AA_RIGHT:
            symbol = 0xff53;
            break;
        case AA_DOWN:
            symbol = 0xff54;
            break;
        default:
            break;
        }
        auto input = nativeKey(symbol);
        if (input.action != Action::None)
            inputs.push_back(input);
    }
    return inputs;
}

void Aalib::draw(const Frame &frame) {
    if (!context_)
        throw std::runtime_error("aa-lib display is closed");
    canvas_.render(frame);
    const int width = aa_imgwidth(context_), height = aa_imgheight(context_);
    if (width < 1 || height < 1)
        return;
    auto pixels = canvas_.scaled(width, height);
    auto *image = aa_image(context_);
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        auto p = pixels[i];
        image[i] = static_cast<unsigned char>(
            (((p >> 16U) & 255U) * 54U + ((p >> 8U) & 255U) * 183U + (p & 255U) * 19U) / 256U);
    }
    aa_render(context_, &aa_defrenderparams, 0, 0, aa_scrwidth(context_), aa_scrheight(context_));
    const int cols = aa_scrwidth(context_), rows = aa_scrheight(context_);
    if (frame.board) {
        for (int y = 0; y < rows; ++y)
            aa_puts(context_, 0, y, AA_NORMAL,
                    std::string(static_cast<std::size_t>(cols), ' ').c_str());
    }
    for (const auto &label : terminalLayout(frame, cols, rows))
        aa_puts(context_, label.x, label.y, AA_BOLD, label.text.c_str());
    aa_flush(context_);
}
} // namespace arcade::bonus
ARCADE_PLUGIN(arcade::bonus::Aalib, arcade::PluginKind::Graphics, "aalib",
              "AA-LIB / MONOCHROME ASCII")
