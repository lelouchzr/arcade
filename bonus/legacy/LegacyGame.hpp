#pragma once

#include "arcade/Api.hpp"
#include "arcade/Plugin.hpp"
#include "sdk/IGame.hpp"
#include <memory>

namespace arcade::legacy {

class LegacyGame final : public IGame {
  public:
    LegacyGame();
    explicit LegacyGame(const std::filesystem::path &path);
    ~LegacyGame() override;
    void reset(std::uint64_t seed) override;
    void update(double seconds, const std::vector<Input> &inputs) override;
    [[nodiscard]] Snapshot snapshot() const override;

  private:
    using Factory = Arcade::SharedInterfaces::IGame *(*)();
    SharedLibrary library_;
    Factory factory_ = nullptr;
    std::unique_ptr<Arcade::SharedInterfaces::IGame> game_;
    std::vector<Arcade::Types::EventType> pending_;
    std::uint64_t generation_ = 0;
    double accumulated_ = 0;
    bool started_ = false;
    static std::filesystem::path configuredPath();
    static Cell translate(Arcade::Types::SpriteType cell);
};

} // namespace arcade::legacy
