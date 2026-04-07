#pragma once
#include "../common/RasterCanvas.hpp"
#include "../common/SdlHost.hpp"

namespace arcade::bonus {
class OpenGl final : public IGraphics {
  public:
    ~OpenGl() override;
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    SdlHost host_;
    SDL_GLContext context_ = nullptr;
    unsigned texture_ = 0;
    RasterCanvas canvas_;
};
} // namespace arcade::bonus
