#include "Irrlicht.hpp"
#include "../common/NativeKeys.hpp"
#include "arcade/Font.hpp"
#include <IVideoDriver.h>
#include <SIrrCreationParameters.h>
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace irr {
extern "C" IRRLICHT_API IrrlichtDevice *IRRCALLCONV
createDeviceEx(const SIrrlichtCreationParameters &);
}

namespace arcade::bonus {
Irrlicht::~Irrlicht() {
    close();
}

void Irrlicht::open() {
    if (device_)
        return;
    irr::SIrrlichtCreationParameters params;
    params.DriverType = irr::video::EDT_BURNINGSVIDEO;
    params.WindowSize = {Frame::Width, Frame::Height};
    params.EventReceiver = this;
    params.LoggingLevel = irr::ELL_NONE;
    device_ = irr::createDeviceEx(params);
    if (!device_)
        throw std::runtime_error("Irrlicht device creation failed");
    device_->setWindowCaption(L"ARCADE / 400 - Irrlicht");
    device_->setResizable(true);
}

void Irrlicht::close() noexcept {
    if (device_) {
        device_->closeDevice();
        device_->drop();
    }
    device_ = nullptr;
    inputs_.clear();
}

bool Irrlicht::OnEvent(const irr::SEvent &event) {
    if (event.EventType != irr::EET_KEY_INPUT_EVENT)
        return false;
    const auto &e = event.KeyInput;
    unsigned key = 0;
    switch (e.Key) {
    case irr::KEY_LEFT:
        key = 0xff51;
        break;
    case irr::KEY_UP:
        key = 0xff52;
        break;
    case irr::KEY_RIGHT:
        key = 0xff53;
        break;
    case irr::KEY_DOWN:
        key = 0xff54;
        break;
    case irr::KEY_RETURN:
        key = 0xff0d;
        break;
    case irr::KEY_ESCAPE:
        key = 0xff1b;
        break;
    case irr::KEY_TAB:
        key = 0xff09;
        break;
    case irr::KEY_BACK:
        key = 0xff08;
        break;
    default:
        if (e.Key >= irr::KEY_F2 && e.Key <= irr::KEY_F7)
            key = 0xffbfU + static_cast<unsigned>(e.Key - irr::KEY_F2);
        else if (e.Char > 0 && e.Char < 127)
            key = static_cast<unsigned>(e.Char);
    }
    auto input = nativeKey(key, e.PressedDown);
    if (input.action != Action::None)
        inputs_.push_back(input);
    return true;
}

std::vector<Input> Irrlicht::poll() {
    if (device_ && !device_->run())
        inputs_.push_back({Action::Quit});
    return std::exchange(inputs_, {});
}

void Irrlicht::draw(const Frame &frame) {
    if (!device_)
        throw std::runtime_error("Irrlicht display is closed");
    auto *driver = device_->getVideoDriver();
    const auto size = driver->getScreenSize();
    const double scale = std::min(static_cast<double>(size.Width) / Frame::Width,
                                  static_cast<double>(size.Height) / Frame::Height);
    const int left = (static_cast<int>(size.Width) - static_cast<int>(Frame::Width * scale)) / 2;
    const int top = (static_cast<int>(size.Height) - static_cast<int>(Frame::Height * scale)) / 2;
    driver->beginScene(true, true, irr::video::SColor(255, 10, 15, 23));
    auto quad = [&](const Quad &q) {
        irr::core::rect<irr::s32> box(left + static_cast<int>(q.x * scale),
                                      top + static_cast<int>(q.y * scale),
                                      left + static_cast<int>((q.x + q.width) * scale),
                                      top + static_cast<int>((q.y + q.height) * scale));
        irr::video::SColor color(255, q.color.r, q.color.g, q.color.b);
        if (q.outline)
            driver->draw2DRectangleOutline(box, color);
        else
            driver->draw2DRectangle(color, box);
    };
    for (const auto &q : frame.quads)
        quad(q);
    for (const auto &label : frame.labels)
        drawLabel(label, quad);
    driver->endScene();
}
} // namespace arcade::bonus
ARCADE_PLUGIN(arcade::bonus::Irrlicht, arcade::PluginKind::Graphics, "irrlicht",
              "IRRLICHT / SOFTWARE VIDEO")
