#include "BoardRenderer.hpp"
#include "arcade/Plugin.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {

void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

void checkFrame(const arcade::Frame &frame) {
    for (const auto &quad : frame.quads)
        require(quad.width > 0 && quad.height > 0 && quad.x >= 76 && quad.y >= 282 &&
                    quad.x + quad.width <= 848 && quad.y + quad.height <= 687,
                "board rectangle escaped the playfield");
    for (const auto &label : frame.labels)
        require(label.x >= 76 && label.y >= 282 && label.x <= 848 && label.y <= 687,
                "board glyph escaped the playfield");
}

void presentation() {
    using namespace arcade;
    Snapshot board;
    board.width = 25;
    board.height = 21;
    board.cells.assign(525, Cell::Empty);
    board.cells[12 * 25 + 12] = Cell::Player;
    board.cells[2 * 25 + 2] = Cell::Wall;
    board.cursor = {12, 12};
    Frame classic;
    BoardRenderer::draw(classic, board);
    require(classic.quads.size() == 526 && classic.labels.size() == 1,
            "classic board lost cells or cursor");
    checkFrame(classic);
    board.camera = Camera{{12, 12}, 17, 13, true};
    BoardViewport viewport(board);
    require(viewport.origin == Point{4, 6} && viewport.columns == 17 && viewport.rows == 13,
            "follow camera did not center the player");
    require(viewport.cellAt(board, 8, 6) == Cell::Player, "camera looked up the wrong world cell");
    Frame grounded;
    BoardRenderer::draw(grounded, board);
    checkFrame(grounded);
    require(grounded.quads.size() > 17 * 13, "isometric tiles were not projected");
    board.sprites.push_back({{12, 12}, Cell::JumpingPlayer, 1.4});
    board.sprites.push_back({{std::numeric_limits<int>::min(), 0}, Cell::Mine, 2});
    board.sprites.push_back({{0, 0}, Cell::Mine, std::numeric_limits<double>::quiet_NaN()});
    require(viewport.cellAt(board, 8, 6) == Cell::JumpingPlayer,
            "terminal camera lost the airborne actor");
    Frame airborne;
    BoardRenderer::draw(airborne, board);
    checkFrame(airborne);
    require(grounded.labels.size() == airborne.labels.size() && !airborne.labels.empty(),
            "airborne actor was duplicated or removed");
    require(airborne.labels.back().y < grounded.labels.back().y,
            "jump height did not change the actor projection");
    board.cursor = {-1, -1};
    board.sprites = {{{12, 12}, Cell::JumpingGhost, 1.4}};
    Frame crossing;
    BoardRenderer::draw(crossing, board);
    require(std::count_if(crossing.labels.begin(), crossing.labels.end(),
                          [](const Label &label) { return label.text == "@"; }) == 1 &&
                std::count_if(crossing.labels.begin(), crossing.labels.end(),
                              [](const Label &label) { return label.text == "G"; }) == 1,
            "airborne ghost erased the player on the ground");
    board.sprites = {{{12, 12}, Cell::GhostRed, 0}, {{12, 12}, Cell::JumpingPlayer, 1.4}};
    Frame above;
    BoardRenderer::draw(above, board);
    require(std::count_if(above.labels.begin(), above.labels.end(),
                          [](const Label &label) { return label.text == "@"; }) == 1 &&
                std::count_if(above.labels.begin(), above.labels.end(),
                              [](const Label &label) { return label.text == "G"; }) == 1,
            "jumping player erased the ground ghost beneath it");
    for (const Point center : {Point{-1000000, -1000000}, Point{1000000, 1000000}}) {
        board.camera = Camera{center, 1000000, 1000000, true};
        const BoardViewport clamped(board);
        require(clamped.origin.x >= 0 && clamped.origin.y >= 0 && clamped.columns <= 30 &&
                    clamped.rows <= 21,
                "camera bounds were not constrained");
        Frame edge;
        BoardRenderer::draw(edge, board);
        checkFrame(edge);
    }
    board.camera = Camera{{24, 20}, 7, 5, false};
    const BoardViewport edge(board);
    require(edge.origin == Point{18, 16}, "camera did not clamp at the bottom/right edge");
    bool rejected = false;
    try {
        (void)edge.cellAt(board, 7, 0);
    } catch (const std::out_of_range &) {
        rejected = true;
    }
    require(rejected, "viewport accepted an out-of-range column");
    Snapshot tiny;
    tiny.width = tiny.height = 1;
    tiny.cells = {Cell::Wall};
    tiny.camera = Camera{{0, 0}, 1, 1, true};
    Frame small;
    BoardRenderer::draw(small, tiny);
    checkFrame(small);
    require(small.quads.size() > 20, "small isometric camera clipped away the wall top");
    board.cells.pop_back();
    Frame invalid;
    BoardRenderer::draw(invalid, board);
    require(invalid.quads.empty() && invalid.labels.empty(), "invalid board was rendered");
}

} // namespace

int main(int argc, char **argv) {
    try {
        require(argc == 2, "provide the old SDK fixture");
        presentation();
        bool rejected = false;
        try {
            arcade::Plugin old(argv[1]);
        } catch (const std::runtime_error &error) {
            rejected = std::string(error.what()).find("ABI") != std::string::npos;
        }
        require(rejected, "ABI 2 plugin was accepted after the Snapshot extension");
        std::cout << "Camera, isometric projection, sprites, clipping and ABI rejection passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
