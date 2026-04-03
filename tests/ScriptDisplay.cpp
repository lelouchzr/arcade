#include "ScriptDisplay.hpp"
#include "arcade/Font.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <stdexcept>

namespace arcade::test {

void ScriptDisplay::open() {
    log(std::string("OPEN ") + TEST_DISPLAY_ID);
}

void ScriptDisplay::close() noexcept {
    try {
        log(std::string("CLOSE ") + TEST_DISPLAY_ID);
    } catch (...) {
    }
}

std::vector<Input> ScriptDisplay::poll() {
    const char *path = std::getenv("ARCADE_TEST_SCRIPT");
    if (!path)
        throw std::runtime_error("missing test script");
    int index = std::getenv("ARCADE_TEST_STEP") ? std::atoi(std::getenv("ARCADE_TEST_STEP")) : 0;
    std::ifstream file(path);
    std::string line;
    for (int i = 0; i <= index; ++i)
        if (!std::getline(file, line))
            return {{Action::Quit}};
    const auto next = std::to_string(index + 1);
    setenv("ARCADE_TEST_STEP", next.c_str(), 1);
    log("INPUT " + line);
    if (line.starts_with("TEXT "))
        return {{Action::Text, line[5]}};
    if (line == "ENTER")
        return {{Action::Confirm}};
    if (line == "SPACE")
        return {{Action::Primary}};
    if (line == "UP")
        return {{Action::Up}};
    if (line == "DOWN")
        return {{Action::Down}};
    if (line == "LEFT")
        return {{Action::Left}};
    if (line == "RIGHT")
        return {{Action::Right}};
    if (line == "F2")
        return {{Action::NextGraphics}};
    if (line == "F3")
        return {{Action::NextGame}};
    if (line == "F4")
        return {{Action::Restart}};
    if (line == "F5")
        return {{Action::Menu}};
    if (line == "F6")
        return {{Action::Pause}};
    if (line == "F7")
        return {{Action::Difficulty}};
    if (line == "BACKSPACE")
        return {{Action::Backspace}};
    if (line == "ESC")
        return {{Action::Quit}};
    return {};
}

void ScriptDisplay::draw(const Frame &frame) {
    std::string labels;
    for (const auto &label : frame.labels)
        labels += label.text + " | ";
    log("FRAME " + labels);
    if (frame.board) {
        const auto &board = *frame.board;
        log("STATE CAMERA " + std::to_string(board.camera ? 1 : 0) + " SPRITES " +
            std::to_string(board.sprites.size()) + " QUESTIONS " +
            std::to_string(std::count(board.cells.begin(), board.cells.end(), Cell::Question)) +
            " SECOND " +
            std::to_string(std::count(board.cells.begin(), board.cells.end(), Cell::SecondPlayer)) +
            " WIDTH " + std::to_string(board.width) + " HEIGHT " + std::to_string(board.height));
    }
    if (const char *output = std::getenv("ARCADE_TEST_CAPTURE")) {
        std::vector<unsigned char> pixels(1280 * 800 * 3, 0);
        const auto paint = [&](const Quad &q) {
            for (int y = std::max(0, q.y); y < std::min(800, q.y + q.height); ++y)
                for (int x = std::max(0, q.x); x < std::min(1280, q.x + q.width); ++x) {
                    if (q.outline && x != q.x && x != q.x + q.width - 1 && y != q.y &&
                        y != q.y + q.height - 1)
                        continue;
                    const auto offset = static_cast<std::size_t>((y * 1280 + x) * 3);
                    pixels[offset] = q.color.r;
                    pixels[offset + 1] = q.color.g;
                    pixels[offset + 2] = q.color.b;
                }
        };
        for (const auto &q : frame.quads)
            paint(q);
        for (const auto &label : frame.labels)
            drawLabel(label, paint);
        std::ofstream image(output, std::ios::binary);
        image << "P6\n1280 800\n255\n";
        image.write(reinterpret_cast<const char *>(pixels.data()),
                    static_cast<std::streamsize>(pixels.size()));
    }
}

void ScriptDisplay::log(const std::string &line) {
    if (const char *path = std::getenv("ARCADE_TEST_LOG")) {
        std::ofstream file(path, std::ios::app);
        file << line << '\n';
    }
}
} // namespace arcade::test

ARCADE_PLUGIN(arcade::test::ScriptDisplay, arcade::PluginKind::Graphics, TEST_DISPLAY_ID,
              "OFFSCREEN PREVIEW")
