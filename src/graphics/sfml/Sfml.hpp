#pragma once
#include "arcade/Api.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

namespace arcade {
class Sfml final : public IGraphics {
  public:
    ~Sfml() override;
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    std::unique_ptr<sf::RenderWindow> window_;
    static void addKey(sf::Keyboard::Key key, std::vector<Input> &result);
};
} // namespace arcade
