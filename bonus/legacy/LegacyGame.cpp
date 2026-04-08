#include "LegacyGame.hpp"
#include "../games/Grid.hpp"
#include <cstdlib>
#include <stdexcept>

namespace arcade::legacy {

std::filesystem::path LegacyGame::configuredPath() {
    const char *path = std::getenv("ARCADE_LEGACY_GAME");
    if (!path || !*path)
        throw std::runtime_error("set ARCADE_LEGACY_GAME to a caller-owned historical game module");
    return path;
}

LegacyGame::LegacyGame() : LegacyGame(configuredPath()) {}

LegacyGame::LegacyGame(const std::filesystem::path &path) : library_(path) {
    // These are the two actual exports of the original Snake, not guessed modern symbols.
    (void)library_.symbol<void (*)()>("createGame");
    factory_ = library_.symbol<Factory>("entryPoint");
    reset(1);
}

LegacyGame::~LegacyGame() = default;

void LegacyGame::reset(std::uint64_t seed) {
    (void)seed; // The historical API has no seed argument.
    game_.reset(factory_());
    if (!game_)
        throw std::runtime_error("historical game factory returned null");
    game_->setPlayerName("LEGACY");
    ++generation_;
    pending_.clear();
    accumulated_ = 0;
    started_ = false;
}

void LegacyGame::update(double seconds, const std::vector<Input> &inputs) {
    bonus::validateDelta(seconds);
    if (snapshot().status != Status::Running)
        return;
    for (const auto &input : inputs) {
        using E = Arcade::Types::EventType;
        switch (input.action) {
        case Action::Up:
            pending_.push_back(E::UP);
            break;
        case Action::Down:
            pending_.push_back(E::DOWN);
            break;
        case Action::Left:
            pending_.push_back(E::LEFT);
            break;
        case Action::Right:
            pending_.push_back(E::RIGHT);
            break;
        default:
            break;
        }
    }
    accumulated_ += seconds;
    while (accumulated_ + 1e-9 >= .1 && snapshot().status == Status::Running) {
        accumulated_ -= .1;
        if (!started_ && pending_.empty())
            pending_.push_back(Arcade::Types::EventType::UP);
        // The original implementation ignores any non-zero tick and waits for an initial event.
        game_->update(pending_, 0);
        started_ = true;
        pending_.clear();
    }
}

} // namespace arcade::legacy
