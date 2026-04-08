#include "curses.hpp"

using namespace Arcade;

extern "C"
{
    Ncurses *entryPoint()
    {
        return new Ncurses();
    }
    void createGraphic()
    {
    }
}


// ---------------------------------------------------------------
// CONSTUCTOR - DESTRUCTOR

Ncurses::Ncurses() : Type(Types::GraphicType::GUI)
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    timeout(0);
    clear();
    refresh();
}

Ncurses::~Ncurses()
{
    endwin();
}


// ---------------------------------------------------------------
// GRAPHIC FUNCTIONS

void Ncurses::clear()
{
    wclear(stdscr);
    wrefresh(stdscr);
}

void Ncurses::display()
{
    refresh();
}


// ---------------------------------------------------------------
// CREATE FUNCTIONS

 // Not used as it is for other graphic library
bool Ncurses::createSprite([[maybe_unused]] Types::SpriteType type, [[maybe_unused]] const Types::string &path)
{
    return false;
}

bool Ncurses::createSprite(Types::SpriteType type, Types::char8 c, Types::BackColor backgroundColor, Types::ForeColor foregroundColor)
{
    map[type] = {type, c, backgroundColor, foregroundColor};
    return true;
}


// ---------------------------------------------------------------
// DRAW FUNCTIONS

void Ncurses::drawTile(Types::SpriteType type, Types::i32 row, Types::i32 col, [[maybe_unused]] Types::i32 maxRow, [[maybe_unused]] Types::i32 maxCol)
{
    int max_col, max_row, ncursesBackColor, ncursesForeColor;
    switch (map[type].backgroundColor) {
        case Types::BackColor::NONE:
            ncursesBackColor = COLOR_BLACK;
            break;
        case Types::BackColor::BLACK:
            ncursesBackColor = COLOR_BLACK;
            break;
        case Types::BackColor::RED:
            ncursesBackColor = COLOR_RED;
            break;
        case Types::BackColor::GREEN:
            ncursesBackColor = COLOR_GREEN;
            break;
        case Types::BackColor::YELLOW:
            ncursesBackColor = COLOR_YELLOW;
            break;
        case Types::BackColor::BLUE:
            ncursesBackColor = COLOR_BLUE;
            break;
        case Types::BackColor::MAGENTA:
            ncursesBackColor = COLOR_MAGENTA;
            break;
        case Types::BackColor::CYAN:
            ncursesBackColor = COLOR_CYAN;
            break;
        case Types::BackColor::WHITE:
            ncursesBackColor = COLOR_WHITE;
            break;
        default:
            ncursesBackColor = COLOR_BLACK;
    }
    switch (map[type].foregroundColor) {
        case Types::ForeColor::NONE:
            ncursesForeColor = COLOR_WHITE;
            break;
        case Types::ForeColor::BLACK:
            ncursesForeColor = COLOR_BLACK;
            break;
        case Types::ForeColor::RED:
            ncursesForeColor = COLOR_RED;
            break;
        case Types::ForeColor::GREEN:
            ncursesForeColor = COLOR_GREEN;
            break;
        case Types::ForeColor::YELLOW:
            ncursesForeColor = COLOR_YELLOW;
            break;
        case Types::ForeColor::BLUE:
            ncursesForeColor = COLOR_BLUE;
            break;
        case Types::ForeColor::MAGENTA:
            ncursesForeColor = COLOR_MAGENTA;
            break;
        case Types::ForeColor::CYAN:
            ncursesForeColor = COLOR_CYAN;
            break;
        case Types::ForeColor::WHITE:
            ncursesForeColor = COLOR_WHITE;
            break;
        default:
            ncursesForeColor = COLOR_WHITE;
    }
    start_color();
    init_pair(static_cast<int>(type) + 1, ncursesForeColor, ncursesBackColor);
    attron(COLOR_PAIR(static_cast<int>(type) + 1));
    getmaxyx(stdscr, max_col, max_row);
    mvaddch((max_col / 2) + col - 15, (max_row / 2) + row - 15, map[type].character);
    attroff(COLOR_PAIR(static_cast<int>(type) + 1));
}

void Ncurses::drawGameName(const Types::string &gameName)
{
    mvprintw(0, 58, gameName.c_str());
    refresh();
}

void Ncurses::drawPlayerName(const Types::string &playerName)
{
    mvprintw(0, 1, ("Player name: " + playerName).c_str());
    refresh();
}

void Ncurses::drawScore(Types::i32 score)
{
    int maxY, maxX;

    getmaxyx(stdscr, maxY, maxX);
    mvprintw(std::min(0, maxY), maxX - std::to_string(score).length() - 8, "Score: %d", score);
    refresh();
}

void Ncurses::drawHighScore(Types::i32 highScore)
{
    int maxY, maxX;

    getmaxyx(stdscr, maxY, maxX);
    mvprintw(std::min(0, maxY), maxX - std::to_string(highScore).length() - 40, "High Score: %d", highScore);
}

void Ncurses::drawLives(Types::i16 lives)
{
    mvprintw(1, 1, "Lives: %d", lives);
}

void Ncurses::drawTimeLeft(Types::i32 timeLeft)
{
    mvprintw(2, 1, "Time left: %d", timeLeft);
}

void Ncurses::drawText(const Types::string &text, Types::i32 row, Types::i32 col)
{
    if (text == "")
        return;
    mvprintw(col, row, "%s", text.c_str());

}


// ---------------------------------------------------------------
// GETTERS

Types::GraphicType Ncurses::getType() const
{
    return Type;
}

Types::event_pool Ncurses::getEventPool()
{
    Types::event_pool events;
    int ch = getch();
    switch (ch) {
        case KEY_UP:
            events.push_back(Types::EventType::UP);
            break;
        case KEY_DOWN:
            events.push_back(Types::EventType::DOWN);
            break;
        case KEY_LEFT:
            events.push_back(Types::EventType::LEFT);
            break;
        case KEY_RIGHT:
            events.push_back(Types::EventType::RIGHT);
            break;
        case 27:
            events.push_back(Types::EventType::EXIT);
            break;
        case 'g':
            events.push_back(Types::EventType::GRAPHIC_NEXT);
            break;
        case 'b':
            events.push_back(Types::EventType::GAME_NEXT);
            break;
        case 'm':
            events.push_back(Types::EventType::MENU);
            break;
        case ' ':
            events.push_back(Types::EventType::RESET);
            break;
        case 'h':
            events.push_back(Types::EventType::ACTION_1);
            break;
        case 'j':
            events.push_back(Types::EventType::ACTION_2);
            break;
        case 'k':
            events.push_back(Types::EventType::ACTION_3);
            break;
        case 'l':
            events.push_back(Types::EventType::ACTION_4);
            break;
        default:
            break;
    }
    return events;
}
