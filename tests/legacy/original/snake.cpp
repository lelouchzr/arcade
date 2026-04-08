#include "snake.hpp"

using namespace Arcade;

extern "C"
{
    Snake *entryPoint()
    {
        return new Snake();
    }
    void createGame()
    {
    }
}


// ---------------------------------------------------------------
// CONSTUCTOR - DESTRUCTOR

Snake::Snake()
{
    GameMap = generateMap();
}

Snake::~Snake()
{

}


// ---------------------------------------------------------------
// GAME FUNCTIONS

Types::map_t Snake::generateMap()
{
    Types::map_t map(20, std::vector<Types::SpriteType>(20, Types::SpriteType::FRUIT));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 18);

    for (int i = 0; i < 20; ++i) {
        map[0][i] = Types::SpriteType::WALL;
        map[19][i] = Types::SpriteType::WALL;
        map[i][0] = Types::SpriteType::WALL;
        map[i][19] = Types::SpriteType::WALL;
    }

    for (int i = 1; i < 19; ++i) {
        for (int j = 1; j < 19; ++j) {
            map[i][j] = Types::SpriteType::EMPTY;
        }
    }

    map[10][10] = Types::SpriteType::S_HEAD;
    map[13][10] = Types::SpriteType::S_BODY;
    bodyPositions.emplace_back(10, 13);
    map[12][10] = Types::SpriteType::S_BODY;
    bodyPositions.emplace_back(10, 12);
    map[11][10] = Types::SpriteType::S_BODY;
    bodyPositions.emplace_back(10, 11);

    for (int i = 0; i < 5; ++i) {
        int x = dis(gen);
        int y = dis(gen);
        if (map[x][y] == Types::SpriteType::EMPTY) {
            map[x][y] = Types::SpriteType::FRUIT;
        }
    }

    return map;
}

void Arcade::Snake::update(const Arcade::Types::event_pool &events, Arcade::Types::tick_t tick)
{
    if (IsRunning == true) {
        // mvprintw(20, 20, " YOU WIN");
        return;
    }
    if (tick != 0)
        return;
    if (getstarted() == false && events.empty())
        return;
    setstarted(true);
    if (getLives() <= 0) {
        // mvprintw(20, 20, " YOU LOOSE");
        return;
    }
    bool berriesOrEmptiesLeft = false;
    for (size_t y = 0; y < GameMap.size(); ++y) {
        for (size_t x = 0; x < GameMap[y].size(); ++x) {
            if (GameMap[y][x] == Types::SpriteType::FRUIT || GameMap[y][x] == Types::SpriteType::EMPTY) {
                berriesOrEmptiesLeft = true;
                break;
            }
        }
        if (berriesOrEmptiesLeft) {
            break;
        }
    }

    if (!berriesOrEmptiesLeft) {
        IsRunning = true;
        return;
    }
    Arcade::Types::event_pool currentEvents;
    if (events.empty()) {
        currentEvents = last_event;
    } else if (!last_event.empty() && ((events[0] == Types::EventType::LEFT && last_event[0] == Types::EventType::RIGHT) ||
    (events[0] == Types::EventType::RIGHT && last_event[0] == Types::EventType::LEFT) ||
    (events[0] == Types::EventType::UP && last_event[0] == Types::EventType::DOWN) ||
    (events[0] == Types::EventType::DOWN && last_event[0] == Types::EventType::UP))) {
        currentEvents = last_event;
    } else if (last_event.empty() && events[0] == Types::EventType::DOWN) {
        return;
    } else if (!events.empty()) {
        last_event = events;
        currentEvents = events;
    }
    for (const auto& event : currentEvents) {
        moveInDirection(event);
    }
}

void Snake::moveInDirection(const Arcade::Types::EventType event)
{
    std::size_t x = getPlayerX();
    std::size_t y = getPlayerY();
    int BodyX = x;
    int BodyY = y;

    switch (event) {
        case Arcade::Types::EventType::UP:
            if (y > 0)
                y -= 1;
            Empty(BodyX, BodyY, x, y);
            Berry(BodyX, BodyY, x, y);
            WallnBody(BodyX, BodyY, x, y);
            break;
        case Arcade::Types::EventType::DOWN:
            if (y < GameMap.size() - 1)
                y += 1;
            Empty(BodyX, BodyY, x, y);
            Berry(BodyX, BodyY, x, y);
            WallnBody(BodyX, BodyY, x, y);
            break;
        case Arcade::Types::EventType::LEFT:
            if (x > 0)
                x -= 1;
            Empty(BodyX, BodyY, x, y);
            Berry(BodyX, BodyY, x, y);
            WallnBody(BodyX, BodyY, x, y);
            break;
        case Arcade::Types::EventType::RIGHT:
            if (x < GameMap[0].size() - 1)
                x += 1;
            Empty(BodyX, BodyY, x, y);
            Berry(BodyX, BodyY, x, y);
            WallnBody(BodyX, BodyY, x, y);
            break;
        default:
            break;
    }
}

void Snake::WallnBody(int BodyX, int BodyY, int x, int y)
{
    if (GameMap[y][x] == Arcade::Types::SpriteType::S_BODY) {
        std::list<std::pair<int, int>> copy = bodyPositions;
        for (const auto& pos : bodyPositions) {
            int bodyX = pos.first;
            int bodyY = pos.second;
            GameMap[bodyY][bodyX] = Types::SpriteType::EMPTY;
        }
        bodyPositions.pop_front();
        bodyPositions.emplace_back(BodyX, BodyY);
        for (const auto& pos : bodyPositions) {
            int bodyX = pos.first;
            int bodyY = pos.second;
            GameMap[bodyY][bodyX] = Types::SpriteType::S_BODY;
        }
        if (GameMap[y][x] == Arcade::Types::SpriteType::S_BODY) {
            setLives(getLives() - 1);
            return;
        }
        GameMap[y][x] = Types::SpriteType::S_HEAD;
    }
    if (GameMap[y][x] == Arcade::Types::SpriteType::WALL) {
        setLives(getLives() - 1);
        return;
    }
}

void Snake::Berry(int BodyX, int BodyY, int x, int y)
{
    if (GameMap[y][x] == Arcade::Types::SpriteType::FRUIT) {
        setScore(getScore() + 100);
        GameMap[y][x] = Arcade::Types::SpriteType::S_HEAD;
        GameMap[BodyY][BodyX] = Arcade::Types::SpriteType::S_BODY;
        bodyPositions.emplace_back(BodyX, BodyY);
        int unknownCount = 0;
        for (const auto& row : GameMap) {
            for (const auto& cell : row) {
                if (cell == Arcade::Types::SpriteType::FRUIT) {
                    unknownCount++;
                    if (unknownCount >= 2) {
                        std::random_device rd;
                        std::mt19937 gen(rd());
                        std::uniform_int_distribution<int> dis(1, 18);
                        int randX = dis(gen);
                        int randY = dis(gen);
                        if (GameMap[randX][randY] == Arcade::Types::SpriteType::EMPTY) {
                            GameMap[randX][randY] = Arcade::Types::SpriteType::FRUIT;
                        }
                        break;
                    }
                }
            }
            if (unknownCount >= 2)
                break;
        }
        if (unknownCount < 2) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dis(1, 18);
            int randX = dis(gen);
            int randY = dis(gen);
            if (GameMap[randX][randY] == Arcade::Types::SpriteType::EMPTY) {
                GameMap[randX][randY] = Arcade::Types::SpriteType::FRUIT;
            }
        }
    }
}

void Snake::Empty(int BodyX, int BodyY, int x, int y)
{
    if (GameMap[y][x] == Types::SpriteType::EMPTY) {
        GameMap[y][x] = Types::SpriteType::S_HEAD;
        for (const auto& pos : bodyPositions) {
            int bodyX = pos.first;
            int bodyY = pos.second;
            GameMap[bodyY][bodyX] = Types::SpriteType::EMPTY;
        }
        bodyPositions.pop_front();
        bodyPositions.emplace_back(BodyX, BodyY);
        for (const auto& pos : bodyPositions) {
            int bodyX = pos.first;
            int bodyY = pos.second;
            GameMap[bodyY][bodyX] = Types::SpriteType::S_BODY;
        }
    }
}


// ---------------------------------------------------------------
// SETTERS

void Snake::setLast_Pool(Types::event_pool event)
{
    last_event = event;
}

void Snake::setstarted(bool start)
{
    started = start;
}

void Snake::setRunning(bool running)
{
    IsRunning = running;
}


void Snake::SetPlayerStart() {
    for (size_t y = 0; y < GameMap.size(); ++y) {
        for (size_t x = 0; x < GameMap[y].size(); ++x) {
            if (GameMap[y][x] == Types::SpriteType::S_HEAD || GameMap[y][x] == Types::SpriteType::S_BODY) {
                GameMap[y][x] = Types::SpriteType::EMPTY;
            }
        }
    }
    bodyPositions.clear();
    GameMap[10][10] = Types::SpriteType::S_HEAD;
    GameMap[13][10] = Types::SpriteType::S_BODY;
    bodyPositions.emplace_back(10, 13);
    GameMap[12][10] = Types::SpriteType::S_BODY;
    bodyPositions.emplace_back(10, 12);
    GameMap[11][10] = Types::SpriteType::S_BODY;
    bodyPositions.emplace_back(10, 11);
}

void Snake::setPlayerName(const Types::string &name)
{
    PlayerName = name;
}

void Snake::setScore (Types::i32 new_Score)
{
    score = new_Score;
}

void Snake::setLives(Types::i32 lives)
{
    life = lives;
}


// ---------------------------------------------------------------
// GETTERS

Types::event_pool Snake::getLast_Pool() const
{
    return last_event;
}

Types::string Snake::getGameName() const
{
    return GameName;
}

Types::string Snake::getPlayerName() const
{
    return PlayerName;
}

Types::i32 Snake::getHighScore() const
{
    return highScore;
}

Types::i32 Snake::getScore() const
{
    return score;
}

Types::i16 Snake::getLives() const
{
    return life; 
}

Types::i32 Snake::getTimeLeft() const
{
    return 0;
}

Types::map_t Snake::getMap() const
{
    return GameMap;
}

bool Snake::isGameRunning() const
{
    return IsRunning;
}

bool Snake::getstarted()
{
    return  started;
}

int Snake::getPlayerX()
{
    for (std::size_t y = 0; y < GameMap.size(); ++y) {
        for (std::size_t x = 0; x < GameMap[y].size(); ++x) {
            if (GameMap[y][x] == Types::SpriteType::S_HEAD) {
                return(x);
            }
        }
    }
    return -1;
}

int Snake::getPlayerY()
{
    for (std::size_t y = 0; y < GameMap.size(); ++y) {
        for (std::size_t x = 0; x < GameMap[y].size(); ++x) {
            if (GameMap[y][x] == Types::SpriteType::S_HEAD) {
                return(y);
            }
        }
    }
    return -1;
}
