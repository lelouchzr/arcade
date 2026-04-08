#include "../../bonus/legacy/LegacyGame.hpp"
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace {

void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

} // namespace

int main(int argc, char **argv) {
    if (argc != 3)
        return 1;
    try {
        const auto source = std::filesystem::absolute(argv[1]);
        {
            arcade::SharedLibrary original(source);
            (void)original.symbol<void (*)()>("createGame");
            const auto factory =
                original.symbol<Arcade::SharedInterfaces::IGame *(*)()>("entryPoint");
            std::unique_ptr<Arcade::SharedInterfaces::IGame> old(factory());
            require(old->getGameName() == "Snake", "fixture is not the original Snake");
            require(!old->isGameRunning(), "historical isGameRunning semantics changed");
            const auto map = old->getMap();
            old->update({}, 0);
            require(old->getMap() == map, "original idle startup semantics changed");
            old->update({Arcade::Types::EventType::UP}, 1);
            require(old->getMap() == map, "original non-zero tick semantics changed");
            old->update({Arcade::Types::EventType::UP}, 0);
            require(old->getMap() != map, "original zero tick must move");
        }
        setenv("ARCADE_LEGACY_GAME", source.c_str(), 1);
        for (int iteration = 0; iteration < 50; ++iteration) {
            arcade::Plugin wrapper(argv[2]);
            auto &game = wrapper.instance<arcade::IGame>();
            const auto before = game.snapshot();
            require(before.status == arcade::Status::Running, "legacy initial status inverted");
            game.update(.1, {});
            require(game.snapshot().cells != before.cells,
                    "adapter did not start historical game automatically");
            const auto generation = game.snapshot().generation;
            game.reset(100);
            require(game.snapshot().generation > generation,
                    "legacy reset did not start fresh round");
        }
        unsetenv("ARCADE_LEGACY_GAME");
        std::cout
            << "Original unmodified Snake, historical ABI, timing and 50 unload cycles passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
