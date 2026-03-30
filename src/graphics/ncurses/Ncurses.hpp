#pragma once
#include "arcade/Api.hpp"
#include "arcade/SignalState.hpp"
#include <curses.h>

namespace arcade {
class Ncurses final : public IGraphics {
  public:
    ~Ncurses() override;
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    SignalState signals_;
    bool active_ = false;
    SCREEN *screen_ = nullptr;
    static void drawBoard(const Frame &frame, int rows, int cols);
    static int pair(Color color);
};
} // namespace arcade
