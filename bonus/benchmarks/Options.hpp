#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

namespace arcade::benchmark {

struct Options {
    std::filesystem::path modules;
    std::filesystem::path graphics;
    std::string scenario = "all";
    std::size_t iterations = 10000;
    std::size_t repetitions = 7;
    std::size_t warmups = 1;
    double profileSeconds = 0;
    bool verify = false;
    bool single = false;
};

Options parseOptions(int argc, char **argv);

} // namespace arcade::benchmark
