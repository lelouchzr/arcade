#pragma once
#include "../common/RasterCanvas.hpp"
#include "arcade/SignalState.hpp"
#include <aalib.h>

namespace arcade::bonus {
class Aalib final : public IGraphics {
  public:
    ~Aalib() override;
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    SignalState signals_;
    aa_context *context_ = nullptr;
    RasterCanvas canvas_;
};
} // namespace arcade::bonus
