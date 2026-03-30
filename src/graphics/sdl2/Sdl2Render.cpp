#include "Sdl2.hpp"
#include "arcade/Font.hpp"

namespace arcade {

void Sdl2::draw(const Frame &frame) {
    SDL_SetRenderDrawColor(renderer_, 10, 15, 23, 255);
    SDL_RenderClear(renderer_);
    auto quad = [&](const Quad &q) {
        SDL_SetRenderDrawColor(renderer_, q.color.r, q.color.g, q.color.b, 255);
        SDL_Rect rectangle{q.x, q.y, q.width, q.height};
        if (q.outline)
            SDL_RenderDrawRect(renderer_, &rectangle);
        else
            SDL_RenderFillRect(renderer_, &rectangle);
    };
    for (const auto &q : frame.quads)
        quad(q);
    for (const auto &label : frame.labels)
        drawLabel(label, quad);
    SDL_RenderPresent(renderer_);
}

} // namespace arcade
