#include "GraphicFixture.hpp"
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace {

void log(const std::string &line) {
    if (const char *path = std::getenv("ARCADE_LEGACY_GRAPHIC_LOG")) {
        std::ofstream file(path, std::ios::app);
        file << line << '\n';
    }
}

} // namespace

namespace arcade::test {

GraphicFixture::GraphicFixture() {
    log("CREATE");
}

GraphicFixture::~GraphicFixture() {
    log("DESTROY");
}

Arcade::Types::GraphicType GraphicFixture::getType() const {
    return Arcade::Types::GraphicType::TUI;
}

bool GraphicFixture::createSprite(Arcade::Types::SpriteType, const Arcade::Types::string &) {
    return false;
}

bool GraphicFixture::createSprite(Arcade::Types::SpriteType type, char glyph,
                                  Arcade::Types::BackColor, Arcade::Types::ForeColor) {
    log("SPRITE " + std::to_string(static_cast<int>(type)) + " " + glyph);
    return std::getenv("ARCADE_LEGACY_TEXT_ONLY") == nullptr;
}

void GraphicFixture::clear() {
    log("CLEAR");
}

Arcade::Types::event_pool GraphicFixture::getEventPool() {
    const char *value = std::getenv("ARCADE_LEGACY_EVENTS");
    if (!value)
        return {};
    std::istringstream stream(value);
    unsetenv("ARCADE_LEGACY_EVENTS");
    std::string token;
    Arcade::Types::event_pool events;
    using E = Arcade::Types::EventType;
    while (stream >> token) {
        if (token == "UP")
            events.push_back(E::UP);
        if (token == "DOWN")
            events.push_back(E::DOWN);
        if (token == "LEFT")
            events.push_back(E::LEFT);
        if (token == "RIGHT")
            events.push_back(E::RIGHT);
        if (token == "G")
            events.push_back(E::GRAPHIC_NEXT);
        if (token == "B")
            events.push_back(E::GAME_NEXT);
        if (token == "SPACE")
            events.push_back(E::RESET);
        if (token == "M")
            events.push_back(E::MENU);
        if (token == "ESC")
            events.push_back(E::EXIT);
        if (token == "H")
            events.push_back(E::ACTION_1);
        if (token == "J")
            events.push_back(E::ACTION_2);
        if (token == "K")
            events.push_back(E::ACTION_3);
        if (token == "L")
            events.push_back(E::ACTION_4);
    }
    return events;
}

void GraphicFixture::display() {
    log("DISPLAY");
}

void GraphicFixture::drawTile(Arcade::Types::SpriteType, int x, int y, int height, int width) {
    log("TILE " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(width) + "x" +
        std::to_string(height));
}

void GraphicFixture::drawGameName(const std::string &value) {
    log("GAME " + value);
}

void GraphicFixture::drawPlayerName(const std::string &value) {
    log("PLAYER " + value);
}

void GraphicFixture::drawScore(int value) {
    log("SCORE " + std::to_string(value));
}

void GraphicFixture::drawHighScore(int value) {
    log("BEST " + std::to_string(value));
}

void GraphicFixture::drawTimeLeft(int value) {
    log("TIME " + std::to_string(value));
}

void GraphicFixture::drawLives(short value) {
    log("LIVES " + std::to_string(value));
}

void GraphicFixture::drawText(const std::string &value, int x, int y) {
    log("TEXT " + std::to_string(x) + " " + std::to_string(y) + " " + value);
}

} // namespace arcade::test

extern "C" Arcade::SharedInterfaces::IGraphic *entryPoint() {
    return new arcade::test::GraphicFixture();
}

extern "C" void createGraphic() {}
