#pragma once

#include <array>
#include <filesystem>
#include <istream>
#include <string>
#include <vector>

namespace arcade::bonus {

struct OriginalLevel {
    int width = 27;
    int height = 27;
    int wave = 1;
    int maze = 1;
    int fruitPattern = 1;
    int speed = 2;
    double period = 8.0 / 120.0;
    bool certified = false;
    std::string provenance;
    std::vector<std::string> rows;
};

class OriginalLevels {
  public:
    // Built-in reconstructions retain their evidence status; the number is one-based.
    [[nodiscard]] static const OriginalLevel &level(int oneBased);
    [[nodiscard]] static std::array<OriginalLevel, 32> load(const std::filesystem::path &path);
    [[nodiscard]] static std::array<OriginalLevel, 32> parse(std::istream &input,
                                                             const std::string &source);
    static void validate(const OriginalLevel &level);

  private:
    [[nodiscard]] static std::array<OriginalLevel, 32> builtin();
};

} // namespace arcade::bonus
