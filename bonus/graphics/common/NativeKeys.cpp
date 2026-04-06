#include "NativeKeys.hpp"
#include "arcade/Keys.hpp"

namespace arcade::bonus {
Input nativeKey(unsigned key, bool pressed) {
    if (!pressed)
        return key >= 0xff51 && key <= 0xff54 ? Input{Action::Stop} : Input{};
    switch (key) {
    case 0xff51:
        return {Action::Left};
    case 0xff52:
        return {Action::Up};
    case 0xff53:
        return {Action::Right};
    case 0xff54:
        return {Action::Down};
    case 0xff0d:
        return {Action::Confirm};
    case 0xff1b:
        return {Action::Quit};
    case 0xff08:
    case 0xffff:
        return {Action::Backspace};
    case 0xff09:
    case 0xffbf:
        return {Action::NextGraphics};
    case 0xffc0:
        return {Action::NextGame};
    case 0xffc1:
        return {Action::Restart};
    case 0xffc2:
        return {Action::Menu};
    case 0xffc3:
        return {Action::Pause};
    case 0xffc4:
        return {Action::Difficulty};
    default:
        return asciiInput(static_cast<int>(key));
    }
}
} // namespace arcade::bonus
