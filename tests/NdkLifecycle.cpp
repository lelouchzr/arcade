#include "arcade/Plugin.hpp"
#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string_view>

int main(int argc, char **argv) {
    try {
        if (argc != 3)
            throw std::runtime_error("expected NDK++ plugin path and cycle count");
        unsigned cycles = 0;
        const std::string_view count(argv[2]);
        const auto parsed = std::from_chars(count.data(), count.data() + count.size(), cycles);
        if (parsed.ec != std::errc{} || parsed.ptr != count.data() + count.size() || !cycles ||
            cycles > 1000)
            throw std::runtime_error("cycle count must be 1..1000");
        arcade::Frame frame;
        frame.title = "NDK LIFECYCLE";
        frame.board = arcade::Snapshot{};
        frame.board->width = frame.board->height = 5;
        frame.board->cells.assign(25, arcade::Cell::Wall);
        frame.board->cells[12] = arcade::Cell::Head;
        for (unsigned cycle = 0; cycle < cycles; ++cycle) {
            arcade::Plugin plugin(argv[1]);
            auto &graphics = plugin.instance<arcade::IGraphics>();
            for (unsigned reopen = 0; reopen < 3; ++reopen) {
                graphics.open();
                graphics.open(); // Opening an already-open instance must remain harmless.
                frame.board->score = cycle * 3 + reopen;
                graphics.draw(frame);
                (void)graphics.poll();
                graphics.close();
                graphics.close();
            }
            // The actual instance and library unload here, after three independent SCREENs.
        }
        std::cout << "NDK_LIFECYCLE_OK: " << cycles << " module loads and " << cycles * 3
                  << " screen open/close cycles\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
