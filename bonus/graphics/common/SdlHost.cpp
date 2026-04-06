#include "SdlHost.hpp"
#include "NativeKeys.hpp"
#include <stdexcept>

namespace arcade::bonus {
SdlHost::~SdlHost() {
    close();
}

void SdlHost::open(const char *title, std::uint32_t flags) {
    if (window_)
        return;
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
        throw std::runtime_error(SDL_GetError());
    initialized_ = true;
    window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Frame::Width,
                               Frame::Height, flags | SDL_WINDOW_RESIZABLE);
    if (!window_) {
        auto error = std::string(SDL_GetError());
        close();
        throw std::runtime_error(error);
    }
    SDL_StartTextInput();
}

void SdlHost::close() noexcept {
    if (window_)
        SDL_DestroyWindow(window_);
    window_ = nullptr;
    if (initialized_) {
        SDL_StopTextInput();
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
    initialized_ = false;
}

SDL_Window *SdlHost::window() const noexcept {
    return window_;
}

std::vector<Input> SdlHost::poll() {
    std::vector<Input> inputs;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            inputs.push_back({Action::Quit});
        if (event.type == SDL_TEXTINPUT)
            for (char c : std::string(event.text.text))
                if (c > 32 && c < 127)
                    inputs.push_back({Action::Text, static_cast<char>(c)});
        if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP)
            continue;
        unsigned key = 0;
        switch (event.key.keysym.sym) {
        case SDLK_LEFT:
            key = 0xff51;
            break;
        case SDLK_UP:
            key = 0xff52;
            break;
        case SDLK_RIGHT:
            key = 0xff53;
            break;
        case SDLK_DOWN:
            key = 0xff54;
            break;
        case SDLK_RETURN:
            key = 0xff0d;
            break;
        case SDLK_ESCAPE:
            key = 0xff1b;
            break;
        case SDLK_TAB:
            key = 0xff09;
            break;
        case SDLK_BACKSPACE:
            key = 0xff08;
            break;
        case SDLK_SPACE:
            key = ' ';
            break;
        default:
            if (event.key.keysym.sym >= SDLK_F2 && event.key.keysym.sym <= SDLK_F7)
                key = 0xffbfU + static_cast<unsigned>(event.key.keysym.sym - SDLK_F2);
        }
        auto input = nativeKey(key, event.type == SDL_KEYDOWN);
        if (input.action != Action::None)
            inputs.push_back(input);
    }
    return inputs;
}
} // namespace arcade::bonus
