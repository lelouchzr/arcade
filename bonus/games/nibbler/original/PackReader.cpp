#include "OriginalLevels.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace arcade::bonus {
namespace {

std::string line(std::istream &input) {
    std::string result;
    char character = 0;
    while (input.get(character)) {
        if (character == '\n')
            break;
        if (result.size() >= 2048)
            throw std::invalid_argument("Nibbler pack line exceeds 2048 bytes");
        result.push_back(character);
    }
    if (!input && result.empty())
        throw std::invalid_argument("unexpected end of Nibbler pack");
    if (!result.empty() && result.back() == '\r')
        result.pop_back();
    return result;
}

void endOfFields(std::istringstream &fields) {
    fields >> std::ws;
    if (!fields.eof())
        throw std::invalid_argument("unexpected trailing Nibbler pack fields");
}

} // namespace

std::array<OriginalLevel, 32> OriginalLevels::load(const std::filesystem::path &path) {
    if (!std::filesystem::is_regular_file(path) || std::filesystem::file_size(path) > 65536)
        throw std::invalid_argument("Nibbler pack must be a regular file of at most 64 KiB");
    std::ifstream input(path);
    if (!input)
        throw std::runtime_error("cannot open Nibbler pack: " + path.string());
    return parse(input, path.string());
}

std::array<OriginalLevel, 32> OriginalLevels::parse(std::istream &input,
                                                    const std::string &source) {
    if (line(input) != "NIBBLER_PACK 1")
        throw std::invalid_argument("expected NIBBLER_PACK 1 header");
    std::string provenance;
    {
        std::istringstream fields(line(input));
        std::string keyword;
        fields >> keyword >> std::ws;
        if (fields.peek() != '"' || !(fields >> std::quoted(provenance)) ||
            keyword != "PROVENANCE" || provenance.empty() || provenance.size() > 1024)
            throw std::invalid_argument("expected a nonempty quoted Nibbler pack provenance");
        endOfFields(fields);
    }
    std::array<OriginalLevel, 32> levels;
    for (std::size_t i = 0; i < levels.size(); ++i) {
        auto &level = levels[i];
        std::istringstream fields(line(input));
        std::string keyword;
        if (!(fields >> keyword >> level.wave >> level.maze >> level.fruitPattern >> level.speed >>
              level.width >> level.height) ||
            keyword != "LEVEL" || level.wave != static_cast<int>(i + 1))
            throw std::invalid_argument("Nibbler pack requires ordered waves 1 through 32");
        endOfFields(fields);
        if ((level.width != 19 && level.width != 27) || level.height != level.width)
            throw std::invalid_argument("Nibbler pack requires 19x19 or 27x27 square levels");
        if (level.speed < 2 || level.speed > 7)
            throw std::invalid_argument("Nibbler speed code must be between 2 and 7");
        level.period = 8.0 / (60.0 * level.speed);
        level.provenance =
            provenance + " [imported from " + source + "; not independently certified]";
        for (int row = 0; row < level.height; ++row)
            level.rows.push_back(line(input));
        if (line(input) != "ENDLEVEL")
            throw std::invalid_argument("expected ENDLEVEL after the Nibbler rows");
        validate(level);
    }
    if (line(input) != "ENDPACK")
        throw std::invalid_argument("expected ENDPACK after exactly 32 Nibbler waves");
    char trailing = 0;
    while (input.get(trailing))
        if (trailing != ' ' && trailing != '\t' && trailing != '\r' && trailing != '\n')
            throw std::invalid_argument("unexpected data after Nibbler pack");
    if (input.bad())
        throw std::runtime_error("I/O error reading Nibbler pack");
    return levels;
}

} // namespace arcade::bonus
