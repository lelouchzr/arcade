#pragma once
#include "arcade/Api.hpp"
#include <IEventReceiver.h>
#include <IrrlichtDevice.h>

namespace arcade::bonus {
class Irrlicht final : public IGraphics, public irr::IEventReceiver {
  public:
    ~Irrlicht() override;
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;
    bool OnEvent(const irr::SEvent &event) override;

  private:
    irr::IrrlichtDevice *device_ = nullptr;
    std::vector<Input> inputs_;
};
} // namespace arcade::bonus
