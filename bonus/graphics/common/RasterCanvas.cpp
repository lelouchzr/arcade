#include "RasterCanvas.hpp"
#include "arcade/Font.hpp"
#include <algorithm>
#include <stdexcept>

namespace arcade::bonus {
RasterCanvas::RasterCanvas() : pixels_(Frame::Width * Frame::Height) {}

void RasterCanvas::quad(const Quad &q) {
    if (q.width <= 0 || q.height <= 0)
        return;
    const int left = std::clamp(q.x, 0, Frame::Width);
    const int top = std::clamp(q.y, 0, Frame::Height);
    const int right = static_cast<int>(std::clamp(static_cast<long long>(q.x) + q.width, 0LL,
                                                  static_cast<long long>(Frame::Width)));
    const int bottom = static_cast<int>(std::clamp(static_cast<long long>(q.y) + q.height, 0LL,
                                                   static_cast<long long>(Frame::Height)));
    const auto color = 0xff000000U | (static_cast<unsigned>(q.color.r) << 16U) |
                       (static_cast<unsigned>(q.color.g) << 8U) | q.color.b;
    for (int y = top; y < bottom; ++y)
        if (!q.outline || y == q.y ||
            static_cast<long long>(y) == static_cast<long long>(q.y) + q.height - 1)
            std::fill(pixels_.begin() + y * Frame::Width + left,
                      pixels_.begin() + y * Frame::Width + right, color);
        else {
            if (q.x >= 0 && q.x < Frame::Width)
                pixels_[static_cast<std::size_t>(y * Frame::Width + q.x)] = color;
            const auto edge = static_cast<long long>(q.x) + q.width - 1;
            if (edge >= 0 && edge < Frame::Width)
                pixels_[static_cast<std::size_t>(y * Frame::Width + edge)] = color;
        }
}

void RasterCanvas::render(const Frame &frame) {
    std::fill(pixels_.begin(), pixels_.end(), 0xff0a0f17U);
    for (const auto &value : frame.quads)
        quad(value);
    for (const auto &label : frame.labels)
        drawLabel(label, [this](const Quad &value) { quad(value); });
}

const std::vector<std::uint32_t> &RasterCanvas::pixels() const noexcept {
    return pixels_;
}

std::vector<std::uint32_t> RasterCanvas::scaled(int width, int height) const {
    if (width < 1 || height < 1 || width > 8192 || height > 8192)
        throw std::runtime_error("display dimensions outside 1..8192");
    std::vector<std::uint32_t> result(
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height), 0xff0a0f17U);
    const double scale = std::min(static_cast<double>(width) / Frame::Width,
                                  static_cast<double>(height) / Frame::Height);
    const int w = std::max(1, static_cast<int>(Frame::Width * scale));
    const int h = std::max(1, static_cast<int>(Frame::Height * scale));
    const int left = (width - w) / 2, top = (height - h) / 2;
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            result[static_cast<std::size_t>((y + top) * width + x + left)] =
                pixels_[static_cast<std::size_t>((y * Frame::Height / h) * Frame::Width +
                                                 x * Frame::Width / w)];
    return result;
}
} // namespace arcade::bonus
