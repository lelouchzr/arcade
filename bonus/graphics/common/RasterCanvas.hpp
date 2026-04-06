#pragma once
#include "arcade/Api.hpp"

namespace arcade::bonus {
class RasterCanvas {
  public:
    RasterCanvas();
    void render(const Frame &frame);
    [[nodiscard]] const std::vector<std::uint32_t> &pixels() const noexcept;
    [[nodiscard]] std::vector<std::uint32_t> scaled(int width, int height) const;

  private:
    std::vector<std::uint32_t> pixels_;
    void quad(const Quad &value);
};
} // namespace arcade::bonus
