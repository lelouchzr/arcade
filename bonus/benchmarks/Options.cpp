#include "Options.hpp"
#include <charconv>
#include <cmath>
#include <stdexcept>
#include <string_view>

namespace arcade::benchmark {

namespace {

std::size_t number(std::string_view text) {
    std::size_t value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || value > 100000000)
        throw std::invalid_argument("invalid benchmark integer");
    return value;
}

} // namespace

Options parseOptions(int argc, char **argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string_view flag(argv[index]);
        if (flag == "--single") {
            options.single = true;
            options.repetitions = 1;
            continue;
        }
        if (flag == "--verify") {
            options.verify = true;
            continue;
        }
        if (index + 1 >= argc)
            throw std::invalid_argument("missing benchmark option value");
        const std::string value(argv[++index]);
        if (flag == "--module-dir")
            options.modules = std::filesystem::canonical(value);
        else if (flag == "--graphics")
            options.graphics = std::filesystem::canonical(value);
        else if (flag == "--case")
            options.scenario = value;
        else if (flag == "--iterations")
            options.iterations = number(value);
        else if (flag == "--repetitions")
            options.repetitions = number(value);
        else if (flag == "--warmups")
            options.warmups = number(value);
        else if (flag == "--profile-seconds") {
            std::size_t used = 0;
            options.profileSeconds = std::stod(value, &used);
            if (used != value.size() || !std::isfinite(options.profileSeconds) ||
                options.profileSeconds < 0 || options.profileSeconds > 600)
                throw std::invalid_argument("invalid profile duration");
        } else
            throw std::invalid_argument("unknown benchmark option: " + std::string(flag));
    }
    if (options.modules.empty() || options.graphics.empty() || options.iterations == 0 ||
        (!options.single && options.repetitions < 5))
        throw std::invalid_argument(
            "provide module-dir/graphics, iterations > 0, and at least 5 repetitions");
    return options;
}

} // namespace arcade::benchmark
