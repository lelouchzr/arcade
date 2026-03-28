#pragma once
#include "Api.hpp"
#include <array>

namespace arcade {
// Original 5x7 pixel alphabet: no font files or additional font libraries needed.
std::array<unsigned char, 7> glyph(char character);

template <class DrawQuad> void drawLabel(const Label &label, DrawQuad draw) {
    int x = label.x;
    for (char character : label.text) {
        const auto bits = glyph(character);
        for (int y = 0; y < 7; ++y)
            for (int col = 0; col < 5; ++col)
                if ((bits[static_cast<std::size_t>(y)] & (1 << (4 - col))) != 0)
                    draw(Quad{x + col * label.scale, label.y + y * label.scale, label.scale,
                              label.scale, label.color, false});
        x += 6 * label.scale;
    }
}
} // namespace arcade
