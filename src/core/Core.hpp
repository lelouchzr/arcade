#pragma once
#include "arcade/Plugin.hpp"
#include "arcade/Scores.hpp"
#include <memory>
#include <set>

namespace arcade::benchmark {
class CoreAccess;
}

namespace arcade {
class Core {
  public:
    explicit Core(const std::filesystem::path &initialGraphics);
    ~Core();
    void run();
    [[nodiscard]] Frame frame() const;

  private:
    friend class benchmark::CoreAccess;
    std::vector<PluginInfo> games_, graphics_;
    std::unique_ptr<Plugin> display_, game_;
    std::size_t selectedGame_ = 0, selectedGraphics_ = 0;
    std::set<std::size_t> unavailableGraphics_;
    std::string graphicsWarning_;
    std::string player_ = "PLAYER";
    Scores scores_;
    bool menu_ = true, running_ = true, paused_ = false, recorded_ = false;
    std::uint64_t runNumber_ = 0;
    void discover();
    void switchGraphics(std::size_t index);
    void cycleGraphics(bool previous = false);
    void startGame(std::size_t index);
    void saveScore();
    std::vector<Input> process(const std::vector<Input> &inputs);
};
} // namespace arcade
