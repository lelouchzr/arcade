#include "arcade/Plugin.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <unistd.h>

namespace {

void require(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}

} // namespace

int main(int argc, char **argv) {
    if (argc != 3)
        return 1;
    const auto log = std::filesystem::temp_directory_path() /
                     ("arcade-legacy-graphics-" + std::to_string(getpid()));
    try {
        setenv("ARCADE_LEGACY_GRAPHICS", std::filesystem::absolute(argv[1]).c_str(), 1);
        setenv("ARCADE_LEGACY_PROFILE", "raw", 1);
        setenv("ARCADE_LEGACY_GRAPHIC_LOG", log.c_str(), 1);
        for (int cycle = 0; cycle < 20; ++cycle) {
            arcade::Plugin plugin(argv[2]);
            require(plugin.info().kind == arcade::PluginKind::Graphics, "wrapper is not graphics");
            auto &display = plugin.instance<arcade::IGraphics>();
            display.open();
            display.open();
            arcade::Frame menu;
            menu.player = "ADA";
            menu.quads.push_back({0, 0, 1280, 800, {255, 255, 255}, true});
            menu.labels.push_back({100, 100, 2, "MENU %n %s 100%", {255, 255, 255}});
            display.draw(menu);
            setenv("ARCADE_LEGACY_EVENTS", "SPACE", 1);
            require(display.poll().at(0).action == arcade::Action::Confirm,
                    "legacy space did not launch menu game");
            setenv("ARCADE_LEGACY_EVENTS", "J", 1);
            const auto rename = display.poll();
            require(rename.size() == 19 && rename[16].text == 'B',
                    "old name editor did not emit replacement name");
            arcade::Frame game;
            game.title = "GAME 100%";
            game.player = "ADA%";
            arcade::Snapshot view;
            view.width = 2;
            view.height = 2;
            view.cells = {arcade::Cell::Player, arcade::Cell::PowerPellet, arcade::Cell::GhostRed,
                          arcade::Cell::Projectile};
            view.cursor = {1, 1};
            view.score = 10000000000LL;
            view.secondsLeft = 41.2;
            game.board = view;
            display.draw(game);
            setenv("ARCADE_LEGACY_EVENTS", "UP DOWN LEFT RIGHT G B SPACE M ESC H J K L", 1);
            const auto inputs = display.poll();
            const std::vector<arcade::Action> expected = {
                arcade::Action::Up,        arcade::Action::Down,         arcade::Action::Left,
                arcade::Action::Right,     arcade::Action::NextGraphics, arcade::Action::NextGame,
                arcade::Action::Primary,   arcade::Action::Menu,         arcade::Action::Quit,
                arcade::Action::Secondary, arcade::Action::Pause,        arcade::Action::Restart,
                arcade::Action::Difficulty};
            require(inputs.size() == expected.size(), "legacy events dropped");
            for (std::size_t i = 0; i < inputs.size(); ++i)
                require(inputs[i].action == expected[i], "legacy event translation mismatch");
            display.close();
            display.close();
        }
        setenv("ARCADE_LEGACY_PROFILE", "sfml", 1);
        setenv("ARCADE_LEGACY_TEXT_ONLY", "1", 1);
        {
            arcade::Plugin plugin(argv[2]);
            auto &display = plugin.instance<arcade::IGraphics>();
            display.open();
            arcade::Frame frame;
            frame.player = "GUI%";
            frame.title = "GUI 100%";
            arcade::Snapshot board;
            board.width = board.height = 1;
            board.cells = {arcade::Cell::Player};
            frame.board = board;
            display.draw(frame);
            display.close();
        }
        std::ifstream file(log);
        const std::string content{std::istreambuf_iterator<char>(file), {}};
        for (const auto &token :
             {"TILE 0 4 2x2", "SPRITE 2 +", "GAME GAME 100%%", "PLAYER ADA%%", "TIME 42",
              "SCORE 2147483647", "TEXT 7 3 MENU %n %s 100%", "DESTROY"})
            require(content.find(token) != std::string::npos,
                    "legacy drawing or HUD was not forwarded");
        require(content.find("TEXT -19 -7 @\n") != std::string::npos,
                "GUI text fallback coordinates were not translated");
        require(content.find("GAME GUI 100%\n") != std::string::npos &&
                    content.find("PLAYER GUI%\n") != std::string::npos,
                "GUI profile doubled a literal percent sign");
        std::filesystem::remove(log);
        std::cout << "Legacy IGraphic drawing, HUD, name editor, all events and 20 ownership "
                     "cycles passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        std::filesystem::remove(log);
        return 1;
    }
}
