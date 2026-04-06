#pragma once
#include "../common/RasterCanvas.hpp"
#include <X11/Xlib.h>
#undef None

namespace arcade::bonus {
class Xlib final : public IGraphics {
  public:
    ~Xlib() override;
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    Display *display_ = nullptr;
    Window window_ = 0;
    GC gc_ = nullptr;
    Atom closeAtom_ = 0;
    RasterCanvas canvas_;
};
} // namespace arcade::bonus
