#include "../../bonus/legacy/graphics/LegacyGraphics.hpp"
#include <curses.h>
#include <iostream>
#include <stdexcept>

namespace {

void expect(arcade::legacy::LegacyGraphics &graphics, int key, arcade::Action action) {
    ungetch(key);
    const auto inputs = graphics.poll();
    if (inputs.size() != 1 || inputs[0].action != action)
        throw std::runtime_error("original ncurses event translation failed");
}

} // namespace

int main(int argc, char **argv) {
    if (argc != 2)
        return 84;
    try {
        for (int cycle = 0; cycle < 3; ++cycle) {
            arcade::legacy::LegacyGraphics graphics(argv[1]);
            graphics.open();
            arcade::Frame menu;
            menu.player = "ADA";
            menu.labels.push_back({40, 40, 4, "LEGACY NCURSES 100% %n %s", {255, 255, 255}});
            graphics.draw(menu);
            expect(graphics, ' ', arcade::Action::Confirm);
            arcade::Frame game;
            game.player = "ADA%n";
            game.title = "OLD 100% %n";
            arcade::Snapshot view;
            view.width = 25;
            view.height = 21;
            view.cells.assign(525, arcade::Cell::Empty);
            for (int x = 0; x < 25; ++x)
                view.cells[static_cast<std::size_t>(20 * 25 + x)] = arcade::Cell::Wall;
            view.cursor = {2, 2};
            view.cells[11] = arcade::Cell::Player;
            view.cells[22] = arcade::Cell::PowerPellet;
            view.cells[33] = arcade::Cell::GhostRed;
            view.score = 250;
            view.secondsLeft = 42;
            game.board = view;
            graphics.draw(game);
            int rows = 0, columns = 0;
            getmaxyx(stdscr, rows, columns);
            if (rows >= 30 && columns >= 70 &&
                (mvinch(rows / 2 + 9, columns / 2 - 15) & A_CHARTEXT) != '#')
                throw std::runtime_error("legacy footer overwrote the last board row");
            expect(graphics, KEY_UP, arcade::Action::Up);
            expect(graphics, ' ', arcade::Action::Primary);
            expect(graphics, 'g', arcade::Action::NextGraphics);
            expect(graphics, 'b', arcade::Action::NextGame);
            expect(graphics, 'm', arcade::Action::Menu);
            expect(graphics, 27, arcade::Action::Quit);
            graphics.close();
        }
        std::cout << "Original ncurses: 3 open/draw/native-input/close cycles passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 84;
    }
}
