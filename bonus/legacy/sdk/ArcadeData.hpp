#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace Arcade::Types
{
    using string = std::string;
    using char8 = char;

    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    using tick_t = Types::i64;

    /**
     * @brief The graphic library type, returned by IGraphic::getType()
     * 
     */
    enum class GraphicType
    {
        GUI,
        TUI,
    };

    /**
     * @brief The event type, returned in a vector by IGraphic::getEventPool()
     * 
     */
    enum class EventType
    {
        NONE,

        GRAPHIC_NEXT,
        GAME_NEXT,
        RESET,
        MENU,
        EXIT,

        UP,
        DOWN,
        LEFT,
        RIGHT,
        ACTION_1,
        ACTION_2,
        ACTION_3,
        ACTION_4,
    };

    /**
     * @brief The sprite type, used by IGraphic::createSprite()
     * 
     */
    enum class SpriteType
    {
        // COMMON

        EMPTY,
        WALL,
        FRUIT,

        // SNAKE

        S_HEAD,
        S_BODY,

        // NIBBLER

        N_HEAD,
        N_BODY,

        // MISC

        UNKNOWN,
        MAX_NB_SPRITE,
    };

    using map_t = std::vector<std::vector<Types::SpriteType>>;
    
    /**
     * @brief The background color of a character in a TUI, used by IGraphic::createSprite()
     * 
     */
    enum class BackColor
    {
        NONE = -1,
        BLACK,
        RED,
        GREEN,
        YELLOW,
        BLUE,
        MAGENTA,
        CYAN,
        WHITE,
    };
    /**
     * @brief The foreground color of a character in a TUI, used by IGraphic::createSprite()
     * 
     */
    enum class ForeColor
    {
        NONE = -1,
        BLACK,
        RED,
        GREEN,
        YELLOW,
        BLUE,
        MAGENTA,
        CYAN,
        WHITE,
    };

    using event_pool = std::vector<Types::EventType>;
}
