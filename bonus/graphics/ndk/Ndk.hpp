#pragma once
#include "arcade/Api.hpp"
#include "arcade/SignalState.hpp"
#include <memory>
#include <ndk++/application.hh>
#include <ndk++/panel.hh>

namespace arcade::bonus {
class Ndk final : public IGraphics {
  public:
    ~Ndk() override;
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    SignalState signals_;
    std::unique_ptr<ndk::panel> panel_;
    SCREEN *screen_ = nullptr;
};
} // namespace arcade::bonus
