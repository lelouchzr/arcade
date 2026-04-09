#include "Scenario.hpp"
#include <array>
#include <stdexcept>

namespace arcade::benchmark {

Scenario::Scenario(std::string name) : name_(std::move(name)) {
    model_ = name_.starts_with("model_");
    render_ = name_ == "pacman_render";
    if (model_)
        gameId_ = name_.substr(6);
    else if (name_ == "pacman_frame" || render_)
        gameId_ = "pacman";
    else if (name_ != "menu_frame")
        throw std::invalid_argument("unknown benchmark case: " + name_);
}

void Scenario::prepare() {
    work_ = {};
    trace_ = Digest{};
    renderer_.reset();
    lastFrame_ = Frame{};
    lastSnapshot_ = Snapshot{};
    if (model_) {
        plugin_ = std::make_unique<Plugin>("lib/arcade_" + gameId_ + ".so");
        plugin_->instance<IGame>().reset(42);
    } else {
        core_ = std::make_unique<Core>("lib/arcade_bench_headless.so");
        CoreAccess::prepare(*core_, gameId_, 42);
    }
}

std::vector<Input> Scenario::inputs(std::size_t tick) const {
    constexpr std::array<Action, 4> directions{Action::Right, Action::Down, Action::Left,
                                               Action::Up};
    const std::size_t period = gameId_ == "pacman"  ? 180
                               : gameId_ == "snake" ? 40
                               : gameId_ == "qix"   ? 16
                                                    : 48;
    std::vector<Input> result;
    if (tick % period == 0)
        result.push_back({directions[(tick / period) % directions.size()]});
    if ((gameId_ == "centipede" || gameId_ == "solarfox") && tick % 12 == 0)
        result.push_back({Action::Primary});
    if (gameId_ == "minesweeper" && tick % 16 == 0)
        result.push_back({Action::Primary});
    return result;
}

void Scenario::run(std::size_t iterations, bool verify) {
    if (model_) {
        auto &game = plugin_->instance<IGame>();
        for (std::size_t tick = 0; tick < iterations; ++tick) {
            if (lastSnapshot_.status != Status::Running) {
                ++work_.resets;
                game.reset(42 + work_.resets);
            }
            game.update(.016, inputs(tick));
            lastSnapshot_ = game.snapshot();
            ++work_.activeTicks;
            if (verify)
                trace_.add(lastSnapshot_);
        }
    } else {
        for (std::size_t frame = 0; frame < iterations; ++frame) {
            lastFrame_ = core_->frame();
            if (render_)
                renderer_.draw(lastFrame_);
            if (verify)
                trace_.add(lastFrame_);
        }
    }
}

Work Scenario::result() const {
    auto result = work_;
    Digest final;
    if (model_)
        final.add(lastSnapshot_);
    else
        final.add(lastFrame_);
    result.checksum = final.value();
    result.trace = trace_.value();
    result.commands = renderer_.commands();
    if (render_) {
        final.add(renderer_.checksum());
        result.checksum = final.value();
    }
    return result;
}

} // namespace arcade::benchmark
