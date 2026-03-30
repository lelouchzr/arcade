#include "Sfml.hpp"
#include "arcade/Font.hpp"
#include <SFML/Config.hpp>
#include <algorithm>

namespace arcade {

void Sfml::draw(const Frame &frame) {
    window_->clear(sf::Color(10, 15, 23));
    // A fixed logical view preserves layout under resize, with letterboxing.
    const auto size = window_->getSize();
    sf::View view;
#if SFML_VERSION_MAJOR >= 3
    view.setCenter({Frame::Width / 2.f, Frame::Height / 2.f});
    view.setSize({Frame::Width, Frame::Height});
    const float ratio = static_cast<float>(size.x) / static_cast<float>(std::max(1u, size.y));
    if (ratio > 1.6f)
        view.setViewport(sf::FloatRect({(1.f - 1.6f / ratio) / 2.f, 0}, {1.6f / ratio, 1}));
    else
        view.setViewport(sf::FloatRect({0, (1.f - ratio / 1.6f) / 2.f}, {1, ratio / 1.6f}));
#else
    view.setCenter(Frame::Width / 2.f, Frame::Height / 2.f);
    view.setSize(Frame::Width, Frame::Height);
    const float ratio = static_cast<float>(size.x) / static_cast<float>(std::max(1u, size.y));
    if (ratio > 1.6f)
        view.setViewport(sf::FloatRect((1.f - 1.6f / ratio) / 2.f, 0, 1.6f / ratio, 1));
    else
        view.setViewport(sf::FloatRect(0, (1.f - ratio / 1.6f) / 2.f, 1, ratio / 1.6f));
#endif
    window_->setView(view);
    // Batch every panel, tile and font pixel into one GPU submission.
    sf::VertexArray vertices(sf::PrimitiveType::Triangles);
    const auto fill = [&](int x, int y, int width, int height, Color ink) {
        const float left = static_cast<float>(x), top = static_cast<float>(y);
        const float right = static_cast<float>(x + width), bottom = static_cast<float>(y + height);
        const sf::Color color(ink.r, ink.g, ink.b);
        for (const auto position :
             {sf::Vector2f{left, top}, sf::Vector2f{right, top}, sf::Vector2f{right, bottom},
              sf::Vector2f{left, top}, sf::Vector2f{right, bottom}, sf::Vector2f{left, bottom}})
            vertices.append(sf::Vertex{position, color});
    };
    const auto quad = [&](const Quad &q) {
        if (q.outline) {
            fill(q.x, q.y, q.width, 1, q.color);
            fill(q.x, q.y + q.height - 1, q.width, 1, q.color);
            fill(q.x, q.y, 1, q.height, q.color);
            fill(q.x + q.width - 1, q.y, 1, q.height, q.color);
        } else
            fill(q.x, q.y, q.width, q.height, q.color);
    };
    for (const auto &q : frame.quads)
        quad(q);
    for (const auto &label : frame.labels)
        drawLabel(label, quad);
    window_->draw(vertices);
    window_->display();
}

} // namespace arcade
