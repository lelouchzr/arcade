#include "Headless.hpp"
#include "arcade/Font.hpp"

namespace arcade::benchmark {

void Headless::open() {}

void Headless::close() noexcept {}

std::vector<Input> Headless::poll() {
    return {};
}

void Headless::reset() {
    checksum_ = commands_ = 0;
}

void Headless::quad(const Quad &q) {
    ++commands_;
    checksum_ += static_cast<std::uint64_t>(q.x + q.width) * 131ULL +
                 static_cast<std::uint64_t>(q.y + q.height) * 17ULL + q.color.r * 7ULL +
                 q.color.g * 3ULL + q.color.b;
}

void Headless::draw(const Frame &frame) {
    for (const auto &value : frame.quads)
        quad(value);
    for (const auto &label : frame.labels)
        drawLabel(label, [this](const Quad &value) { quad(value); });
}

std::uint64_t Headless::checksum() const {
    return checksum_;
}

std::uint64_t Headless::commands() const {
    return commands_;
}

} // namespace arcade::benchmark

#ifndef ARCADE_NO_ENTRY
ARCADE_PLUGIN(arcade::benchmark::Headless, arcade::PluginKind::Graphics, "bench_headless",
              "BENCH HEADLESS")
#endif
