#pragma once
#include "arcade/Api.hpp"
#include <SDL.h>

namespace arcade::bonus {
class SdlHost {
  public:
    ~SdlHost();
    void open(const char *title, std::uint32_t flags);
    void close() noexcept;
    std::vector<Input> poll();
    [[nodiscard]] SDL_Window *window() const noexcept;

  private:
    SDL_Window *window_ = nullptr;
    bool initialized_ = false;
};
} // namespace arcade::bonus
