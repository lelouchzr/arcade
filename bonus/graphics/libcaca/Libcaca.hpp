#pragma once
#include "../common/RasterCanvas.hpp"
#include "arcade/SignalState.hpp"
#include <caca.h>

namespace arcade::bonus {
class Libcaca final : public IGraphics {
  public:
    ~Libcaca() override;
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    SignalState signals_;
    caca_canvas_t *canvas_ = nullptr;
    caca_display_t *display_ = nullptr;
    caca_dither_t *dither_ = nullptr;
    RasterCanvas pixels_;
};
} // namespace arcade::bonus
