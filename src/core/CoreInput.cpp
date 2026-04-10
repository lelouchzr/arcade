#include "Core.hpp"
#include <cctype>

namespace arcade {
namespace {
Input gameplayInput(Input input) {
    if (input.action != Action::Text)
        return input;
    switch (std::tolower(static_cast<unsigned char>(input.text))) {
    case 'w':
        return {Action::Up};
    case 's':
        return {Action::Down};
    case 'a':
        return {Action::Left};
    case 'd':
        return {Action::Difficulty};
    case 'f':
        return {Action::Secondary};
    case '?':
        return {Action::Secondary, '?'};
    case 'z':
        return {Action::Jump};
    case 'v':
        return {Action::Camera};
    case 'i':
        return {Action::PlayerTwoUp};
    case 'k':
        return {Action::PlayerTwoDown};
    case 'j':
        return {Action::PlayerTwoLeft};
    case 'l':
        return {Action::PlayerTwoRight};
    case 'h':
        return {Action::PlayerTwoFire};
    case 'p':
        return {Action::Pause};
    case 'r':
        return {Action::Restart};
    case 'm':
        return {Action::Menu};
    case 'g':
        return {Action::NextGraphics};
    case 'b':
        return {Action::NextGame};
    default:
        return input;
    }
}
} // namespace

std::vector<Input> Core::process(const std::vector<Input> &inputs) {
    std::vector<Input> gameInputs;
    for (auto input : inputs) {
        if (!menu_)
            input = gameplayInput(input);
        switch (input.action) {
        case Action::Quit:
            saveScore();
            running_ = false;
            break;
        case Action::NextGraphics:
            cycleGraphics();
            break;
        case Action::NextGame:
            if (menu_)
                selectedGame_ = (selectedGame_ + 1) % games_.size();
            else
                startGame((selectedGame_ + 1) % games_.size());
            gameInputs.clear();
            break;
        case Action::Menu:
            saveScore();
            menu_ = true;
            paused_ = false;
            gameInputs.clear();
            break;
        case Action::Restart:
            if (game_) {
                startGame(selectedGame_);
                gameInputs.clear();
            }
            break;
        case Action::Pause:
            if (!menu_)
                paused_ = !paused_;
            break;
        default:
            if (menu_) {
                switch (input.action) {
                case Action::Up:
                    selectedGame_ = (selectedGame_ + games_.size() - 1) % games_.size();
                    break;
                case Action::Down:
                    selectedGame_ = (selectedGame_ + 1) % games_.size();
                    break;
                case Action::Left:
                    cycleGraphics(true);
                    break;
                case Action::Right:
                    cycleGraphics();
                    break;
                case Action::Confirm:
                    startGame(selectedGame_);
                    gameInputs.clear();
                    break;
                case Action::Backspace:
                    if (!player_.empty())
                        player_.pop_back();
                    break;
                case Action::Text:
                    if (player_.size() < 16 &&
                        (std::isalnum(static_cast<unsigned char>(input.text)) ||
                         input.text == '_' || input.text == '-'))
                        player_ += input.text;
                    break;
                default:
                    break;
                }
            } else
                gameInputs.push_back(input);
            break;
        }
    }
    return gameInputs;
}

} // namespace arcade
