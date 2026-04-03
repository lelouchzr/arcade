#include "OriginalLevels.hpp"
#include <cstdlib>
#include <stdexcept>

namespace arcade::bonus {

const OriginalLevel &OriginalLevels::level(int oneBased) {
    if (oneBased < 1)
        throw std::invalid_argument("Nibbler wave number must be positive");
    static const auto levels = [] {
        const char *path = std::getenv("ARCADE_NIBBLER_LEVELS");
        return path && *path ? load(path) : builtin();
    }();
    return levels[static_cast<std::size_t>((oneBased - 1) % 32)];
}

} // namespace arcade::bonus
