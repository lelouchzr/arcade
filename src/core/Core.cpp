#include "Core.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <set>
#include <thread>

namespace arcade {
Core::Core(const std::filesystem::path &initialGraphics)
    : scores_(std::getenv("ARCADE_SCORE_FILE") ? std::getenv("ARCADE_SCORE_FILE")
                                               : "data/scores.tsv") {
    // Validate the requested module before discovery and before opening any GUI.
    Plugin initial(initialGraphics);
    if (initial.info().kind != PluginKind::Graphics)
        throw std::runtime_error("'" + initialGraphics.string() + "' is not a graphical library");
    discover();
    auto found = std::find_if(graphics_.begin(), graphics_.end(),
                              [&](const auto &info) { return info.path == initial.info().path; });
    if (found == graphics_.end()) {
        graphics_.push_back(initial.info());
        selectedGraphics_ = graphics_.size() - 1;
    } else
        selectedGraphics_ = static_cast<std::size_t>(found - graphics_.begin());
    if (games_.empty())
        throw std::runtime_error("no compatible game plugins found in ./lib");
    switchGraphics(selectedGraphics_);
}

Core::~Core() {
    if (display_)
        display_->instance<IGraphics>().close();
}

void Core::discover() {
    if (!std::filesystem::is_directory("lib"))
        throw std::runtime_error("missing ./lib directory; run Arcade from its repository root");
    std::vector<std::filesystem::path> paths;
    for (const auto &entry : std::filesystem::directory_iterator("lib"))
        if (entry.is_regular_file() && entry.path().extension() == ".so")
            paths.push_back(entry.path());
    std::sort(paths.begin(), paths.end());
    std::set<std::string> identifiers;
    for (const auto &path : paths) {
        try {
            Plugin plugin(path);
            auto info = plugin.info();
            if (!identifiers.insert(info.id).second)
                continue;
            (info.kind == PluginKind::Game ? games_ : graphics_).push_back(std::move(info));
        } catch (const std::exception &) {
            // A foreign .so is not an Arcade plugin; never instantiate it.
        }
    }
}

void Core::saveScore() {
    if (game_ && !recorded_) {
        scores_.add(game_->info().id, player_.empty() ? "PLAYER" : player_,
                    game_->instance<IGame>().snapshot().score);
        recorded_ = true;
    }
}

void Core::startGame(std::size_t index) {
    saveScore();
    auto replacement = std::make_unique<Plugin>(games_.at(index).path);
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    replacement->instance<IGame>().reset(static_cast<std::uint64_t>(now) ^ ++runNumber_);
    game_ = std::move(replacement);
    selectedGame_ = index;
    menu_ = false;
    paused_ = false;
    recorded_ = false;
}

void Core::run() {
    using Clock = std::chrono::steady_clock;
    auto previous = Clock::now();
    while (running_) {
        const auto frameStart = Clock::now();
        const auto elapsed = std::chrono::duration<double>(frameStart - previous).count();
        previous = frameStart;
        auto inputs = process(display_->instance<IGraphics>().poll());
        if (!running_)
            break;
        if (!menu_ && !paused_ && game_) {
            const auto before = game_->instance<IGame>().snapshot();
            game_->instance<IGame>().update(std::min(elapsed, 0.25), inputs);
            const auto after = game_->instance<IGame>().snapshot();
            if (after.generation != before.generation) {
                // A plugin can change difficulty or begin a fresh round without being reloaded.
                if (!recorded_)
                    scores_.add(game_->info().id, player_.empty() ? "PLAYER" : player_,
                                before.score);
                recorded_ = false;
            }
            if (after.status != Status::Running)
                saveScore();
        }
        display_->instance<IGraphics>().draw(frame());
        std::this_thread::sleep_until(frameStart + std::chrono::milliseconds(16));
    }
}
} // namespace arcade
