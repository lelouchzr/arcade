#include "LegacyGraphics.hpp"
#include <string_view>

namespace arcade::legacy {

void LegacyGraphics::editName(int change, std::vector<Input> &inputs) {
    constexpr std::string_view alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
    if (player_.empty())
        player_ = "PLAYER";
    letter_ %= player_.size();
    auto index = alphabet.find(player_[letter_]);
    if (index == std::string_view::npos)
        index = 0;
    const int next = (static_cast<int>(index) + change + static_cast<int>(alphabet.size())) %
                     static_cast<int>(alphabet.size());
    player_[letter_] = alphabet[static_cast<std::size_t>(next)];
    inputs.insert(inputs.end(), 16, Input{Action::Backspace});
    for (char character : player_)
        inputs.push_back({Action::Text, character});
}

std::vector<Input> LegacyGraphics::poll() {
    if (!display_)
        throw std::runtime_error("historical graphics is not open");
    std::vector<Input> result;
    for (const auto event : display_->getEventPool()) {
        using E = Arcade::Types::EventType;
        switch (event) {
        case E::UP:
            result.push_back({Action::Up});
            break;
        case E::DOWN:
            result.push_back({Action::Down});
            break;
        case E::LEFT:
            result.push_back({Action::Left});
            break;
        case E::RIGHT:
            result.push_back({Action::Right});
            break;
        case E::GRAPHIC_NEXT:
            result.push_back({Action::NextGraphics});
            break;
        case E::GAME_NEXT:
            result.push_back({Action::NextGame});
            break;
        case E::RESET:
            result.push_back({playing_ ? Action::Primary : Action::Confirm});
            break;
        case E::MENU:
            result.push_back({Action::Menu});
            break;
        case E::EXIT:
            result.push_back({Action::Quit});
            break;
        case E::ACTION_1:
            if (playing_)
                result.push_back({Action::Secondary});
            else
                editName(-1, result);
            break;
        case E::ACTION_2:
            if (playing_)
                result.push_back({Action::Pause});
            else
                editName(1, result);
            break;
        case E::ACTION_3:
            if (playing_)
                result.push_back({Action::Restart});
            else if (!player_.empty())
                letter_ = (letter_ + player_.size() - 1) % player_.size();
            break;
        case E::ACTION_4:
            if (playing_)
                result.push_back({Action::Difficulty});
            else if (!player_.empty())
                letter_ = (letter_ + 1) % player_.size();
            break;
        default:
            break;
        }
    }
    return result;
}

} // namespace arcade::legacy
