#pragma once

#include "IGame.hpp"
#include <vector>
#include <random>
#include <list>

namespace Arcade
{
    class Snake : public SharedInterfaces::IGame
    {
        public:
            // Constructor - Destructor
            Snake();
            ~Snake() override;

            // Game functions
            Types::map_t generateMap();
            void update(const Types::event_pool& events, Types::tick_t tick) override;
            void moveInDirection(const Arcade::Types::EventType event);
            void WallnBody(int BodyX, int BodyY, int x, int y);
            void Berry(int BodyX, int BodyY, int x, int y);
            void Empty(int BodyX, int BodyY, int x, int y);

            // Setters
            void setLast_Pool(Types::event_pool event);
            void setstarted(bool start);
            void setRunning(bool running);
            void SetPlayerStart();
            void setPlayerName(const Types::string &name) override;
            void setScore (Types::i32 new_Score);
            void setLives(Types::i32 lives);

            // Getters
            Types::event_pool getLast_Pool() const;
            Types::string getGameName() const;
            Types::string getPlayerName() const override;
            Types::i32 getHighScore() const;
            Types::i32 getScore() const override;
            Types::i16 getLives() const override;
            Types::i32 getTimeLeft() const override;
            Types::map_t getMap() const override;
            bool isGameRunning() const;
            bool getstarted();
            int getPlayerX();
            int getPlayerY();

        private:
            bool IsRunning = false;
            bool started = false;
            Types::string GameName = "Snake";
            Types::map_t GameMap = {};
            Types::string PlayerName = "DefaultPlayer";
            Types::i32 score = 0;
            Types::i32 highScore = 0;
            Types::i16 life = 1;
            std::list<std::pair<int, int>> bodyPositions = {};
            Types::event_pool last_event = {};
    };
}
