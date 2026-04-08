#pragma once

#include "ArcadeData.hpp"

namespace Arcade::SharedInterfaces
{
    /**
     * @brief Interface for the game library
     * 
     */
    class IGame
    {
    public:
        /**
         * @brief Destroy the IGame object
         * 
         */
        virtual ~IGame() = default;

        /**
        * @brief Returns whether the current game is running or not
         * 
         * @return true if the program is running
         * @return false if the program is not running
         */
        virtual bool isGameRunning() const = 0;
        /**
         * @brief Update the internal state of the game
         * 
         * @param events The list of events, returned by IGraphic::getEventPool()
         * @param tick The current tick, can used to limit the number of updates per second
         */
        virtual void update(const Types::event_pool &events, Types::tick_t tick) = 0;
        
        /**
         * @brief Get the map of the game
         * 
         * @return A 2D vector of SpriteType representing the map
         */
        virtual Types::map_t getMap() const = 0;
        /**
         * @brief Get the name of the game
         * 
         * @return The name of the game as a string
         */
        virtual Types::string getGameName() const = 0;
        /**
         * @brief Get the name of the player
         * 
         * @return The name of the player as a string
         */
        virtual Types::string getPlayerName() const = 0;
        /**
         * @brief Get the score of the player
         * 
         * @return The score of the player as an int
         */
        virtual Types::i32 getScore() const = 0;
        /**
         * @brief Get the highscore of the game
         * 
         * @return The highscore of the game as an int
         */
        virtual Types::i32 getHighScore() const = 0;
        /**
         * @brief Get the time left to complete the game
         * 
         * @return The time left of the game as an int
         */
        virtual Types::i32 getTimeLeft() const = 0;
        /**
         * @brief Get the lives of the player
         * 
         * @return The lives of the player as an int
         */
        virtual Types::i16 getLives() const = 0;

        /**
         * @brief Set the name of the player
         * 
         * @param name The name of the player
         */
        virtual void setPlayerName(const Types::string &name) = 0;
    };
}
