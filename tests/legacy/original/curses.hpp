#pragma once

#include "IGraphic.hpp"
#include <curses.h>
#include <map>

namespace Arcade
{
    class Ncurses : public SharedInterfaces::IGraphic
    {
        public:
            // Constructor - Destructor
            Ncurses();
            ~Ncurses() override;

            // Graphic functions
            void clear() override;
            void display() override;

            // Create functions
            bool createSprite(Types::SpriteType type, const Types::string &path) override;
            bool createSprite(Types::SpriteType type, Types::char8 c, Types::BackColor backgroundColor, Types::ForeColor foregroundColor) override;

            // Draw functions
            void drawTile(Types::SpriteType type, Types::i32 row, Types::i32 col, Types::i32 maxRow, Types::i32 maxCol) override;
            void drawGameName(const Types::string &gameName) override;
            void drawPlayerName(const Types::string &playerName) override;
            void drawScore(Types::i32 score) override;
            void drawHighScore(Types::i32 highScore) override;
            void drawTimeLeft(Types::i32 timeLeft) override;
            void drawLives(Types::i16 lives) override;
            void drawText(const Types::string &text, Types::i32 row, Types::i32 col) override;

            // Getters
            Types::GraphicType getType() const override;
            Types::event_pool getEventPool() override;

        private:
            struct SpriteColor {
                Types::SpriteType type;
                Types::char8 character;
                Types::BackColor backgroundColor;
                Types::ForeColor foregroundColor;
            };

            std::map<Types::SpriteType, SpriteColor> map = {};
            Types::GraphicType Type;
    };
}
