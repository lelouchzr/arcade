#include "arcade/Keys.hpp"

namespace arcade {
Input asciiInput(int key) {
    switch (key) {
    case 27:
        return {Action::Quit};
    case '\n':
    case '\r':
        return {Action::Confirm};
    case '\t':
        return {Action::NextGraphics};
    case 127:
    case 8:
        return {Action::Backspace};
    case ' ':
        return {Action::Primary};
    default:
        if (key >= 32 && key < 127)
            return {Action::Text, static_cast<char>(key)};
    }
    return {};
}

} // namespace arcade
