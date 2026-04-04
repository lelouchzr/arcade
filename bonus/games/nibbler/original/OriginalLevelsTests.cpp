#include "OriginalLevelsTests.hpp"
#include "../../../../tests/games/TestSupport.hpp"
#include "OriginalLevels.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace arcade::test {
namespace {

void rejected(const std::function<void()> &operation, const std::string &message) {
    bool caught = false;
    try {
        operation();
    } catch (const std::invalid_argument &) {
        caught = true;
    }
    require(caught, message);
}

std::string serialized() {
    std::ostringstream text;
    text << "NIBBLER_PACK 1\nPROVENANCE " << std::quoted("OriginalLevelsTests fixture") << '\n';
    for (int wave = 1; wave <= 32; ++wave) {
        const auto &level = bonus::OriginalLevels::level(wave);
        text << "LEVEL " << wave << ' ' << level.maze << ' ' << level.fruitPattern << ' '
             << level.speed << ' ' << level.width << ' ' << level.height << '\n';
        for (const auto &row : level.rows)
            text << row << '\n';
        text << "ENDLEVEL\n";
    }
    text << "ENDPACK\n";
    return text.str();
}

void referenceTests() {
    // Compare the compiled tables against the independent, video-verified export.
    const auto reference = bonus::OriginalLevels::load(ARCADE_REFERENCE_PACK);
    for (int wave = 1; wave <= 32; ++wave) {
        const auto &expected = reference[static_cast<std::size_t>(wave - 1)];
        const auto &actual = bonus::OriginalLevels::level(wave);
        require(actual.rows == expected.rows && actual.wave == expected.wave &&
                    actual.maze == expected.maze && actual.fruitPattern == expected.fruitPattern &&
                    actual.speed == expected.speed && actual.period == expected.period,
                "compiled historical wave differs from independently verified reference");
        require(!expected.certified && actual.certified,
                "reference import must not grant certification to external packs");
    }
}

void metadataTests() {
    constexpr std::array<int, 11> counts{28, 28, 28, 29, 29, 26, 29, 28, 29, 27, 30};
    for (int wave = 1; wave <= 32; ++wave) {
        const auto &level = bonus::OriginalLevels::level(wave);
        bonus::OriginalLevels::validate(level);
        require(level.wave == wave && level.width == 27 && level.height == 27,
                "historical pack dimensions or wave order");
        require(level.certified && !level.provenance.empty(), "historical evidence missing");
        int fruits = 0, heads = 0, bodies = 0;
        for (const auto &row : level.rows) {
            fruits += static_cast<int>(std::count(row.begin(), row.end(), '.'));
            heads += static_cast<int>(std::count(row.begin(), row.end(), 'H'));
            bodies += static_cast<int>(std::count(row.begin(), row.end(), 'o'));
        }
        require(fruits == counts[static_cast<std::size_t>(level.fruitPattern - 1)],
                "historical food pattern count");
        require(heads == 1 && bodies == 3 && level.rows[25][13] == 'H',
                "historical pack must start with four connected cells at the bottom");
        require(std::abs(level.period - 8.0 / (60.0 * level.speed)) < 1e-12,
                "historical speed-code timing model");
    }
    require(&bonus::OriginalLevels::level(33) == &bonus::OriginalLevels::level(1),
            "32-wave pack did not wrap");
    rejected([] { static_cast<void>(bonus::OriginalLevels::level(0)); }, "accepted zero wave");
    rejected([] { static_cast<void>(bonus::OriginalLevels::level(-1)); }, "accepted negative wave");
}

void importTests() {
    const auto text = serialized();
    std::istringstream input(text);
    const auto imported = bonus::OriginalLevels::parse(input, "roundtrip");
    for (std::size_t i = 0; i < imported.size(); ++i) {
        const auto &expected = bonus::OriginalLevels::level(static_cast<int>(i + 1));
        require(imported[i].rows == expected.rows && imported[i].speed == expected.speed,
                "external Nibbler pack roundtrip changed level");
        require(!imported[i].certified, "an arbitrary imported pack certified itself");
    }
    for (const auto &bad : {std::string("NIBBLER_PACK 2\n"),
                            std::string("NIBBLER_PACK 1\nPROVENANCE missingquotes\n"),
                            text.substr(0, text.size() / 2), text + "TRAILING\n"}) {
        rejected(
            [&] {
                std::istringstream broken(bad);
                static_cast<void>(bonus::OriginalLevels::parse(broken, "broken"));
            },
            "accepted malformed or truncated Nibbler pack");
    }
    auto invalidSpeed = text;
    const auto offset = invalidSpeed.find("LEVEL 1 1 1 2 27 27");
    require(offset != std::string::npos, "speed fixture header missing");
    invalidSpeed.replace(offset, 19, "LEVEL 1 1 1 0 27 27");
    rejected(
        [&] {
            std::istringstream broken(invalidSpeed);
            static_cast<void>(bonus::OriginalLevels::parse(broken, "speed"));
        },
        "accepted zero speed before period division");
}

void invalidGeometryTests() {
    const auto &original = bonus::OriginalLevels::level(1);
    auto invalid = original;
    invalid.rows[0][1] = ' ';
    rejected([&] { bonus::OriginalLevels::validate(invalid); }, "accepted open outer wall");
    invalid = original;
    invalid.rows[1][1] = '?';
    rejected([&] { bonus::OriginalLevels::validate(invalid); }, "accepted unknown cell");
    invalid = original;
    invalid.rows[1][1] = 'H';
    rejected([&] { bonus::OriginalLevels::validate(invalid); }, "accepted duplicate head");
    invalid = original;
    invalid.rows[25][10] = ' ';
    invalid.rows[1][1] = 'o';
    rejected([&] { bonus::OriginalLevels::validate(invalid); }, "accepted disconnected body");
    invalid = original;
    invalid.rows[4][4] = '.'; // The central hole in the top-left closed wall box is unreachable.
    rejected([&] { bonus::OriginalLevels::validate(invalid); }, "accepted unreachable food");
}

} // namespace

void originalLevelsTests() {
    metadataTests();
    referenceTests();
    importTests();
    invalidGeometryTests();
}

} // namespace arcade::test
