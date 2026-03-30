#pragma once
#include "arcade/Api.hpp"
#include <SDL.h>

namespace arcade {
class Sdl2 final : public IGraphics {
  public:
    ~Sdl2() override;
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    bool initialized_ = false;
};
} // namespace arcade
