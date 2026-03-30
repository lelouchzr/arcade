#include "Sdl2.hpp"
#include <stdexcept>

namespace arcade {

Sdl2::~Sdl2() {
    close();
}

void Sdl2::open() {
    if (window_)
        return;
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw std::runtime_error(std::string("SDL2: ") + SDL_GetError());
    initialized_ = true;
    window_ = SDL_CreateWindow("ARCADE / 400 - SDL2", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, Frame::Width, Frame::Height,
                               SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window_) {
        const std::string message = SDL_GetError();
        close();
        throw std::runtime_error(message);
    }
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_)
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer_) {
        const std::string message = SDL_GetError();
        close();
        throw std::runtime_error(message);
    }
    SDL_RenderSetLogicalSize(renderer_, Frame::Width, Frame::Height);
    SDL_StartTextInput();
}

void Sdl2::close() noexcept {
    if (initialized_)
        SDL_StopTextInput();
    if (renderer_)
        SDL_DestroyRenderer(renderer_);
    if (window_)
        SDL_DestroyWindow(window_);
    renderer_ = nullptr;
    window_ = nullptr;
    if (initialized_)
        SDL_Quit();
    initialized_ = false;
}

} // namespace arcade
ARCADE_PLUGIN(arcade::Sdl2, arcade::PluginKind::Graphics, "sdl2", "SDL2 / PIXEL ENGINE")
