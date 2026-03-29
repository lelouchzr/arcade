#pragma once
#include <array>

namespace arcade::bonus {
struct MineDifficulty {
    int width, height, mines;
    double seconds;
    const char *name;
};

inline constexpr std::array<MineDifficulty, 3> MineDifficulties{
    {{9, 9, 10, 180, "BEGINNER"}, {16, 16, 40, 420, "INTERMEDIATE"}, {30, 16, 99, 600, "EXPERT"}}};
} // namespace arcade::bonus
