#pragma once
#include <array>
#include <string_view>
#include <utility>
#include <vector>

namespace arcade::bonus {

class OriginalMapData {
  public:
    [[nodiscard]] static const std::array<std::string_view, 19> &geometry(int maze);
    [[nodiscard]] static const std::vector<std::pair<int, int>> &fruit(int pattern);
};

} // namespace arcade::bonus
