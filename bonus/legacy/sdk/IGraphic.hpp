#pragma once

#include "ArcadeData.hpp"

namespace Arcade::SharedInterfaces
{
    /**
     * @brief Interface for the graphic library
     * 
     */
    class IGraphic
    {
    public:
        /**
         * @brief Destroy the IGraphic object
         * 
         */
        virtual ~IGraphic() = default;

        /**
         * @brief Returns whether the graphic library is a Graphical User Interface (GUI) or a Text-based User Interface (TUI)
         * 
         * @return Types::GraphicType::GUI or Types::GraphicType::TUI
         */
        virtual Types::GraphicType getType() const = 0;

        /**
         * @brief Create a Sprite object for GUI graphic libraries
         * 
         * @param type The type of the new sprite
         * @param path The path to the texture
         * @return true if the new sprite is created
         * @return false if the new sprite is not created
         */
        virtual bool createSprite(Types::SpriteType type, const Types::string &path) = 0;
        /**
         * @brief Create a Sprite object for TUI graphic libraries
         * 
         * @param type The type of the new sprite
         * @param c The character to use for the sprite
         * @param backgroundColor The background color of the character
         * @param foregroundColor The color of the character
         * @return true if the new sprite is created
         * @return false if the new sprite is not created
         */
        virtual bool createSprite(Types::SpriteType type, Types::char8 c, Types::BackColor backgroundColor, Types::ForeColor foregroundColor) = 0;

        /**
         * @brief Calls the graphical library's 'screen clear' function
         * 
         */
        virtual void clear() = 0;
        /**
         * @brief Get the list of events
         * 
         * @return A vector containing the events
         */
        virtual Types::event_pool getEventPool() = 0;
        /**
         * @brief Needs to be called at the very end after ALL the calls to IGraphic::drawTile(), IGraphic::drawGameName(), IGraphic::drawPlayerName(), IGraphic::drawScore() and IGraphic::drawHighScore()
         * 
         */
        virtual void display() = 0;
        /**
         * @brief Draw a sprite onto the screen at a give position
         * 
         * @param type The type of the new sprite
         * @param row The row that the sprite will be drawn at
         * @param col The column that the sprite will be drawn at
         * @param maxRow The maximum number of rows on the map
         * @param maxCol The maximum number of columns on the map
         */
        virtual void drawTile(Types::SpriteType type, Types::i32 row, Types::i32 col, Types::i32 maxRow, Types::i32 maxCol) = 0;
        
        /**
         * @brief Write the game name on the screen
         * 
         * @param gameName The name of the game
         */
        virtual void drawGameName(const Types::string &gameName) = 0;
        /**
         * @brief Write the player name on the screen
         * 
         * @param playerName The name of the player
         */
        virtual void drawPlayerName(const Types::string &playerName) = 0;
        /**
         * @brief Write the score on the screen
         * 
         * @param score The score of the player
         */
        virtual void drawScore(Types::i32 score) = 0;
        /**
         * @brief Write the high score on the screen
         * 
         * @param highScore The high score of the player
         */
        virtual void drawHighScore(Types::i32 highScore) = 0;
        /**
         * @brief Write the time left on the screen
         * 
         * @param timeLeft 
         */
        virtual void drawTimeLeft(Types::i32 timeLeft) = 0;
        /**
         * @brief Write the lives left on the screen
         * 
         * @param lives 
         */
        virtual void drawLives(Types::i16 lives) = 0;
        /**
         * @brief Write text at given position
         * 
         * @param text
         * @param row
         * @param col
         */
        virtual void drawText(const Types::string &text, Types::i32 row, Types::i32 col) = 0;
    };
}
