#include "Ncurses.hpp"
#include <clocale>
#include <cstdlib>
#include <dlfcn.h>
#include <stdexcept>
#include <unistd.h>

namespace arcade {

Ncurses::~Ncurses() {
    close();
}

void Ncurses::open() {
    if (active_)
        return;
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
        throw std::runtime_error("ncurses needs an interactive terminal");
    const char *term = std::getenv("TERM");
    if (!term || !*term || std::string(term) == "dumb")
        throw std::runtime_error("ncurses needs a valid TERM");
#ifdef RTLD_NODELETE
    // System ncurses keeps process-wide caches beyond delscreen(). Keep their owner loaded.
    // The Arcade module and its SCREEN still unload and close normally.
    Dl_info runtime{};
    if (!dladdr(reinterpret_cast<void *>(newterm), &runtime) || !runtime.dli_fname)
        throw std::runtime_error("ncurses: cannot locate runtime library");
    void *resident = dlopen(runtime.dli_fname, RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
    if (!resident)
        throw std::runtime_error("ncurses: cannot retain runtime caches");
    dlclose(resident);
#endif
    std::setlocale(LC_ALL, "");
    signals_.capture();
    screen_ = newterm(nullptr, stdout, stdin);
    if (!screen_)
        throw std::runtime_error("cannot initialize ncurses (check TERM)");
    set_term(screen_);
    active_ = true;
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    if (has_colors()) {
        start_color();
        use_default_colors();
        for (short i = 0; i < 8; ++i)
            init_pair(static_cast<short>(i + 1), i, -1);
    }
}

void Ncurses::close() noexcept {
    if (active_) {
        endwin();
        active_ = false;
    }
    if (screen_) {
        delscreen(screen_);
        screen_ = nullptr;
    }
    signals_.restore();
}

} // namespace arcade
ARCADE_PLUGIN(arcade::Ncurses, arcade::PluginKind::Graphics, "ncurses", "NCURSES / TERMINAL")
