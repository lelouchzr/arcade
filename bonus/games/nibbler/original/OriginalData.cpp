#include "OriginalLevels.hpp"
#include "OriginalMapData.hpp"
#include <array>
#include <stdexcept>
#include <utility>

namespace arcade::bonus {

std::array<OriginalLevel, 32> OriginalLevels::builtin() {
    // Junctions occupy one 8px tile; the space between junctions occupies two.
    constexpr std::array<int, 27> coarse{0,  1,  2,  2,  3,  4,  4,  5,  6,  6,  7,  8,  8, 9,
                                         10, 10, 11, 12, 12, 13, 14, 14, 15, 16, 16, 17, 18};
    constexpr std::array<std::pair<int, int>, 32> plan{{
        {1, 1}, {2, 1}, {3, 1}, {4, 2}, {2, 1}, {3, 1},  {1, 3}, {2, 4},  {3, 1},  {4, 2}, {5, 5},
        {2, 6}, {6, 7}, {1, 3}, {7, 1}, {8, 8}, {5, 9},  {3, 2}, {6, 7},  {4, 10}, {5, 5}, {8, 8},
        {7, 1}, {1, 3}, {6, 7}, {5, 9}, {8, 8}, {4, 10}, {7, 1}, {3, 11}, {5, 9},  {8, 8},
    }};
    constexpr std::array<int, 32> speeds{2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6,
                                         6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
    std::array<OriginalLevel, 32> levels;
    for (std::size_t i = 0; i < levels.size(); ++i) {
        auto &level = levels[i];
        level.wave = static_cast<int>(i + 1);
        level.maze = plan[i].first;
        level.fruitPattern = plan[i].second;
        level.speed = speeds[i];
        level.period = 8.0 / (60.0 * level.speed);
        level.certified = true;
        level.provenance = "Video transcription: https://youtu.be/60gVbJTOUOo; "
                           "composition/speed codes: FrankRizzo890/NibblerBot/NibblerNotes.txt; "
                           "see original/data/manifest.json for per-wave evidence";
        const auto &geometry = OriginalMapData::geometry(level.maze);
        for (const auto y : coarse) {
            std::string row;
            for (const auto x : coarse)
                row.push_back(geometry[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)]);
            level.rows.push_back(std::move(row));
        }
        for (const auto &[x, y] : OriginalMapData::fruit(level.fruitPattern)) {
            auto &cell = level.rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            if (cell != ' ')
                throw std::logic_error("Nibbler reference fruit overlaps a wall");
            cell = '.';
        }
        for (int x = 10; x < 13; ++x)
            level.rows[25][static_cast<std::size_t>(x)] = 'o';
        level.rows[25][13] = 'H';
        validate(level);
    }
    return levels;
}

} // namespace arcade::bonus
