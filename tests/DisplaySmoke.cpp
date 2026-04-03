#include "BoardRenderer.hpp"
#include "Core.hpp"
#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char **argv) {
    if (argc != 2)
        return 84;
    try {
        arcade::Plugin plugin(argv[1]);
        auto &display = plugin.instance<arcade::IGraphics>();
        display.open();
        arcade::Frame frame;
        frame.quads.push_back({0, 0, 1280, 800, {10, 15, 23}, false});
        frame.labels.push_back({80, 80, 6, "ARCADE / DISPLAY VERIFIED", {190, 244, 109}});
        arcade::Snapshot board;
        board.width = board.height = 9;
        board.cells.assign(81, arcade::Cell::Empty);
        for (int index = 0; index < 9; ++index) {
            board.cells[static_cast<std::size_t>(index)] = arcade::Cell::Wall;
            board.cells[static_cast<std::size_t>(72 + index)] = arcade::Cell::Wall;
            board.cells[static_cast<std::size_t>(index * 9)] = arcade::Cell::Wall;
            board.cells[static_cast<std::size_t>(index * 9 + 8)] = arcade::Cell::Wall;
        }
        board.cells[40] = arcade::Cell::Player;
        board.cells[39] = arcade::Cell::SecondPlayer;
        board.cells[41] = arcade::Cell::Question;
        board.cells[31] = arcade::Cell::BonusFood;
        board.camera = arcade::Camera{{4, 4}, 7, 7, true};
        board.sprites.push_back({{4, 4}, arcade::Cell::JumpingPlayer, 1.1});
        arcade::BoardRenderer::draw(frame, board);
        display.draw(frame);
        for (int i = 0; i < 5; ++i) {
            (void)display.poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        display.close();
        std::cout << "Display open / camera / elevated sprites / new glyphs / poll / close OK\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 84;
    }
}
