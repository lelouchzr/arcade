#pragma once

#include "CoreAccess.hpp"
#include "Digest.hpp"
#include "Headless.hpp"
#include <memory>
#include <string>

namespace arcade::benchmark {

struct Work {
    std::uint64_t checksum = 0;
    std::uint64_t trace = 0;
    std::uint64_t commands = 0;
    std::uint64_t resets = 0;
    std::uint64_t activeTicks = 0;
};

class Scenario {
  public:
    explicit Scenario(std::string name);
    void prepare();
    void run(std::size_t iterations, bool verify);
    [[nodiscard]] Work result() const;

  private:
    std::string name_;
    std::string gameId_;
    std::unique_ptr<Core> core_;
    std::unique_ptr<Plugin> plugin_;
    Headless renderer_;
    Frame lastFrame_;
    Snapshot lastSnapshot_;
    Digest trace_;
    Work work_;
    bool render_ = false;
    bool model_ = false;
    [[nodiscard]] std::vector<Input> inputs(std::size_t tick) const;
};

} // namespace arcade::benchmark
