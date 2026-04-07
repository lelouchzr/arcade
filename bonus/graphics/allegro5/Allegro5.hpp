#pragma once
#include "arcade/Api.hpp"
#include <allegro5/allegro.h>

namespace arcade::bonus {
class Allegro5 final : public IGraphics {
  public:
    ~Allegro5() override;
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    ALLEGRO_DISPLAY *display_ = nullptr;
    ALLEGRO_EVENT_QUEUE *queue_ = nullptr;
    bool initialized_ = false;
};
} // namespace arcade::bonus
