#include "../bonus/graphics/common/NativeKeys.hpp"
#include "../bonus/graphics/common/RasterCanvas.hpp"
#include <climits>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool value) {
    if (!value)
        throw std::runtime_error("extra display raster/key regression");
}
} // namespace

int main() {
    try {
        using namespace arcade;
        bonus::RasterCanvas canvas;
        Frame frame;
        frame.quads = {{-5, -5, 10, 10, {255, 0, 0}, false},
                       {-2, 10, 10, 10, {0, 255, 0}, true},
                       {INT_MAX, INT_MAX, INT_MAX, INT_MAX, {255, 255, 255}, false},
                       {INT_MIN, INT_MIN, 2, 2, {255, 255, 255}, false}};
        canvas.render(frame);
        auto pixel = [&](int x, int y) {
            return canvas.pixels().at(static_cast<std::size_t>(y * Frame::Width + x));
        };
        require(pixel(0, 0) == 0xffff0000U && pixel(4, 4) == 0xffff0000U);
        require(pixel(5, 5) == 0xff0a0f17U);
        require(pixel(0, 10) == 0xff00ff00U && pixel(7, 15) == 0xff00ff00U);
        require(pixel(0, 15) == 0xff0a0f17U); // Clipping must not invent an outline edge.
        require(canvas.scaled(1280, 800) == canvas.pixels());
        auto letterbox = canvas.scaled(1280, 1000);
        require(letterbox[0] == 0xff0a0f17U && letterbox[100 * 1280] == 0xffff0000U);
        require(canvas.scaled(1, 1).size() == 1);
        for (int invalid : {0, -1, 8193}) {
            bool rejected = false;
            try {
                (void)canvas.scaled(invalid, 10);
            } catch (const std::runtime_error &) {
                rejected = true;
            }
            require(rejected);
        }
        canvas.render(Frame{});
        require(pixel(0, 0) == 0xff0a0f17U); // A new frame clears old pixels.
        for (unsigned key = 0xff51; key <= 0xff54; ++key) {
            require(bonus::nativeKey(key).action != Action::None);
            require(bonus::nativeKey(key, false).action == Action::Stop);
        }
        require(bonus::nativeKey(0xffc4).action == Action::Difficulty);
        require(bonus::nativeKey('a').action == Action::Text);
        require(bonus::nativeKey('a').text == 'a');
        require(bonus::nativeKey('a', false).action == Action::None);
        std::cout << "Raster clipping, extreme coordinates, clear, resize and native key mapping "
                     "passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
