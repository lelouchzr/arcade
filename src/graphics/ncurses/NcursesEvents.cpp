#include "Ncurses.hpp"
#include "arcade/Keys.hpp"

namespace arcade {

std::vector<Input> Ncurses::poll() {
    std::vector<Input> result;
    for (int key = getch(); key != ERR; key = getch()) {
        Input input;
        switch (key) {
        case KEY_UP:
            input = {Action::Up};
            break;
        case KEY_DOWN:
            input = {Action::Down};
            break;
        case KEY_LEFT:
            input = {Action::Left};
            break;
        case KEY_RIGHT:
            input = {Action::Right};
            break;
        case KEY_F(2):
            input = {Action::NextGraphics};
            break;
        case KEY_F(3):
            input = {Action::NextGame};
            break;
        case KEY_F(4):
            input = {Action::Restart};
            break;
        case KEY_F(5):
            input = {Action::Menu};
            break;
        case KEY_F(6):
            input = {Action::Pause};
            break;
        case KEY_F(7):
            input = {Action::Difficulty};
            break;
        case KEY_BACKSPACE:
            input = {Action::Backspace};
            break;
        case KEY_RESIZE:
            continue;
        default:
            input = asciiInput(key);
        }
        if (input.action != Action::None)
            result.push_back(input);
    }
    return result;
}

} // namespace arcade
