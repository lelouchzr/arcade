#include "OriginalLevels.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace arcade::bonus {

void OriginalLevels::validate(const OriginalLevel &level) {
    if ((level.width != 19 && level.width != 27) || level.height != level.width ||
        level.rows.size() != static_cast<std::size_t>(level.height) || level.wave < 1 ||
        level.wave > 32 || level.maze < 1 || level.maze > 8 || level.fruitPattern < 1 ||
        level.fruitPattern > 11 || level.speed < 2 || level.speed > 7 ||
        !std::isfinite(level.period) || level.period <= 0 || level.provenance.empty())
        throw std::invalid_argument("invalid Nibbler level metadata");
    const auto width = static_cast<std::size_t>(level.width);
    const auto area = width * width;
    std::size_t head = 0;
    int heads = 0, bodies = 0, fruits = 0;
    for (std::size_t y = 0; y < width; ++y) {
        if (level.rows[y].size() != width)
            throw std::invalid_argument("Nibbler rows must contain exactly the declared width");
        for (std::size_t x = 0; x < width; ++x) {
            const char cell = level.rows[y][x];
            if (cell != '#' && cell != ' ' && cell != '.' && cell != 'H' && cell != 'o')
                throw std::invalid_argument("unsupported Nibbler cell");
            if ((x == 0 || y == 0 || x + 1 == width || y + 1 == width) && cell != '#')
                throw std::invalid_argument("Nibbler pack needs closed outer walls");
            if (cell == 'H') {
                head = y * width + x;
                ++heads;
            }
            bodies += cell == 'o' ? 1 : 0;
            fruits += cell == '.' ? 1 : 0;
        }
    }
    if (heads != 1 || bodies != 3 || fruits == 0)
        throw std::invalid_argument("Nibbler level needs one head, three body cells and food");
    const auto cellAt = [&](std::size_t index) { return level.rows[index / width][index % width]; };
    const auto neighbours = [width](std::size_t index) {
        return std::array<std::size_t, 4>{index + 1, index + width, index - 1, index - width};
    };
    // Closed borders ensure all queued cells have four safe, non-wrapping neighbours.
    std::vector<bool> visited(area, false);
    visited[head] = true;
    std::vector<std::size_t> pending{head};
    for (std::size_t i = 0; i < pending.size(); ++i)
        for (const auto next : neighbours(pending[i]))
            if (!visited[next] && cellAt(next) != '#') {
                visited[next] = true;
                pending.push_back(next);
            }
    for (std::size_t i = 0; i < visited.size(); ++i)
        if ((cellAt(i) == '.' || cellAt(i) == 'o') && !visited[i])
            throw std::invalid_argument("unreachable food or body in Nibbler level");
    std::vector<bool> bodyVisited(area, false);
    bodyVisited[head] = true;
    std::size_t current = head;
    for (int segment = 0; segment < 3; ++segment) {
        std::vector<std::size_t> choices;
        for (const auto next : neighbours(current))
            if (!bodyVisited[next] && cellAt(next) == 'o')
                choices.push_back(next);
        if (choices.size() != 1)
            throw std::invalid_argument("Nibbler body must form a simple four-cell chain");
        current = choices.front();
        bodyVisited[current] = true;
    }
    for (const auto next : neighbours(current))
        if (cellAt(next) == 'H')
            throw std::invalid_argument("Nibbler body cannot start in a loop");
}

} // namespace arcade::bonus
