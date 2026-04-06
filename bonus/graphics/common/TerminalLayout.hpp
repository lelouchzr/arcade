#pragma once
#include "arcade/Api.hpp"

namespace arcade::bonus {
struct TerminalText {
    int x, y;
    std::string text;
    Color color;
};

// Preserve semantic game cells and readable menus in character-based libraries.
std::vector<TerminalText> terminalLayout(const Frame &frame, int columns, int rows);
} // namespace arcade::bonus
