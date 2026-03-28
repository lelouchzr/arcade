#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace arcade {

struct Score {
    std::string game;
    std::string player;
    std::int64_t points = 0;
};

class Scores {
  public:
    explicit Scores(std::filesystem::path path);
    [[nodiscard]] std::vector<Score> top(const std::string &game, std::size_t count = 5) const;
    void add(std::string game, std::string player, std::int64_t points);

  private:
    std::filesystem::path path_;
};

} // namespace arcade
