#include "Options.hpp"
#include "Scenario.hpp"
#include "Workspace.hpp"
#include <chrono>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <vector>

namespace {

void output(const std::string &name, std::size_t iterations, std::size_t repetition,
            const arcade::benchmark::Work &work, double wall, double cpu, bool verify) {
    std::cout << std::setprecision(12) << "{\"case\":\"" << name << "\",\"phase\":\""
              << (verify ? "verify" : "timed") << "\",\"iterations\":" << iterations
              << ",\"repetition\":" << repetition << ",\"wall_us\":" << wall
              << ",\"cpu_us\":" << cpu << ",\"checksum\":" << work.checksum
              << ",\"trace\":" << work.trace << ",\"commands\":" << work.commands
              << ",\"resets\":" << work.resets << ",\"active_ticks\":" << work.activeTicks << "}\n"
              << std::flush;
}

} // namespace

int main(int argc, char **argv) {
    try {
        const auto options = arcade::benchmark::parseOptions(argc, argv);
        arcade::benchmark::Workspace workspace(options.modules, options.graphics);
        const std::vector<std::string> names =
            options.scenario == "all"
                ? std::vector<std::string>{"menu_frame",    "pacman_frame",      "pacman_render",
                                           "model_snake",   "model_minesweeper", "model_nibbler",
                                           "model_pacman",  "model_qix",         "model_centipede",
                                           "model_solarfox"}
                : std::vector<std::string>{options.scenario};
        std::cerr << "benchmark pid=" << getpid() << " iterations=" << options.iterations
                  << " repetitions=" << options.repetitions << '\n';
        using Clock = std::chrono::steady_clock;
        for (const auto &name : names) {
            arcade::benchmark::Scenario scenario(name);
            if (options.verify) {
                scenario.prepare();
                scenario.run(options.iterations, true);
                output(name, options.iterations, 0, scenario.result(), 0, 0, true);
                continue;
            }
            if (options.profileSeconds > 0) {
                scenario.prepare();
                const auto start = Clock::now();
                std::size_t batches = 0;
                do {
                    scenario.run(options.iterations, false);
                    ++batches;
                } while (std::chrono::duration<double>(Clock::now() - start).count() <
                         options.profileSeconds);
                output(name, options.iterations * batches, 0, scenario.result(),
                       std::chrono::duration<double, std::micro>(Clock::now() - start).count(), 0,
                       false);
                continue;
            }
            for (std::size_t round = 0; round < options.warmups + options.repetitions; ++round) {
                scenario.prepare();
                const auto cpuStart = std::clock();
                const auto wallStart = Clock::now();
                scenario.run(options.iterations, false);
                const auto wallEnd = Clock::now();
                const auto cpuEnd = std::clock();
                if (cpuStart == static_cast<std::clock_t>(-1) ||
                    cpuEnd == static_cast<std::clock_t>(-1))
                    throw std::runtime_error("process CPU clock unavailable");
                if (round >= options.warmups)
                    output(name, options.iterations, round - options.warmups, scenario.result(),
                           std::chrono::duration<double, std::micro>(wallEnd - wallStart).count(),
                           static_cast<double>(cpuEnd - cpuStart) * 1000000.0 / CLOCKS_PER_SEC,
                           false);
            }
        }
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "benchmark error: " << error.what() << '\n';
        return 84;
    }
}
