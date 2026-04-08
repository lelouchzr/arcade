#pragma once

#include "../../bonus/legacy/sdk/IGraphic.hpp"

namespace arcade::test {

class GraphicFixture final : public Arcade::SharedInterfaces::IGraphic {
  public:
    GraphicFixture();
    ~GraphicFixture() override;
    Arcade::Types::GraphicType getType() const override;
    bool createSprite(Arcade::Types::SpriteType, const Arcade::Types::string &) override;
    bool createSprite(Arcade::Types::SpriteType, Arcade::Types::char8, Arcade::Types::BackColor,
                      Arcade::Types::ForeColor) override;
    void clear() override;
    Arcade::Types::event_pool getEventPool() override;
    void display() override;
    void drawTile(Arcade::Types::SpriteType, Arcade::Types::i32, Arcade::Types::i32,
                  Arcade::Types::i32, Arcade::Types::i32) override;
    void drawGameName(const Arcade::Types::string &) override;
    void drawPlayerName(const Arcade::Types::string &) override;
    void drawScore(Arcade::Types::i32) override;
    void drawHighScore(Arcade::Types::i32) override;
    void drawTimeLeft(Arcade::Types::i32) override;
    void drawLives(Arcade::Types::i16) override;
    void drawText(const Arcade::Types::string &, Arcade::Types::i32, Arcade::Types::i32) override;
};

} // namespace arcade::test
